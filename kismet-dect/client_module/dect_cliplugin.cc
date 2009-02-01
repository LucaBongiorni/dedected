/* kismet dect client plugin 
 * (c) 2008 by Mike Kershaw <dragorn (at) kismetwireless (dot) net,
 *             Jake Appelbaum <ioerror (at) appelbaum (dot) net,
 *             Christian Fromme <kaner (at) strace (dot) org
 *
 * GPL'd code, see LICENCE file for licence information
 */

#include <config.h>

#include <stdio.h>
#include <string.h>

#include <globalregistry.h>

#include <kis_panel_plugin.h>
#include <kis_panel_frontend.h>
#include <kis_panel_windows.h>

#define KCLI_DECT_CHANNEL_FIELDS "rfpi,rssi,channel,first_seen,last_seen,count_seen"

#define SORT_BY_RFPI        0
#define SORT_BY_RSSI        1
#define SORT_BY_CHANNEL     2
#define SORT_BY_COUNTSEEN   5

#define MODE_ASYNC_FP_SCAN  0
#define MODE_ASYNC_PP_SCAN  1
#define MODE_SYNC_CALL_SCAN 2

// default: sort by RSSI
static int sort_by = SORT_BY_RSSI;
static bool descending = true;
static int mode = MODE_ASYNC_FP_SCAN;

class DecTable;
struct dect_data {
    DecTable *dtable;
    DecTable *ctable;
    vector<vector <string> > info_vec_fp; 
    vector<vector <string> > info_vec_pp;
    vector<string> sync_station;
    int addref;
    int numrows;
    int mode;
};

class DecTable : public Kis_Scrollable_Table {
public:
    DecTable(GlobalRegistry *in_globalreg, Kis_Panel *in_panel, dect_data *ddata) :
        Kis_Scrollable_Table(in_globalreg, in_panel)
    {
        this->globalreg = in_globalreg;
        this->ddata = ddata;
    }
    ~DecTable() {}

    int KeyPress(int in_key)
    {
        if (visible == 0)
            return 0;

        int scrollable = 1;
        if ((int) data_vec.size() < ly)
            scrollable = 0;
        // Selected up one, scroll up one if we need to
        if (in_key == KEY_UP) {
            if (draw_highlight_selected == 0 && scrollable) {
                // If we're not drawing the highlights then we don't mess
                // with the selected item at all, we just slide the scroll
                // pos up and down, and make sure we don't let them scroll
                // off the end of the world, keep as much of the tail in view
                // as possible
                if (scroll_pos > 0)
                    scroll_pos--;
            } else if (selected > 0) {
                selected--;
                if (scrollable && scroll_pos > 0 && scroll_pos > selected) {
                    scroll_pos--;
                }
            }
        }

        if (in_key == KEY_DOWN && selected < (int) data_vec.size() - 1) {
            if (draw_highlight_selected == 0 && scrollable) {
                // If we're not drawing the highlights then we don't mess
                // with the selected item at all, we just slide the scroll
                // pos up and down, and make sure we don't let them scroll
                // off the end of the world, keep as much of the tail in view
                // as possible
                if (scroll_pos + ly <= (int) data_vec.size() - 1)
                    scroll_pos++;
            } else if (draw_lock_scroll_top && scrollable &&
                scroll_pos + ly - 1 <= selected) {
                // If we're locked to always keep the list filled, we can only
                // scroll until the bottom is visible.  This implies we don't 
                // show the selected row, too
                selected++;
                scroll_pos++;
            } else {
                selected++;
                if (scrollable && scroll_pos + ly - 1 <= selected) {
                    scroll_pos++;
                }
            }
        }

        if (in_key == KEY_RIGHT && hscroll_pos < (int) title_vec.size() - 1) {
            hscroll_pos++;
        }

        if (in_key == KEY_LEFT && hscroll_pos > 0) {
            hscroll_pos--;
        }

        if (in_key == '\n' || in_key == '\r' || in_key == ' ') {
            if (cb_activate != NULL)
                (*cb_activate)(this, GetSelected(), cb_activate_aux, globalreg);

            return GetSelected();
        }

        // Display help
        if (in_key == 'h' || in_key == '?') {
            string help_title = "  Help  ";
            string help_text = "h       - Display this help\n"
                               "L       - Lock channel hopping to current channel\n"
                               "U       - Unlock channel hopping\n"
                               "F       - Do async FP scan (default)\n"
                               "A       - Do async PP scan\n"
                               "M       - Show current mode\n"
                               "i       - Sort by RFPI (ascending)\n"
                               "I       - Sort by RFPI (descending)\n"
                               "r       - Sort by RSSI (ascending)\n"
                               "R       - Sort by RSSI (descending)\n"
                               "c       - Sort by Channel (ascending)\n"
                               "C       - Sort by Channel (descending)\n"
                               "s       - Sort by view count (ascending)\n"
                               "S       - Sort by view count (descending)\n"
                               "<enter> - Sync with selected station and dump calls\n";
            Kis_ModalAlert_Panel *ma = new Kis_ModalAlert_Panel(globalreg, globalreg->panel_interface);
            ma->Position(2, 2, 21, 70);
            ma->ConfigureAlert(help_title, help_text);
            globalreg->panel_interface->AddPanel(ma);
            return 0;
        }
        if (in_key == 'L') {
            // Return currently selected channel to server
            vector<string> s = GetSelectedData();
            if (s.size() < 2) {
                return 1;
            }
            string cmd("DECT 0 0 0 " + s[2]);
            if (globalreg && 
                globalreg->panel_interface && 
                globalreg->panel_interface->FetchFirstNetclient()) {
                globalreg->panel_interface->FetchFirstNetclient()->InjectCommand(cmd.c_str());
            }
            return 0;
        }
        if (in_key == 'U') {
            if (globalreg && 
                globalreg->panel_interface && 
                globalreg->panel_interface->FetchFirstNetclient()) {
                globalreg->panel_interface->FetchFirstNetclient()->InjectCommand("DECT 0 1 0");
            }
            return 0;
        }
        if (in_key == 'F') {
            if (globalreg && 
                globalreg->panel_interface && 
                globalreg->panel_interface->FetchFirstNetclient()) {
                globalreg->panel_interface->FetchFirstNetclient()->InjectCommand("DECT 1 0 0");
            }
            mode = MODE_ASYNC_FP_SCAN;
            if (ddata) {
                ddata->info_vec_pp.clear();
                ddata->dtable->Clear();
            }
            return 0;
        }
        if (in_key == 'A') {
            if (globalreg && 
                globalreg->panel_interface && 
                globalreg->panel_interface->FetchFirstNetclient()) {
               globalreg->panel_interface->FetchFirstNetclient()->InjectCommand("DECT 1 1 0");
            }
            mode = MODE_ASYNC_PP_SCAN;
            if (ddata) {
                ddata->info_vec_fp.clear();
                ddata->dtable->Clear();
            }
            return 0;
        }
        if (in_key == 'M') {
            string current_mode = "Unkown/Other";
            string mode_title = "";
            if (mode == MODE_ASYNC_FP_SCAN) {
                current_mode = "Async FP Scan";
            } else if (mode == MODE_ASYNC_PP_SCAN) {
                current_mode = "Async PP Scan";
            }
            string mode_text = "Current mode is: " + current_mode;
                               
            Kis_ModalAlert_Panel *ma = new Kis_ModalAlert_Panel(globalreg, globalreg->panel_interface);
            ma->Position(6, 10, 5, 45);
            ma->ConfigureAlert(mode_title, mode_text);
            globalreg->panel_interface->AddPanel(ma);
            return 0;
        }
        if (in_key == 'i') {
            sort_by = SORT_BY_RFPI;
            descending = false;
        }
        if (in_key == 'I') {
            sort_by = SORT_BY_RFPI;
            descending = true;
        }
        if (in_key == 'r') {
            sort_by = SORT_BY_RSSI;
            descending = false;
        }
        if (in_key == 'R') {
            sort_by = SORT_BY_RSSI;
            descending = true;
        }
        if (in_key == 'c') {
            sort_by = SORT_BY_CHANNEL;
            descending = false;
        }
        if (in_key == 'C') {
            sort_by = SORT_BY_CHANNEL;
            descending = true;
        }
        if (in_key == 's') {
            sort_by = SORT_BY_COUNTSEEN;
            descending = false;
        }
        if (in_key == 'S') {
            sort_by = SORT_BY_COUNTSEEN;
            descending = true;
        }

        return 0;
    }

private:
    GlobalRegistry *globalreg;
    dect_data *ddata;
};

bool less_by_RSSI(const vector<string> &v1, const vector<string> &v2)
{
    if (sort_by == SORT_BY_RFPI) {
        if (v1[sort_by].compare(v2[sort_by]) < 0) {
            return true;
        }
    } else {
        if (atoi(v1[sort_by].c_str()) < atoi(v2[sort_by].c_str())) {
            return true;
        }
    }
    return false; 
}

void DectDetailsProtoDECT(CLIPROTO_CB_PARMS) 
{
    KisPanelPluginData *pdata = (KisPanelPluginData *)auxptr;
    dect_data *ddata = (dect_data *)pdata->pluginaux;
    bool match = false;
    vector<string> inf;
    vector<string> showinf;
    string buf;
    stringstream ss(proto_string);

    // Dissect string at whitespace
    while (ss >> buf) {
        inf.push_back(buf);
    }
    // Niceify dates
    string f, l;
    time_t first, last;
    char first_s[30], last_s[30];
    first = atoi(inf[3].c_str());
    last = atoi(inf[4].c_str());
    ctime_r(&first, first_s);
    ctime_r(&last, last_s);
    inf[3] = first_s;
    inf[4] = last_s;
    
    if (mode == MODE_ASYNC_FP_SCAN) {
        vector<vector <string> >::iterator i = ddata->info_vec_fp.begin();
        for (int j = 0; i < ddata->info_vec_fp.end(); ++i, ++j) {
            if ((*i)[0] == inf[0]) {
                match = true;
                // Update
                ddata->info_vec_fp[j] = inf;
            }
        }
        if (!match) {
            ddata->info_vec_fp.push_back(inf);
        }
        sort(ddata->info_vec_fp.begin(), ddata->info_vec_fp.end(), less_by_RSSI);   
        if (descending) {
            reverse(ddata->info_vec_fp.begin(), ddata->info_vec_fp.end());
        }
    } else if (mode == MODE_ASYNC_PP_SCAN) {
        vector<vector <string> >::iterator i = ddata->info_vec_pp.begin();
        for (int j = 0; i < ddata->info_vec_pp.end(); ++i, ++j) {
            if ((*i)[0] == inf[0]) {
                match = true;
                // Update
                ddata->info_vec_pp[j] = inf;
            }
        }
        if (!match) {
            ddata->info_vec_pp.push_back(inf);
        }
        sort(ddata->info_vec_pp.begin(), ddata->info_vec_pp.end(), less_by_RSSI);   
        if (descending) {
            reverse(ddata->info_vec_pp.begin(), ddata->info_vec_pp.end());
        }
    }
    if (mode == MODE_ASYNC_FP_SCAN) {
            ddata->dtable->Clear();
            ddata->ctable->Hide();
            vector<vector <string> >::iterator i = ddata->info_vec_fp.begin();
            for (int j = 0; i < ddata->info_vec_fp.end(); ++i, ++j) {
                ddata->dtable->AddRow(j, (*i));
            }
            ddata->dtable->Show();
            ddata->dtable->DrawComponent();
    } else if (mode == MODE_ASYNC_PP_SCAN) {
            ddata->ctable->Clear();
            ddata->dtable->Hide();
            vector<vector <string> >::iterator i = ddata->info_vec_pp.begin();
            for (int j = 0; i < ddata->info_vec_pp.end(); ++i, ++j) {
                ddata->ctable->AddRow(j, (*i));
            }
            ddata->ctable->Show();
            ddata->ctable->DrawComponent();
    }
}

void DectCliConfigured(CLICONF_CB_PARMS) 
{
    KisPanelPluginData *pdata = (KisPanelPluginData *) auxptr;
    dect_data *ddata = (dect_data *) pdata->pluginaux;

    if (recon)
        return;

    if (kcli->RegisterProtoHandler("DECT", KCLI_DECT_CHANNEL_FIELDS,
                                   DectDetailsProtoDECT, pdata) < 0) {
        _MSG("Could not register DECT protocol with remote server", 
             MSGFLAG_ERROR);
    }
}

void DectCliAdd(KPI_ADDCLI_CB_PARMS) 
{
    KisPanelPluginData *pdata = (KisPanelPluginData *) auxptr;
    dect_data *adata = (dect_data *) pdata->pluginaux;

    if (add == 0)
        return;

    netcli->AddConfCallback(DectCliConfigured, 1, pdata);
}

// Menu event plugin
int menu_callback(void *auxptr) 
{
    /*
	((KisPanelPluginData *) auxptr)->kpinterface->RaiseAlert("Example",
		"Example plugin raising alert since \n"
		"you picked it from the menu.\n");
        */
	return 1;
}

int DectListerButtonCB(COMPONENT_CALLBACK_PARMS)
{
    dect_data *ddata = (dect_data *) aux;
    GlobalRegistry *greg = globalreg;

    if(!ddata || !greg) {
        return 0;
    }

    if (mode == MODE_ASYNC_FP_SCAN) {
        vector<string> data = ddata->dtable->GetSelectedData();    
        if (data.size() < 1) {
            // We got a button event even though the table was empty.
            string cmd("DECT 1 0 0");
            _MSG(cmd, MSGFLAG_INFO);
            if (globalreg &&
                globalreg->panel_interface &&
                globalreg->panel_interface->FetchFirstNetclient()) {
                globalreg->panel_interface->FetchFirstNetclient()->InjectCommand(cmd);
            }
            return 1;
        }
        vector<vector<string> >::iterator i = ddata->info_vec_fp.begin();
        for (int j = 0; i < ddata->info_vec_fp.end(); ++i, ++j) {
            if ((*i)[0] == data[0]) {
                ddata->sync_station = (*i);
                ddata->ctable->Hide();
                ddata->dtable->Clear();
                ddata->dtable->AddRow(0, (*i));
                ddata->dtable->Show();
                ddata->dtable->DrawComponent();
                string cmd("DECT 1 2 " + data[2] + " " + data[0]);
                _MSG(cmd, MSGFLAG_INFO);
                if (globalreg &&
                    globalreg->panel_interface &&
                    globalreg->panel_interface->FetchFirstNetclient()) {
                    globalreg->panel_interface->FetchFirstNetclient()->InjectCommand(cmd);
                }
                Kis_ModalAlert_Panel *ma = new Kis_ModalAlert_Panel(globalreg, globalreg->panel_interface);
                ma->Position((LINES / 2) - 8, (COLS / 2) - 25, 10, 50);
                ma->ConfigureAlert("", "Syncing to chosen station " + data[0]
                                   + "\n\nUse key 'F' to get back to FP scan"
                                   + "\nUse key 'A' to get back to PP scan");
                globalreg->panel_interface->AddPanel(ma);
                mode = MODE_SYNC_CALL_SCAN;
            }
        }
    } else  if (mode == MODE_ASYNC_PP_SCAN) { 
        vector<string> data = ddata->ctable->GetSelectedData();    
        if (data.size() < 1) {
            // We got a button event even though the table was empty.
            string cmd("DECT 1 1 0");
            _MSG(cmd, MSGFLAG_INFO);
            if (globalreg &&
                globalreg->panel_interface &&
                globalreg->panel_interface->FetchFirstNetclient()) {
                globalreg->panel_interface->FetchFirstNetclient()->InjectCommand(cmd);
            }
            return 1;
        }
        vector<vector<string> >::iterator i = ddata->info_vec_pp.begin();
        for (int j = 0; i < ddata->info_vec_pp.end(); ++i, ++j) {
            if ((*i)[0] == data[0]) {
                ddata->sync_station = (*i);
                ddata->ctable->Hide();
                ddata->dtable->Clear();
                ddata->dtable->AddRow(0, (*i));
                ddata->dtable->Show();
                ddata->dtable->DrawComponent();
                string cmd("DECT 1 2 " + data[2] + " " + data[0]);
                _MSG(cmd, MSGFLAG_INFO);
                if (globalreg &&
                    globalreg->panel_interface &&
                    globalreg->panel_interface->FetchFirstNetclient()) {
                    globalreg->panel_interface->FetchFirstNetclient()->InjectCommand(cmd);
                }
                Kis_ModalAlert_Panel *ma = new Kis_ModalAlert_Panel(globalreg, globalreg->panel_interface);
                ma->Position((LINES / 2) - 8, (COLS / 2) - 25, 10, 50);
                ma->ConfigureAlert("", "Syncing to chosen station " + data[0]
                                   + "\n\nUse key 'F' to get back to FP scan"
                                   + "\nUse key 'A' to get back to PP scan");
                globalreg->panel_interface->AddPanel(ma);
                mode = MODE_SYNC_CALL_SCAN;
            }
        }
    } else if (mode == MODE_SYNC_CALL_SCAN) {
        vector<string> data = ddata->dtable->GetSelectedData();    
        if (data.size() < 1) {
            // We got a button event even though the table was empty.
            string cmd("DECT 1 0 0");
            _MSG(cmd, MSGFLAG_INFO);
            if (globalreg &&
                globalreg->panel_interface &&
                globalreg->panel_interface->FetchFirstNetclient()) {
                globalreg->panel_interface->FetchFirstNetclient()->InjectCommand(cmd);
            }
            return 1;
        }
        Kis_ModalAlert_Panel *ma = new Kis_ModalAlert_Panel(globalreg, globalreg->panel_interface);
        ma->Position((LINES / 2) - 5, (COLS / 2) - 25, 10, 50);
        if (ddata->sync_station.size() < 1) {
            _MSG("No synced station recored in sync mode", MSGFLAG_ERROR);
            return 1;
        }
        if (data[0] == ddata->sync_station[0]) {
            ma->ConfigureAlert("", "Already synced to station " + data[0]
                                   + "\n\nUse key 'F' to get back to FP scan"
                                   + "\nUse key 'A' to get back to PP scan");
        } else {
            ma->ConfigureAlert("", "Not syncing to station " + data[0]
                                   + ", already synced to "
                                   + ddata->sync_station[0]
                                   + "\n\nUse key 'F' to get back to FP scan"
                                   + "\nUse key 'A' to get back to PP scan");
        }
        globalreg->panel_interface->AddPanel(ma);
    }

    return 0;
}

// Init plugin gets called when plugin loads
extern "C" {

int panel_plugin_init(GlobalRegistry *globalreg, KisPanelPluginData *pdata) {
    dect_data *ddata = new dect_data;
    ddata->numrows = 0;

    pdata->pluginaux = (void *)ddata;
	_MSG("Loading DECT plugin", MSGFLAG_INFO);

	pdata->mainpanel->AddPluginMenuItem("DECT Plugin", menu_callback, pdata);
    ddata->dtable = new DecTable(globalreg, pdata->mainpanel, ddata);
    ddata->ctable = new DecTable(globalreg, pdata->mainpanel, ddata);

    vector<Kis_Scrollable_Table::title_data> ti;
    Kis_Scrollable_Table::title_data t1;
    t1.width = 15;
    t1.draw_width = 15;
    t1.title = "RFPI";
    t1.alignment = 15;
    ti.push_back(t1);

    Kis_Scrollable_Table::title_data t2;
    t2.width = 5;
    t2.draw_width = 5;
    t2.title = "RSSI";
    t2.alignment = 5;
    ti.push_back(t2);

    Kis_Scrollable_Table::title_data t3;
    t3.width = 4;
    t3.draw_width = 4;
    t3.title = "Ch";
    t3.alignment = 4;
    ti.push_back(t3);

    Kis_Scrollable_Table::title_data t4;
    t4.width = 20;
    t4.draw_width = 8;
    t4.title = "First";
    t4.alignment = 8;
    ti.push_back(t4);

    Kis_Scrollable_Table::title_data t5;
    t5.width = 20;
    t5.draw_width = 8;
    t5.title = "Last";
    t5.alignment = 8;
    ti.push_back(t5);

    Kis_Scrollable_Table::title_data t6;
    t6.width = 10;
    t6.draw_width = 10;
    t6.title = "Seen";
    t6.alignment = 10;
    ti.push_back(t6);

    ddata->dtable->AddTitles(ti);
    ddata->ctable->AddTitles(ti);
    pdata->mainpanel->AddComponentVec(ddata->dtable, (KIS_PANEL_COMP_DRAW | 
                                                      KIS_PANEL_COMP_TAB | 
													  KIS_PANEL_COMP_EVT));
    pdata->mainpanel->AddComponentVec(ddata->ctable, (KIS_PANEL_COMP_DRAW | 
                                                      KIS_PANEL_COMP_TAB | 
													  KIS_PANEL_COMP_EVT));
    pdata->mainpanel->FetchNetBox()->Pack_After_Named("KIS_MAIN_NETLIST",
                                                      ddata->dtable, 1, 0);
    pdata->mainpanel->FetchNetBox()->Pack_After_Named("KIS_MAIN_NETLIST",
                                                      ddata->ctable, 1, 0);

    ddata->dtable->Activate(1);
    ddata->ctable->Activate(1);
    ddata->dtable->Show();
    ddata->dtable->DrawComponent();
    // Callback shows details on the station list
    ddata->dtable->SetCallback(COMPONENT_CBTYPE_ACTIVATED, 
                               DectListerButtonCB, ddata);
    ddata->ctable->SetCallback(COMPONENT_CBTYPE_ACTIVATED, 
                               DectListerButtonCB, ddata);
    ddata->addref =
       pdata->kpinterface->Add_NetCli_AddCli_CB(DectCliAdd, (void *) pdata);

	return 1;
}

}

