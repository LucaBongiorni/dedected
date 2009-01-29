/* dect source plugin 
 * (c) 2008 by Mike Kershaw <dragorn (at) kismetwireless (dot) net,
 *             Jake Appelbaum <ioerror (at) appelbaum (dot) net,
 *             Christian Fromme <kaner (at) strace (dot) org
 *
 * GPL'd code, see LICENCE file for licence information
 */

#include <config.h>
#include <string>
#include <errno.h>
#include <time.h>

#include <util.h>
#include <messagebus.h>
#include <packet.h>
#include <packetchain.h>
#include <packetsource.h>
#include <packetsourcetracker.h>
#include <timetracker.h>
#include <configfile.h>
#include <plugintracker.h>
#include <globalregistry.h>
#include <dumpfile.h>
#include <pcap.h>

//#include "audioDecode.h"

#define COA_IOCTL_MODE                  0xD000
#define COA_IOCTL_CHAN                  0xD004
#define COA_IOCTL_SETRFPI               0xD008
#define COA_MODE_SNIFF                  0x0300
#define COA_SUBMODE_SNIFF_ALL           0x0000
#define COA_SUBMODE_SNIFF_SCANFP        0x0001
#define COA_SUBMODE_SNIFF_SCANPP        0x0002
#define COA_SUBMODE_SNIFF_SYNC          0x0003


/* DECT Client Command Defs */
#define DECT_CMD_START              0
#define DECT_CMD_END                9
#define DECT_SUBCMD_START           0
#define DECT_SUBCMD_END             9

/* Commands */
#define DECT_CMD_CHANHOP            0
#define DECT_CMD_SCAN               1

/* Subcommands */
#define DECT_SUBCMD_CHANHOP_ENABLE  1
#define DECT_SUBCMD_CHANHOP_DISABLE 0
#define DECT_SUBCMD_SCAN_FP         0
#define DECT_SUBCMD_SCAN_PP         1
#define DECT_SUBCMD_SCAN_CALLS      2

// Globals
int dect_comp_datachunk;

static int mode = DECT_SUBCMD_SCAN_FP;
static int switched = 0;

/* This is the 7 bytes we read while scanning */
typedef struct {
    uint8_t               channel;
    uint8_t               RSSI;
    uint8_t               RFPI[5];
} dect_data_scan_t;

typedef struct {
    dect_data_scan_t      sdata;
    uint32_t              first_seen;
    uint32_t              last_seen;
    uint32_t              count_seen;
} dect_data_t;

typedef struct
{
   unsigned char rssi;
   unsigned char channel;
   unsigned char slot;
   struct timespec   timestamp;
   unsigned char data[53];
} pp_packet_t;

// Define an enum of all the fields we send
enum DECT_fields {
	DECT_RFPI, 
    DECT_RSSI, 
    DECT_channel, 
    DECT_first_seen, 
    DECT_last_seen,
    DECT_count_seen, 
	DECT_maxfield
};

// Define the names of each
const char *DECT_fields_text[] = {
	"rfpi", 
    "rssi", 
    "channel", 
    "first_seen", 
    "last_seen", 
    "count_seen", 
    NULL
};

// Prototypes
int dect_register(GlobalRegistry *);
int dect_unregister(GlobalRegistry *);

// Prototype the network write function
int Protocol_DECT(PROTO_PARMS);
void Protocol_DECT_enable(PROTO_ENABLE_PARMS);

// Timer prototype for kicking DECT network data at regular intervals
int dect_timer(TIMEEVENT_PARMS);

// Prototype callback hook for attaching to the chain
int dect_chain_hook(CHAINCALL_PARMS);

// DECT data in a packet
class dect_datachunk : public packet_component {
public:
    dect_data_scan_t sdata;
    pp_packet_t pdata;
    int kind;
    bool sync;
};

class Dumpfile_Dectpcap : public Dumpfile {
public:
    Dumpfile_Dectpcap();
    Dumpfile_Dectpcap(GlobalRegistry *in_globalreg);
    virtual ~Dumpfile_Dectpcap();

    virtual int chain_handler(kis_packet *in_pack);
    virtual int Flush();
private:
    FILE *dectpcapfile;
    pcap_t *pcap;
    pcap_dumper_t *pcap_d;
};

// Dumpfile stuff
int dumpfiledectpcap_chain_hook(CHAINCALL_PARMS) {
	Dumpfile_Dectpcap *auxptr = (Dumpfile_Dectpcap *) auxdata;
	return auxptr->chain_handler(in_pack);
}

Dumpfile_Dectpcap::Dumpfile_Dectpcap() {
    fprintf(stderr, "FATAL OOPS: Dumpfile_Dectpcap called with no globalreg\n");
    exit(1);
}

Dumpfile_Dectpcap::Dumpfile_Dectpcap(GlobalRegistry *in_globalreg) : 
    Dumpfile(in_globalreg) {

    char errstr[STATUS_MAX];
    globalreg = in_globalreg;

    dectpcapfile = NULL;

    type = "dectpcap";

    if (globalreg->packetchain == NULL) {
        fprintf(stderr, "FATAL OOPS:  Packetchain missing before "
                "Dumpfile_Dectpcap\n");
        exit(1);
    }

    // Find the file name
    if ((fname = ProcessConfigOpt("dectpcap")) == "" || 
        globalreg->fatal_condition) {
        return;
    }

    dectpcapfile = fopen(fname.c_str(), "w");
    if (dectpcapfile == NULL) {
        snprintf(errstr, STATUS_MAX, "Failed to open dectpcap dump file '%s': %s",
                 fname.c_str(), strerror(errno));
        globalreg->fatal_condition = 1;
        return;
    }

    char ftime[256];
    char dfname[512];
    time_t rawtime;
    struct tm *timeinfo;

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(ftime, sizeof(ftime), "%Y-%m-%d_%H_%M_%S", timeinfo);

    sprintf(dfname, "dect_dump_%s.pcap", ftime);
    printf("Dumping to %s\n", dfname);
    pcap = pcap_open_dead(DLT_EN10MB, 73);
    if (!pcap) {
        fprintf(stderr, "couldn't pcap_open_dead(\"%s\")\n", dfname);
    }
    pcap_d = pcap_dump_open(pcap, dfname);
    if (!pcap_d) {
        fprintf(stderr, "couldn't pcap_dump_open(\"%s\")\n", dfname);
    }

    globalreg->packetchain->RegisterHandler(&dumpfiledectpcap_chain_hook, this,
                                            CHAINPOS_LOGGING, -100);

    globalreg->RegisterDumpFile(this);
}

Dumpfile_Dectpcap::~Dumpfile_Dectpcap() {
    int opened = 0;

    globalreg->packetchain->RemoveHandler(&dumpfiledectpcap_chain_hook, 
                                          CHAINPOS_LOGGING);
    
    // Close files
    if (dectpcapfile != NULL) {
        Flush();
        fclose(dectpcapfile);
        opened = 1;
    }

    dectpcapfile = NULL;

}

int Dumpfile_Dectpcap::Flush() {
    if (dectpcapfile == NULL)
        return 0;

    fflush(dectpcapfile);

    return 1;
}

int Dumpfile_Dectpcap::chain_handler(kis_packet *in_pack) {
    if (dectpcapfile == NULL)
        return 0;

    if (in_pack->error)
        return 0;
    dect_datachunk *dc = (dect_datachunk *)
            in_pack->fetch(dect_comp_datachunk);

    // We only want kind "2"
    if (!dc || dc->kind != 2) {
        return 0;
    }
    
    // XXX Dump .wav?
    struct pcap_pkthdr pcap_hdr;
    pcap_hdr.caplen = 73;
    pcap_hdr.len = 73;
    int ret = gettimeofday(&pcap_hdr.ts, NULL);
    if (ret) {
        fprintf(stderr, "couldn't gettimeofday(): %s\n", strerror(errno));
        return 0;
    }
    uint8_t pcap_packet[100];
    memset(pcap_packet, 0, 100);
    pcap_packet[12] = 0x23;
    pcap_packet[13] = 0x23;
    pcap_packet[14] = 0x00;        /* decttype (receive) */
    pcap_packet[15] = dc->pdata.channel;
    pcap_packet[16] = 0;
    pcap_packet[17] = dc->pdata.slot;
    pcap_packet[18] = 0;
    pcap_packet[19] = dc->pdata.rssi;
    memcpy(&pcap_packet[20], dc->pdata.data, 53);

    pcap_dump((u_char*)pcap_d, &pcap_hdr, pcap_packet);

    dumped_frames++;

    return 1;
}

// Packetsource - where packets come from.  Reads the device and writes into the chain
class PacketSource_Dect : public KisPacketSource {
public:
	PacketSource_Dect() {
		fprintf(stderr, "FATAL OOPS: KisDectSource()\n");
		exit(1);
	}

	PacketSource_Dect(GlobalRegistry *in_globalreg) 
        : KisPacketSource(in_globalreg),
          locked(false), sync(false), globalreg(in_globalreg) {
	}

	PacketSource_Dect(GlobalRegistry *in_globalreg, string in_interface,
					  vector<opt_pair> *in_opts) : 
		KisPacketSource(in_globalreg, in_interface, in_opts),
        locked(false), sync(false), globalreg(in_globalreg) {

		serial_fd = -1;

		ParseOptions(in_opts);
	}

	virtual ~PacketSource_Dect() {
		if (serial_fd >= 0) {
			close(serial_fd);
		}
	}
    
    virtual void SetExternal(PacketSource_Dect *ex) {
        external = ex;
    }

    virtual PacketSource_Dect* GetExternal(void) {
        return external;
    }

	virtual PacketSource_Dect *CreateSource(GlobalRegistry *in_globalreg,
											string interface,
											vector<opt_pair> *in_opts) {
        PacketSource_Dect *ref = new PacketSource_Dect(in_globalreg, interface, in_opts);
        // XXX Hackalert
        SetExternal(ref);
		return ref;
	}

	virtual int AutotypeProbe(string in_device) {
		if (StrLower(in_device) == "dect") {
			type = "dect";
			return 1;
		}

		return 0;
	}

	// No low-level radio data to fetch
	virtual void FetchRadioData(kis_packet *in_pack) { return; }

	virtual int RegisterSources(Packetsourcetracker *tracker) {
		// source type dect, channel list "DECT", not root
		tracker->RegisterPacketProto("dect", this, "DECT", 0);
		return 1;
	}

	virtual int ParseOptions(vector<opt_pair> *in_opts) {
		if ((serialdevice = FetchOpt("device", in_opts)) == "") {
			_MSG("Packetsource::Dect expected option 'device' for serial dev",
				 MSGFLAG_PRINTERROR);
			return -1;
		}
	}

	virtual int FetchChannelCapable() { return 1; }

	virtual int EnableMonitor() {
		// Do setup, but remember this is called before OpenSource()
		return 1;
	}

	virtual int DisableMonitor() {
		// Same
		return 1;
	}

	virtual int SetChannel(unsigned int in_ch) {
        if (locked) {
            return 0;
        }
        printf("switching to channel %d\n", in_ch);
        if (ioctl(serial_fd, COA_IOCTL_CHAN, &in_ch)){
            printf("couldn't ioctl()\n");
            return 0;
        }

		return 1;
	}

    void startScanFp()
    {
        if (serial_fd == -1) 
            return;
        /* start sniffer mode */
        int val = COA_MODE_SNIFF|COA_SUBMODE_SNIFF_SCANFP;
        if (ioctl(serial_fd, COA_IOCTL_MODE, &val)) {
            fprintf(stderr, "Couldn't ioctl to COA_MODE_SNIFF\n");
            return;
        }
        mode = COA_SUBMODE_SNIFF_SCANFP;
        switched = 1;
        if (sync) {
            printf("Sync off.\n");
            sync = false;
        }
        // Remove lock, if there is any, and start scanning at channel 0
        setLock(false, 0);
    }

    void startScanPp()
    {
        if (serial_fd == -1) 
            return;
        /* start sniffer mode */
        int val = COA_MODE_SNIFF | COA_SUBMODE_SNIFF_SCANPP;
        if (ioctl(serial_fd, COA_IOCTL_MODE, &val)) {
            fprintf(stderr, "Couldn't ioctl to COA_MODE_SNIFF\n");
            return;
        }
        mode = COA_SUBMODE_SNIFF_SCANFP;
        switched = 1;
        if (sync) {
            printf("Sync off.\n");
            sync = false;
        }
        // Remove lock, if there is any, and start scanning at channel 0
        setLock(false, 0);
    }

    void startScanCalls(uint8_t *RFPI, int channel)
    {
        if (serial_fd == -1 || !RFPI)
            return;
        /* set sync sniff mode */
        uint16_t val;
        val = COA_MODE_SNIFF | COA_SUBMODE_SNIFF_SYNC;
        if (ioctl(serial_fd, COA_IOCTL_MODE, &val)){
            fprintf(stderr, "Couldn't ioctl to COA_MODE_SNIFF\n");
            return;
        }
        /* set rfpi to sync with */
        if(ioctl(serial_fd, COA_IOCTL_SETRFPI, RFPI)){
            fprintf(stderr, "Couldn't ioctl SETRFPI\n");
            return;
        }
        mode = 2;
        if (sync) {
            sync = false;
        }
        // Don't hop channels while synced
        setLock(true, channel);
    }

	virtual int FetchDescriptor() {
		// If -1 this will be handled upstream
		return serial_fd;
	}

	virtual int OpenSource() {
        short val;

        if((serial_fd = open(serialdevice.c_str(), O_RDONLY)) == -1) {
            fprintf(stderr, "Could not open %s\n", serialdevice.c_str());
            return 0;
        }

        startScanFp();

		return 1;
	}

	virtual int CloseSource() {
		// Issue any shutdown you need to do
		if (serial_fd >= 0)
			close(serial_fd);
		serial_fd = -1;
        return 1;
	}

	virtual int Poll() {
        unsigned char buf[7];
        int rbytes = -1;
		kis_packet *newpack = globalreg->packetchain->GeneratePacket();
		dect_datachunk *dc = new dect_datachunk;

        // Don't remove this, or it's gonna lock your box
        usleep(10000);

        // Async FP/PP scan read 7 bytes each
        if (mode == 0 || mode == 1) {
            if ((rbytes = read(serial_fd, &(dc->sdata), 7)) != 7) {
                // Fail
                fprintf(stderr, "Bad read. Expected: 7 Got: %d\n", rbytes);
                return 0;
            } else {
                printf("RFPI: ");
                for (int i=0; i < 5; i++) {
                    printf("%.2x:", dc->sdata.RFPI[i]);
                }
                printf("\n");
                dc->kind = 0;
                newpack->insert(dect_comp_datachunk, dc);
                globalreg->packetchain->ProcessPacket(newpack);
            }
        } else if (mode == 2) {
            if ((rbytes = read(serial_fd, &(dc->pdata), sizeof(dc->pdata))) != sizeof(dc->pdata)) {
                fprintf(stderr, "Bad read. Expected: %d Got: %d\n", sizeof(dc->pdata), rbytes);
                return 0;
            } else {
                if (!sync) {
                    printf("Got sync.\n");
                    sync = true;
                }
                dc->sync = sync;
                //dc->kind = 2;
                newpack->insert(dect_comp_datachunk, dc);
                globalreg->packetchain->ProcessPacket(newpack);
            }
        } else {
            fprintf(stderr, "Bad mode selected\n");
            return 0;
        }

		return 1;
	}

    void setLock(bool lock, int arg) 
    {
        if (arg != -1) {
            SetChannel(arg);
        }
        locked = lock;
    }

protected:
	string serialdevice;
	int serial_fd;
    bool locked;
    bool sync;
    PacketSource_Dect *external;
    GlobalRegistry *globalreg;
};

class DectTracker {
public:
	DectTracker() { fprintf(stderr, "FATAL OOPS: DectTracker()\n"); }
	DectTracker(GlobalRegistry *in_globalreg) {
		globalreg = in_globalreg;

		// Register a handler in the packet chain in the classifier category,
		// tho we're going to do our own decoding
		globalreg->packetchain->RegisterHandler(&dect_chain_hook, this,
												CHAINPOS_CLASSIFIER, -100);

		// Register the network protocol
		proto_ref =
			globalreg->kisnetserver->RegisterProtocol("DECT", 0, 1,
													  DECT_fields_text,
													  &Protocol_DECT,
													  &Protocol_DECT_enable,
													  this);

		// Add a timer for once a second to send updated DECT data
		time_ref =
			globalreg->timetracker->RegisterTimer(SERVER_TIMESLICES_SEC, NULL,
												  1, &dect_timer, this);
	}

	// Send dect records
	void BlitDECTProto(int in_fd) {
		for (map<int, dect_data_t *>::iterator x = dect_map.begin();
			 x != dect_map.end(); ++x) {
			kis_protocol_cache cache;

			if (in_fd == -1) {
				if (globalreg->kisnetserver->SendToAll(proto_ref,
													   (void *) x->second) < 0) 
					break;
			} else {
				if (globalreg->kisnetserver->SendToClient(in_fd, proto_ref,
														  (void *) x->second,
														  &cache) < 0)
					break;
			}
		}
	}

	int chain_handler(kis_packet *in_pack) {
		dect_datachunk *dectinfo = (dect_datachunk *)
			in_pack->fetch(dect_comp_datachunk);

        int currentmode = mode;
		// If this packet doesn't have dect info or if it's call data, move 
		// along.
		if (dectinfo == NULL || dectinfo->kind == 2) {
			return 0;
		}
        if (switched) {
            dect_map.clear();
            switched = 0;
        }

        dect_data_t *td = new dect_data_t;
        memcpy(td->sdata.RFPI, &(dectinfo->sdata.RFPI), 5);
        td->sdata.channel = dectinfo->sdata.channel;
        td->sdata.RSSI = dectinfo->sdata.RSSI;
        td->first_seen = time(NULL);
        td->last_seen = time(NULL);
        td->count_seen = 1;

        bool found = false;
        map<int, dect_data_t *>::iterator x = dect_map.begin();
        for (; x != dect_map.end(); ++x) {
            if (!memcmp(x->second->sdata.RFPI, 
                        td->sdata.RFPI, 5)) {
                found = true;
                // Update
                x->second->sdata.RSSI = td->sdata.RSSI;
                x->second->last_seen = time(NULL);
                x->second->count_seen++;
                if (x->second->sdata.channel != td->sdata.channel) {
                    printf("Station %.2x:%.2x:%.2x:%.2x:%.2x\n",
                                              td->sdata.RFPI[0],
                                              td->sdata.RFPI[1],
                                              td->sdata.RFPI[2],
                                              td->sdata.RFPI[3],
                                              td->sdata.RFPI[4]);
                    printf("Changed channels from %d to %d\n", 
                                              td->sdata.channel,
                                              x->second->sdata.channel);
                    x->second->sdata.channel = td->sdata.channel;
                }
            }
        }
        if (!found) {
            dect_map[x->first + 1] = td;
        }

		return 1;
	}

    void emptyMap(void)
    {
        this->dect_map.clear();
    }

protected:
	GlobalRegistry *globalreg;
	map<int, dect_data_t *> dect_map;
	int time_ref;
	int proto_ref;
	// What we track about something
    dect_data_t ddata;

};

// This really should be a struct
class DectCcc {
public:
    PacketSource_Dect *psd;
    DectTracker *dtracker;
};

int dect_cc_callback(CLIENT_PARMS)
{
    int cmd = -1;
    int subcmd = -1;
    int arg = -1;
    uint8_t rfpi[5];
    char rfpi_s[15];

    DectCcc *dc = (DectCcc *)auxptr;
    if (!dc) {
        fprintf(stderr, "Bad arg.\n");
        return 0;
    }

    PacketSource_Dect *psd = dc->psd;
    PacketSource_Dect *ex_psd = psd->GetExternal();
    DectTracker *dtracker = dc->dtracker;
    if (!psd || !ex_psd || !dtracker) {
        fprintf(stderr, "Bad args.\n");
        return 0;
    }
    memset(rfpi, 0, sizeof(rfpi));
    memset(rfpi_s, 0, sizeof(rfpi_s));

    if (parsedcmdline->size() < 3) {
        fprintf(stderr, "Bad client command.\n");
        return 0;
    }
    cmd = atoi((*parsedcmdline)[0].word.c_str());
    subcmd = atoi((*parsedcmdline)[1].word.c_str());
    arg = atoi((*parsedcmdline)[2].word.c_str());
    if (cmd < DECT_CMD_START || cmd > DECT_CMD_END ||
        subcmd < DECT_SUBCMD_START || subcmd > DECT_SUBCMD_END) {
        fprintf(stderr, "Bad DECT client command: %s.\n", cmdline.c_str());
        return 0;
    }
    if (parsedcmdline->size() == 4) {
        // XXX Do this much more elegant
        long rfpi0, rfpi1, rfpi2, rfpi3, rfpi4;
        strncpy(rfpi_s, (*parsedcmdline)[3].word.c_str(), 14);
        sscanf(rfpi_s, "%x:%x:%x:%x:%x", &rfpi0, &rfpi1, &rfpi2, &rfpi3, &rfpi4);
        rfpi[0] = (uint8_t)rfpi0;
        rfpi[1] = (uint8_t)rfpi1;
        rfpi[2] = (uint8_t)rfpi2;
        rfpi[3] = (uint8_t)rfpi3;
        rfpi[4] = (uint8_t)rfpi4;
    }
    printf("CMD: %d SUBCMD: %d ARG: %d\n", cmd, subcmd, arg);

    switch (cmd) {
        case DECT_CMD_CHANHOP:
            switch (subcmd) {
                case DECT_SUBCMD_CHANHOP_ENABLE:
                    printf("DECT_CMD_CHANHOP ENABLE\n");
                    psd->setLock(false, arg);
                    break;
                case DECT_SUBCMD_CHANHOP_DISABLE:
                    printf("DECT_CMD_CHANHOP DISABLE\n");
                    psd->setLock(true, arg);
                    break;
                default:
                    fprintf(stderr, "Bad DECT_CMD_CHANHOP subcommand.\n");
                    break;
            }
            break;
        case DECT_CMD_SCAN:
            switch(subcmd) {
                case DECT_SUBCMD_SCAN_FP:
                    printf("DECT_CMD_SCAN FP\n");
                    ex_psd->startScanFp();
                    dtracker->emptyMap();
                    break;
                case DECT_SUBCMD_SCAN_PP:
                    printf("DECT_CMD_SCAN PP\n");
                    ex_psd->startScanPp();
                    dtracker->emptyMap();
                    break;
                case DECT_SUBCMD_SCAN_CALLS:
                    printf("DECT_CMD_SCAN CALLS\n");
                    printf("Station %.2x:%.2x:%.2x:%.2x:%.2x\n",
                                              rfpi[0],
                                              rfpi[1],
                                              rfpi[2],
                                              rfpi[3],
                                              rfpi[4]);
                    ex_psd->startScanCalls(rfpi, arg);
                    break;
                default:
                    fprintf(stderr, "Bad DECT_CMD_SCAN subcommand.\n");
                    break;
            }
            break;
        default:
            fprintf(stderr, "Bad DECT client command.\n");
            return 0;
    }

    return 1;
}

int dect_chain_hook(CHAINCALL_PARMS) {
	return ((DectTracker *) auxdata)->chain_handler(in_pack);
}

int dect_timer(TIMEEVENT_PARMS) {
	// All we do is send the proto to everyone
	((DectTracker *) parm)->BlitDECTProto(-1);
	return 1;
}

// All the enable function does is send everything we have already
void Protocol_DECT_enable(PROTO_ENABLE_PARMS) {
	((DectTracker *) data)->BlitDECTProto(in_fd);
}

void RFPItoString(string &out, uint8_t RFPI[5])
{
    char tmp[16];
    snprintf(tmp, 16, "%.2x:%.2x:%.2x:%.2x:%.2x", RFPI[0], 
                                                  RFPI[1],
                                                  RFPI[2],
                                                  RFPI[3],
                                                  RFPI[4]);
    out = tmp;
}

// We get a ptr to a dect tracked record, cache system, etc.  Just fill it in.
int Protocol_DECT(PROTO_PARMS) {
	dect_data_t *dect = (dect_data_t *) data;
	
	// Alloc the cache to size
	cache->Filled(field_vec->size());

	for (unsigned int x = 0; x < field_vec->size(); x++) {
		unsigned int fnum = (*field_vec)[x];

		if (fnum >= DECT_maxfield) {
			out_string = "Unknown field requested";
			return -1;
		}

		if (cache->Filled(fnum)) {
			out_string += cache->GetCache(fnum) + " ";
			continue;
		}

        string rfpi;
		// Fill in and cache
		switch (fnum) {
			case DECT_RFPI:
                RFPItoString(rfpi, dect->sdata.RFPI);
                cache->Cache(fnum, rfpi);
				break;
            case DECT_RSSI:
                cache->Cache(fnum, IntToString(dect->sdata.RSSI));
                break;
			case DECT_channel:
				cache->Cache(fnum, IntToString(dect->sdata.channel));
				break;
            case DECT_first_seen:
                cache->Cache(fnum, IntToString(dect->first_seen));
                break;
            case DECT_last_seen:
                cache->Cache(fnum, IntToString(dect->last_seen));
                break;
            case DECT_count_seen:
                cache->Cache(fnum, IntToString(dect->count_seen));
                break;
		}

		out_string += cache->GetCache(fnum) + " ";
	}

	return 1;
}

// This portion has to be an extern c for symbol matching
extern "C" {
	int kis_plugin_info(plugin_usrdata *data) {
		data->pl_name = "DECT";
		data->pl_version = "1.0.0";
		data->pl_description = "DECT sniffer interface";
		data->pl_unloadable = 0; // We can't be unloaded because we defined a source
		data->plugin_register = dect_register;
		data->plugin_unregister = dect_unregister;

		return 1;
	}
}

int dect_unregister(GlobalRegistry *in_globalreg) {
	return 0;
}

int dect_register(GlobalRegistry *globalreg) {
	// Add the dect packet component to the chain registry
	dect_comp_datachunk =
		globalreg->packetchain->RegisterPacketComponent("DECTDATA");
	
	// Add the channels
	globalreg->sourcetracker->AddChannelList("DECT:1,2,3,4,5,6,7,8,9");

    // Hopefully this won't break since we don't know about memory management
    // of packetsources
    PacketSource_Dect *psd = new PacketSource_Dect(globalreg);
    if (!psd) {
        return -1;
    }

	// Add the packet source
	if (globalreg->sourcetracker->RegisterPacketSource(psd) < 0 || globalreg->fatal_condition) {
		_MSG("Failed to add DECT source\n", MSGFLAG_ERROR);
		return -1;
	}

	// Make a tracker
	DectTracker *dtracker;
	dtracker = new DectTracker(globalreg);
    if (!dtracker) {
        return -1;
    }
    DectCcc *dc = new DectCcc;
    if (!dc) {
        return -1;
    }
    dc->dtracker = dtracker;
    dc->psd = psd;

    new Dumpfile_Dectpcap(globalreg);

    globalreg->kisnetserver->RegisterClientCommand("DECT",
                                                    dect_cc_callback,
                                                    dc);

	return 1;
}
