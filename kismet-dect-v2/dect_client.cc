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
#include <kis_panel_widgets.h>

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

struct dect_data {
    Kis_Scrollable_Table *dtable;
    Kis_Free_Text *modetxt;

    vector<vector <string> > info_vec_fp; 
    vector<vector <string> > info_vec_pp;
    vector<string> sync_station;
    int addref;
    int numrows;

    int view;

    int mn_dect, mi_show, mi_showd, mi_showc, mi_fpscan, mi_ppscan, 
        mi_sort_rfpi, mi_sort_rfpi_desc, mi_sort_rssi, mi_sort_rssi_desc, 
        mi_sort_channel, mi_sort_view, mi_sort_view_desc;
    Kis_Menu *menu;

    KisPanelPluginData *pdata;
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

        ddata->dtable->Clear();

        if (mode == MODE_ASYNC_FP_SCAN) {
		vector<vector <string> >::iterator i = ddata->info_vec_fp.begin();
		for (int j = 0; i < ddata->info_vec_fp.end(); ++i, ++j) {
			ddata->dtable->AddRow(j, (*i));
		}
	} else if (mode == MODE_ASYNC_PP_SCAN || mode == MODE_SYNC_CALL_SCAN) {
		vector<vector <string> >::iterator i = ddata->info_vec_pp.begin();
		for (int j = 0; i < ddata->info_vec_pp.end(); ++i, ++j) {
			ddata->dtable->AddRow(j, (*i));
		}
        }

        ddata->dtable->DrawComponent();
}

void dect_prompt_nodect(KIS_PROMPT_CB_PARMS) {
	if (check) 
		globalreg->panel_interface->prefs->SetOpt("PLUGIN_DECT_WARNDECT", "false", 1);
}

void DectCliConfigured(CLICONF_CB_PARMS) 
{
    KisPanelPluginData *pdata = (KisPanelPluginData *) auxptr;
    dect_data *ddata = (dect_data *) pdata->pluginaux;

    if (kcli->RegisterProtoHandler("DECT", KCLI_DECT_CHANNEL_FIELDS,
                                   DectDetailsProtoDECT, pdata) < 0) {
        _MSG("Could not register DECT protocol with remote server", 
             MSGFLAG_ERROR);

		if (globalreg->panel_interface->prefs->FetchOpt("PLUGIN_DECT_WARNDECT") == "" ||
			globalreg->panel_interface->prefs->FetchOpt("PLUGIN_DECT_WARNDECT") == 
																			"true") {
			vector<string> t;
			t.push_back("No support for the DECT protocol on the Kismet");
			t.push_back("server.  The Dedected Kismet server plugin must");
			t.push_back("be loaded as well.");

			Kis_Prompt_Panel *kpp =
				new Kis_Prompt_Panel(globalreg, globalreg->panel_interface);
			kpp->SetTitle("No DECT support in server");
			kpp->SetDisplayText(t);
			kpp->SetCheckText("Do not show this warning in the future");
			kpp->SetChecked(0);
			kpp->SetButtonText("OK", "");
			kpp->SetCallback(dect_prompt_nodect, NULL);
			globalreg->panel_interface->QueueModalPanel(kpp);
		}
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

int DectDListerButtonCB(COMPONENT_CALLBACK_PARMS)
{
    dect_data *ddata = (dect_data *) aux;

	// If in basestation scan mode, switch to call scan mode
    if (mode == MODE_ASYNC_FP_SCAN) {
        vector<string> data = ddata->dtable->GetSelectedData();    

        if (data.size() < 1) {
            // We got a button event even though the table was empty.
			// Re-send the PP scan command
            string cmd("DECT 1 0 0");

            if (globalreg &&
                globalreg->panel_interface &&
                globalreg->panel_interface->FetchNetClient()) {
                globalreg->panel_interface->FetchNetClient()->InjectCommand(cmd);
            }

            return 1;
        }

        vector<vector<string> >::iterator i = ddata->info_vec_fp.begin();

		// Switch to 
        for (int j = 0; i < ddata->info_vec_fp.end(); ++i, ++j) {
            if ((*i)[0] == data[0]) {
                ddata->sync_station = (*i);

                ddata->dtable->AddRow(0, (*i));
                ddata->dtable->DrawComponent();

                string cmd("DECT 1 2 " + data[2] + " " + data[0]);

                if (globalreg &&
                    globalreg->panel_interface &&
                    globalreg->panel_interface->FetchNetClient()) {
                    globalreg->panel_interface->FetchNetClient()->InjectCommand(cmd);
                }

				ddata->modetxt->SetText("Syncing calls on station " + data[0]);

                mode = MODE_SYNC_CALL_SCAN;
            }
        }
	}

    return 0;
}

void DectMenuCB(MENUITEM_CB_PARMS) {
	dect_data *ddata = (dect_data *) auxptr;

	if (menuitem == ddata->mi_show) {
		if (ddata->pdata->kpinterface->prefs->FetchOpt("PLUGIN_DECT_SHOW") == "true" ||
			ddata->pdata->kpinterface->prefs->FetchOpt("PLUGIN_DECT_SHOW") == "") {

			ddata->pdata->kpinterface->prefs->SetOpt("PLUGIN_DECT_SHOW", "false", 1);

			ddata->dtable->Hide();
			ddata->modetxt->Hide();

			ddata->menu->SetMenuItemChecked(ddata->mi_show, 0);
			ddata->menu->DisableAllItems(ddata->mn_dect);
			ddata->menu->EnableMenuItem(ddata->mi_show);
		} else {
			ddata->pdata->kpinterface->prefs->SetOpt("PLUGIN_DECT_SHOW", "true", 1);

			ddata->dtable->Show();
			ddata->modetxt->Show();

			ddata->menu->SetMenuItemChecked(ddata->mi_show, 1);
			ddata->menu->EnableAllItems(ddata->mn_dect);
		}
	} else if (menuitem == ddata->mi_sort_rfpi) {
		sort_by = SORT_BY_RFPI;
		descending = false;
		ddata->pdata->kpinterface->prefs->SetOpt("PLUGIN_DECT_SORT", "rfpi", 1);
	} else if (menuitem == ddata->mi_sort_rfpi_desc) {
		sort_by = SORT_BY_RFPI;
		descending = true;
		ddata->pdata->kpinterface->prefs->SetOpt("PLUGIN_DECT_SORT", "rfpi_desc", 1);
	} else if (menuitem == ddata->mi_sort_rssi) {
		sort_by = SORT_BY_RSSI;
		descending = false;
		ddata->pdata->kpinterface->prefs->SetOpt("PLUGIN_DECT_SORT", "rssi", 1);
	} else if (menuitem == ddata->mi_sort_rssi_desc) {
		sort_by = SORT_BY_RSSI;
		descending = true;
		ddata->pdata->kpinterface->prefs->SetOpt("PLUGIN_DECT_SORT", "rssi_desc", 1);
	} else if (menuitem == ddata->mi_sort_channel) {
		sort_by = SORT_BY_CHANNEL;
		descending = false;
		ddata->pdata->kpinterface->prefs->SetOpt("PLUGIN_DECT_SORT", "channel", 1);
	} else if (menuitem == ddata->mi_sort_view) {
		sort_by = SORT_BY_COUNTSEEN;
		descending = false;
		ddata->pdata->kpinterface->prefs->SetOpt("PLUGIN_DECT_SORT", "view", 1);
	} else if (menuitem == ddata->mi_sort_view_desc) {
		sort_by = SORT_BY_COUNTSEEN;
		descending = true;
		ddata->pdata->kpinterface->prefs->SetOpt("PLUGIN_DECT_SORT", "view_desc", 1);
	} else if (menuitem == ddata->mi_ppscan) {
		if (globalreg->panel_interface->FetchNetClient()) {
			globalreg->panel_interface->FetchNetClient()->InjectCommand("DECT 1 0 0");
		}
		mode = MODE_ASYNC_PP_SCAN;
		ddata->info_vec_pp.clear();
		ddata->modetxt->SetText("DECT: Scanning Calls");
		ddata->dtable->Clear();
	} else if (menuitem == ddata->mi_fpscan) {
		if (globalreg->panel_interface->FetchNetClient()) {
			globalreg->panel_interface->FetchNetClient()->InjectCommand("DECT 1 1 0");
		}

		mode = MODE_ASYNC_FP_SCAN;
		ddata->modetxt->SetText("DECT: Scanning Basestations");
		ddata->info_vec_fp.clear();
		ddata->dtable->Clear();
	}

	string s = StrLower(ddata->pdata->kpinterface->prefs->FetchOpt("PLUGIN_DECT_SORT"));

	if (s == "") {
		s = "channel";
		sort_by = SORT_BY_CHANNEL;
		descending = false;
	}

	ddata->menu->SetMenuItemChecked(ddata->mi_sort_rfpi, 0);
	ddata->menu->SetMenuItemChecked(ddata->mi_sort_rfpi_desc, 0);
	ddata->menu->SetMenuItemChecked(ddata->mi_sort_rssi, 0);
	ddata->menu->SetMenuItemChecked(ddata->mi_sort_rssi_desc, 0);
	ddata->menu->SetMenuItemChecked(ddata->mi_sort_channel, 0);
	ddata->menu->SetMenuItemChecked(ddata->mi_sort_view, 0);
	ddata->menu->SetMenuItemChecked(ddata->mi_sort_view_desc, 0);

	if (s == "rfpi")
		ddata->menu->SetMenuItemChecked(ddata->mi_sort_rfpi_desc, 1);
	else if (s == "rfpi_desc")
		ddata->menu->SetMenuItemChecked(ddata->mi_sort_rfpi, 1);
	else if (s == "rssi")
		ddata->menu->SetMenuItemChecked(ddata->mi_sort_rssi, 1);
	else if (s == "rssi_desc")
		ddata->menu->SetMenuItemChecked(ddata->mi_sort_rssi_desc, 1);
	else if (s == "view") 
		ddata->menu->SetMenuItemChecked(ddata->mi_sort_view, 1);
	else if (s == "view_desc")
		ddata->menu->SetMenuItemChecked(ddata->mi_sort_view_desc, 1);
	else 
		ddata->menu->SetMenuItemChecked(ddata->mi_sort_channel, 1);
}

// Init plugin gets called when plugin loads
extern "C" {

int panel_plugin_init(GlobalRegistry *globalreg, KisPanelPluginData *pdata) {
    dect_data *ddata = new dect_data;

	ddata->pdata = pdata;

    ddata->numrows = 0;

	ddata->view = 0;

    pdata->pluginaux = (void *)ddata;
	_MSG("Loading DECT plugin", MSGFLAG_INFO);

    ddata->dtable = new Kis_Scrollable_Table(globalreg, pdata->mainpanel);

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
    pdata->mainpanel->AddComponentVec(ddata->dtable, 
									  (KIS_PANEL_COMP_DRAW | KIS_PANEL_COMP_TAB | 
									   KIS_PANEL_COMP_EVT));

	ddata->modetxt = new Kis_Free_Text(globalreg, pdata->mainpanel);
	ddata->modetxt->SetText("DECT: Scanning Basestations");
	ddata->modetxt->SetPreferredSize(0, 1);

    pdata->mainpanel->FetchNetBox()->Pack_End(ddata->dtable, 1, 0);
	pdata->mainpanel->FetchNetBox()->Pack_End(ddata->modetxt, 0, 0);

    // Callback shows details on the station list
    ddata->dtable->SetCallback(COMPONENT_CBTYPE_ACTIVATED, 
                               DectDListerButtonCB, ddata);

    ddata->addref =
       pdata->kpinterface->Add_NetCli_AddCli_CB(DectCliAdd, (void *) pdata);

	Kis_Menu *menu = pdata->kpinterface->FetchMainPanel()->FetchMenu();

	ddata->mn_dect = menu->AddMenu("DECT", -1);
	
	ddata->mi_show = menu->AddMenuItem("Show DECT", ddata->mn_dect, 'D');
	menu->SetMenuItemCallback(ddata->mi_show, DectMenuCB, ddata);

	menu->AddMenuItem("-", ddata->mn_dect, 0);
	ddata->mi_fpscan = menu->AddMenuItem("Basestation Scan", ddata->mn_dect, 'F');
	menu->SetMenuItemCallback(ddata->mi_fpscan, DectMenuCB, ddata);
	ddata->mi_ppscan = menu->AddMenuItem("Call Scan", ddata->mn_dect, 'P');
	menu->SetMenuItemCallback(ddata->mi_ppscan, DectMenuCB, ddata);

	menu->AddMenuItem("-", ddata->mn_dect, 0);

	ddata->mi_sort_channel = menu->AddMenuItem("Sort Channel", ddata->mn_dect, 'c');
	menu->SetMenuItemCallback(ddata->mi_sort_channel, DectMenuCB, ddata);
	menu->SetMenuItemCheckSymbol(ddata->mi_sort_channel, '*');
	ddata->mi_sort_rfpi = menu->AddMenuItem("Sort RFPI", ddata->mn_dect, 'R');
	menu->SetMenuItemCallback(ddata->mi_sort_rfpi, DectMenuCB, ddata);
	menu->SetMenuItemCheckSymbol(ddata->mi_sort_rfpi, '*');
	ddata->mi_sort_rfpi_desc = menu->AddMenuItem("Sort Reverse RFPI", 
												ddata->mn_dect, 'r');
	menu->SetMenuItemCallback(ddata->mi_sort_rfpi_desc, DectMenuCB, ddata);
	menu->SetMenuItemCheckSymbol(ddata->mi_sort_rfpi_desc, '*');
	ddata->mi_sort_rssi = menu->AddMenuItem("Sort RSSI", ddata->mn_dect, 'S');
	menu->SetMenuItemCallback(ddata->mi_sort_rssi, DectMenuCB, ddata);
	menu->SetMenuItemCheckSymbol(ddata->mi_sort_rssi, '*');
	ddata->mi_sort_rssi_desc = menu->AddMenuItem("Sort Reverse RSSI", 
												 ddata->mn_dect, 's');
	menu->SetMenuItemCallback(ddata->mi_sort_rssi_desc, DectMenuCB, ddata);
	menu->SetMenuItemCheckSymbol(ddata->mi_sort_rssi_desc, '*');
	ddata->mi_sort_view = menu->AddMenuItem("Sort Count", ddata->mn_dect, 'V');
	menu->SetMenuItemCallback(ddata->mi_sort_view, DectMenuCB, ddata);
	menu->SetMenuItemCheckSymbol(ddata->mi_sort_view, '*');
	ddata->mi_sort_view_desc = menu->AddMenuItem("Sort Reverse View Count", 
												 ddata->mn_dect, 'V');
	menu->SetMenuItemCallback(ddata->mi_sort_view_desc, DectMenuCB, ddata);
	menu->SetMenuItemCheckSymbol(ddata->mi_sort_view_desc, '*');

	ddata->menu = menu;

	menu->SetMenuItemChecked(ddata->mi_showd, 1);
	menu->SetMenuItemChecked(ddata->mi_showc, 0);
	ddata->dtable->Hide();
	ddata->modetxt->Hide();

	if (pdata->kpinterface->prefs->FetchOpt("PLUGIN_DECT_SHOW") == "true" ||
		pdata->kpinterface->prefs->FetchOpt("PLUGIN_DECT_SHOW") == "") {

		menu->SetMenuItemChecked(ddata->mi_show, 1);

		menu->EnableAllItems(ddata->mn_dect);

		ddata->dtable->Show();
		ddata->modetxt->Show();

	} else {
		menu->SetMenuItemChecked(ddata->mi_show, 0);
		menu->DisableAllItems(ddata->mn_dect);
		menu->EnableMenuItem(ddata->mi_show);
	}
	
	// Fire the menu handler to update sort
	DectMenuCB(globalreg, -1, ddata);

	return 1;
}

}

