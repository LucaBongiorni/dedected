/* kismet dect client plugin 
 * (c) 2008 by Mike Kershaw <dragorn (at) kismetwireless (dot) net,
 *             Jake Appelbaum <ioerror (at) appelbaum (dot) net,
 *             Christian Fromme <kaner (at) strace (dot) org
 *
 * Don't distribute (yet)
 */

#include <config.h>

#include <stdio.h>
#include <string.h>

#include <globalregistry.h>

#include <kis_panel_plugin.h>
#include <kis_panel_frontend.h>
#include <kis_panel_windows.h>

#define KCLI_DECT_CHANNEL_FIELDS "rfpi,rssi,channel,first_seen,last_seen,count_seen"

#define SORT_BY_RSSI 1
#define SORT_BY_CHANNEL 2
#define SORT_BY_COUNTSEEN 5

#define MODE_ASYNC_FP_SCAN 0
#define MODE_ASYNC_PP_SCAN 1

// default: sort by RSSI
static int sort_by = SORT_BY_RSSI;
static bool descending = true;
static int mode = MODE_ASYNC_FP_SCAN;

class DecTable : public Kis_Scrollable_Table {
public:
    DecTable(GlobalRegistry *in_globalreg, Kis_Panel *in_panel) :
        Kis_Scrollable_Table(in_globalreg, in_panel)
    {
        this->globalreg = in_globalreg;
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
                               "<enter> - Show details of currently selected station\n"
                               "L       - Lock channel hopping to current channel\n"
                               "U       - Unlock channel hopping\n"
                               //"S       - Set (and lock) channel\n"
                               "F       - Do async FP scan (default)\n"
                               "A       - Do async call scan\n"
                               //"P       - PP scan for active calls\n"
                               "r       - Sort by RSSI (ascending)\n"
                               "R       - Sort by RSSI (descending)\n"
                               "c       - Sort by Channel (ascending)\n"
                               "C       - Sort by Channel (descending)\n"
                               "s       - Sort by view count (ascending)\n"
                               "S       - Sort by view count (descending)\n";
                               
            Kis_ModalAlert_Panel *ma = new Kis_ModalAlert_Panel(globalreg, globalreg->panel_interface);
            ma->Position(2, 2, 19, 55);
            ma->ConfigureAlert(help_title, help_text);
            globalreg->panel_interface->AddPanel(ma);
            return 0;
        }
        if (in_key == 'L') {
            // Return currently selected channel to server
            vector<string> s = GetSelectedData();
            string cmd("DECT 1 0 0 " + s[2]);
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
                globalreg->panel_interface->FetchFirstNetclient()->InjectCommand("DECT 1 0 1");
            }
            return 0;
        }
        if (in_key == 'F') {
            if (globalreg && 
                globalreg->panel_interface && 
                globalreg->panel_interface->FetchFirstNetclient()) {
                globalreg->panel_interface->FetchFirstNetclient()->InjectCommand("DECT 1 1 0");
            }
            mode = MODE_ASYNC_FP_SCAN;
            return 0;
        }
        if (in_key == 'A') {
            if (globalreg && 
                globalreg->panel_interface && 
                globalreg->panel_interface->FetchFirstNetclient()) {
                globalreg->panel_interface->FetchFirstNetclient()->InjectCommand("DECT 1 1 1");
            }
            mode = MODE_ASYNC_PP_SCAN;
            return 0;
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
};

struct dect_data {
    DecTable *dtable;
    DecTable *ctable;
    // This should be something else than just strings,
    // but it's too handy at the moment since we have
    // to put strings into 
    vector<vector <string> > info_vec; 
    int addref;
    int numrows;
};

bool less_by_RSSI(const vector<string> &v1, const vector<string> &v2)
{
    if (atoi(v1[sort_by].c_str()) < atoi(v2[sort_by].c_str())) {
        return true;
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
    /*
    string f, l;
    time_t first, last;
    char first_s[30], last_s[30];
    first = atoi(inf[3].c_str());
    last = atoi(inf[4].c_str());
    ctime_r(&first, first_s);
    ctime_r(&last, last_s);
    inf[3] = first_s;
    inf[4] = last_s;
    */
    
    // RFPI is primary key
    vector<vector <string> >::iterator i = ddata->info_vec.begin();
    for (int j = 0; i < ddata->info_vec.end(); ++i, ++j) {
        if ((*i)[0] == inf[0]) {
            match = true;
            // Update
            ddata->info_vec[j] = inf;
            //ddata->dtable->ReplaceRow(j, inf);
        }
    }
    if (!match) {
        ddata->info_vec.push_back(inf);
        //ddata->dtable->AddRow(ddata->info_vec.size(), inf);
    }
    sort(ddata->info_vec.begin(), ddata->info_vec.end(), less_by_RSSI);   
    if (descending) {
        reverse(ddata->info_vec.begin(), ddata->info_vec.end());
    }
    if (mode == MODE_ASYNC_FP_SCAN) {
        ddata->dtable->Clear();
        i = ddata->info_vec.begin();
        for (int j = 0; i < ddata->info_vec.end(); ++i, ++j) {
            ddata->dtable->AddRow(j, (*i));
        }
        ddata->dtable->Show();
        ddata->dtable->DrawComponent();
    } else {
        ddata->ctable->Clear();
        i = ddata->info_vec.begin();
        for (int j = 0; i < ddata->info_vec.end(); ++i, ++j) {
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
        _MSG("Could not register DECT protocol with remote server", MSGFLAG_ERROR);
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
    vector<string> data = ddata->dtable->GetSelectedData();    

    vector<vector<string> >::iterator i = ddata->info_vec.begin();
    for (int j = 0; i < ddata->info_vec.end(); ++i, ++j) {
        if ((*i)[0] == data[0]) {
            time_t first, last;
            char first_s[30], last_s[30];
            first = atoi((*i)[3].c_str());
            last = atoi((*i)[4].c_str());
            ctime_r(&first, first_s);
            ctime_r(&last, last_s);
            string in_title = "Station details";
            string in_text =  "Station:    " + data[0] + "\n" + 
                    "RSSI:       " + (*i)[1] + "\n" +
                    "Channel:    " + (*i)[2] + "\n" +
                    "First seen: " + first_s +
                    "Last seen:  " + last_s +
                    "Count seen: " + (*i)[5] + "\n";
            // XXX: This could be another panel as well
            Kis_ModalAlert_Panel *ma = new Kis_ModalAlert_Panel(globalreg, globalreg->panel_interface);
            ma->Position((LINES / 2) - 5, (COLS / 2) - 20, 12, 40);
            ma->ConfigureAlert(in_title, in_text);
            globalreg->panel_interface->AddPanel(ma);
            return 1;
        }
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
    ddata->dtable = new DecTable(globalreg, pdata->mainpanel);
    ddata->ctable = new DecTable(globalreg, pdata->mainpanel);

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
    t4.width = 16;
    t4.draw_width = 8;
    t4.title = "First";
    t4.alignment = 8;
    ti.push_back(t4);

    Kis_Scrollable_Table::title_data t5;
    t5.width = 16;
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
    pdata->mainpanel->AddComponentVec(ddata->dtable, (KIS_PANEL_COMP_DRAW | 
                                                      KIS_PANEL_COMP_TAB | 
													  KIS_PANEL_COMP_EVT));
    pdata->mainpanel->FetchNetBox()->Pack_After_Named("KIS_MAIN_NETLIST",
                                                      ddata->dtable, 1, 0);

    ddata->dtable->Activate(1);
    ddata->dtable->Show();
    ddata->dtable->DrawComponent();
    // Callback shows details on the station list
    ddata->dtable->SetCallback(COMPONENT_CBTYPE_ACTIVATED, 
                               DectListerButtonCB, ddata);
    ddata->addref =
       pdata->kpinterface->Add_NetCli_AddCli_CB(DectCliAdd, (void *) pdata);

	return 1;
}

}

