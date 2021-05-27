#include "logboek.h"
#include "manager.h"

#ifdef flagIMPORT
#include "import/import.h"
#endif

ManagerTab::ManagerTab()
{
	hasActiveColumn = false;
	b_add <<= THISBACK(_AddRecord);
	b_edit <<= THISBACK(_EditRecord);
	b_remove <<= THISBACK(_RemoveRecord);
	//editlist.MultiSelect();
	editlist.WhenBar = THISBACK(PopupMenu);
	editlist.WhenLeftDouble = THISBACK(_EditRecord);
#ifdef _DEBUG
	editlist.WhenHeaderLayout = THISBACK (_WhenHeaderLayout);
#endif
}

void ManagerTab::_AddRecord()
{
	try {
		int id = Null;
		EditRecord(id);
		if (!IsNull(id)) editlist.FindSetCursor(id);
	}
	catch(SqlExc& e) {
		ExceptionMessage("adding record in manager", e);
	}

}

void ManagerTab::_EditRecord()
{
	try {
		int id = editlist.GetKey();
		EditRecord(id);
		editlist.FindSetCursor(id);
	}
	catch(SqlExc& e) {
		ExceptionMessage("editing record in manager", e);
	}
}

void ManagerTab::_RemoveRecord()
{
	try {
		RemoveRecord();
	}
	catch(SqlExc& e) {
		ExceptionMessage("removing record in manager", e);
	}
}

void ManagerTab::PopupMenu(Bar& bar)
{
	bar.Add("Toevoegen", THISBACK(_AddRecord))
		.Key(K_INSERT)
		.Image(LogboekImg::add_16());
	bar.Add(editlist.IsCursor(), "Bewerken...", THISBACK(_EditRecord))
		.Key(K_ENTER)
		.Image(LogboekImg::form_edit_16());
	bar.Add(editlist.IsCursor(), "Verwijderen...", THISBACK(_RemoveRecord))
		.Key(K_DELETE)
		.Image(LogboekImg::delete_16());
	if (hasActiveColumn) {
		bool isActive = false;
		if (editlist.IsCursor()) {
			Value v = editlist.Get(editlist.GetCursor(), editlist.GetPos(ActiveId));
			if (v=="1" || v==true) isActive=true;
		}
		bar.Separator();
		bar.Add(editlist.IsCursor(), "Actief", THISBACK1(SetActive, !isActive))
			.Check(isActive)
			.Key(K_CTRL_SPACE);
	}
}

void ManagerTab::AddActiveColumn(const SqlId& Id)
{
	ActiveId = Id;
	editlist.AddColumn(Id, "Actief", 20)
		.SetConvert(Single<ConvertBool>())
		//.Ctrls<Option>()
		.Sorting();
	hasActiveColumn = true;
}

void ManagerTab::SetActive(Value v)
{
	try {
		if(editlist.IsCursor()) {
			editlist.Set(editlist.GetCursor(),editlist.GetPos(ActiveId),v);
	/*
			SQL & ::Update(PERSONS)
				(P_ACTIVE, v)
				.Where(P_ID == editlist.GetCursor());
	*/
		}
	}
	catch(SqlExc& e) {
		ExceptionMessage("updating: ", e);
		return;
	}
}



/////////////////////////////////////////////////////////
//
//  ManagerDlg implementation
//

#ifdef flagIMPORT
void ManagerDlg::import()
{
	import_logboek_mdb();

	// database has probably changed, so update cache and editlists
	try {
		dbCache.UpdateCache(typeAll);
		personsTab.editlist.ReQuery();
		activitiesTab.editlist.ReQuery();
		issuetypesTab.editlist.ReQuery();
		linesTab.editlist.ReQuery();
		machinesTab.editlist.ReQuery();
		machinetypesTab.editlist.ReQuery();
		departmentsTab.editlist.ReQuery();
		issuesTab.editlist.Clear();
		issuesLoaded = false;
	}
	catch(SqlExc& e) {
		ExceptionMessage("ReQuerying Manager", e);
		return;
	}
}
#endif

void ManagerDlg::WhenTabAction() {
	if(!issuesLoaded && issuesTab.editlist.IsVisible()) {
		WaitCursor waitcursor;
		statusbar = "Laden gegevens...";
		Sync();
		issuesTab.editlist.ReQuery();
		issuesLoaded = true;
		statusbar = "";
	}
}

void ManagerDlg::ManagerMenu(Bar& bar)
{
	bar.Add("Sluiten", THISBACK(Close))
		//.Image(LogboekImg::door_close)
		.Help("Venster sluiten");
}
void ManagerDlg::RecordMenu(Bar& bar)
{
	if (ManagerTabs.IsAt(personsTab)) personsTab.PopupMenu(bar);
	else if (ManagerTabs.IsAt(activitiesTab)) activitiesTab.PopupMenu(bar);
	else if (ManagerTabs.IsAt(issuetypesTab)) issuetypesTab.PopupMenu(bar);
	else if (ManagerTabs.IsAt(linesTab)) linesTab.PopupMenu(bar);
	else if (ManagerTabs.IsAt(machinesTab)) machinesTab.PopupMenu(bar);
	else if (ManagerTabs.IsAt(machinetypesTab)) machinetypesTab.PopupMenu(bar);
	else if (ManagerTabs.IsAt(departmentsTab)) departmentsTab.PopupMenu(bar);
	else if (ManagerTabs.IsAt(issuesTab)) issuesTab.PopupMenu(bar);
}

void ManagerDlg::ExtraMenu(Bar& bar)
{
#ifdef flagIMPORT
	bar.Add("Importeer STEWAD...", THISBACK(import))
		.Image(LogboekImg::Import_16x16())
		.Help("Importeer oud STEWAD logboek (.MDB)");
#endif

#ifdef _DEBUG
	bar.Add("SQL Commander...", callback(SQLCommander))
		.Image(LogboekImg::SqlConsoleIconSmall())
		.Help("Voer SQL commando's uit op database");
#endif
}

void ManagerDlg::MainMenu(Bar& bar)
{
	bar.Add("Beheer", THISBACK(ManagerMenu));
	bar.Add("Record", THISBACK(RecordMenu));
	bar.Add("Extra", THISBACK(ExtraMenu));
}

ManagerDlg::ManagerDlg()
{
	CtrlLayout(*this, "");
	Title("Storing database manager").Sizeable().MaximizeBox().MinimizeBox();

	AddFrame(statusbar);
	AddFrame(menubar);
	menubar.Set(THISBACK(MainMenu));
	menubar.WhenHelp = statusbar;

	CtrlLayout(personsTab);
	CtrlLayout(activitiesTab);
	CtrlLayout(issuetypesTab);
	CtrlLayout(linesTab);
	CtrlLayout(machinesTab);
	CtrlLayout(machinetypesTab);
	CtrlLayout(departmentsTab);
	CtrlLayout(issuesTab);

	ManagerTabs.Add(departmentsTab.SizePos(), "Afdelingen");
	ManagerTabs.Add(machinetypesTab.SizePos(), "Machine types");
	ManagerTabs.Add(linesTab.SizePos(), "Lijnen");
	ManagerTabs.Add(machinesTab.SizePos(), "Machines");
	ManagerTabs.Add(personsTab.SizePos(), "Monteurs");
	ManagerTabs.Add(activitiesTab.SizePos(), "Type");
	ManagerTabs.Add(issuetypesTab.SizePos(), "Soort");
	ManagerTabs.Add(issuesTab.SizePos(), "Storingen");
	ManagerTabs.WhenAction = THISBACK(WhenTabAction);

	try //load from SQL
	{
		personsTab.EditRecord = THISBACK(EditPerson);
		personsTab.RemoveRecord = THISBACK(RemovePerson);
		personsTab.editlist.SetTable(PERSONS);
		personsTab.editlist.AddKey(P_ID);
		personsTab.AddActiveColumn(P_ACTIVE);
		personsTab.editlist.AddColumn(P_NUMBER, "Kloknummer").Sorting();
		personsTab.editlist.AddColumn(FIRST_NAME, "Voornaam").Sorting();
		personsTab.editlist.AddColumn(MID_NAME, "Tussenvoegsel");
		personsTab.editlist.AddColumn(LAST_NAME, "Achternaam").Sorting();
		personsTab.editlist.SetOrderBy(P_ACTIVE);
		personsTab.editlist.SetSortColumn(0,true);
		personsTab.editlist.ColumnWidths("50 80 95 95 277");

		activitiesTab.EditRecord = THISBACK(EditActivity);
		activitiesTab.RemoveRecord = THISBACK(RemoveActivity);
		activitiesTab.editlist.SetTable(ACTIVITIES);
		activitiesTab.editlist.AddKey(ACT_ID);
		activitiesTab.AddActiveColumn(ACT_ACTIVE);
		activitiesTab.editlist.AddColumn(ACT_SHORT, "Code").Sorting();
		activitiesTab.editlist.AddColumn(ACT_LONG, "Omschrijving").Sorting();
		activitiesTab.editlist.SetOrderBy(ACT_SHORT);
		activitiesTab.editlist.SetSortColumn(0,true);
		activitiesTab.editlist.ColumnWidths("50 60 487");

		issuetypesTab.EditRecord = THISBACK(EditIssuetype);
		issuetypesTab.RemoveRecord = THISBACK(RemoveIssuetype);
		issuetypesTab.editlist.SetTable(ISSUETYPES);
		issuetypesTab.editlist.AddKey(TYPE_ID);
		issuetypesTab.AddActiveColumn(TYPE_ACTIVE);
		issuetypesTab.editlist.AddColumn(TYPE_SHORT, "Code").Sorting();
		issuetypesTab.editlist.AddColumn(TYPE_LONG, "Omschrijving").Sorting();
		issuetypesTab.editlist.SetOrderBy(TYPE_SHORT);
		issuetypesTab.editlist.SetSortColumn(0,true);
		issuetypesTab.editlist.ColumnWidths("50 60 487");

		linesTab.EditRecord = THISBACK(EditLine);
		linesTab.RemoveRecord = THISBACK(RemoveLine);
		linesTab.editlist.SetTable(LINES);
		linesTab.editlist.AddKey(L_ID);
		linesTab.AddActiveColumn(L_ACTIVE);
		linesTab.editlist.AddColumn(DEPARTMENT, "Afdeling").SetConvert(Single<ConvertDepartmentId>()).Sorting();
		linesTab.editlist.AddColumn(LINE_NUMBER, "Lijn").Sorting();
		linesTab.editlist.AddColumn(LINE_DESCRIPTION, "Omschrijving").Sorting();
		linesTab.editlist.SetOrderBy(DEPARTMENT);
		linesTab.editlist.SetSortColumn(0,true);
		linesTab.editlist.ColumnWidths("50 75 50 422");

		machinesTab.EditRecord = THISBACK(EditMachine);
		machinesTab.RemoveRecord = THISBACK(RemoveMachine);
		machinesTab.editlist.SetTable(MACHINES);
		machinesTab.editlist.AddKey(M_ID);
		machinesTab.AddActiveColumn(M_ACTIVE);
		machinesTab.editlist.AddColumn(LINE, "lijn").SetConvert(Single<ConvertLineId>()).Sorting();
		machinesTab.editlist.AddColumn(MACHINE_DESCRIPTION, "Omschrijving").Sorting();
		machinesTab.editlist.AddColumn(MACHINETYPE, "Machinetype").SetConvert(Single<ConvertMachinetypeId>()).Sorting();
		machinesTab.editlist.SetOrderBy(LINE);
		machinesTab.editlist.SetSortColumn(0,true);
		machinesTab.editlist.ColumnWidths("50 150 150 263");

		machinetypesTab.EditRecord = THISBACK(EditMachinetype);
		machinetypesTab.RemoveRecord = THISBACK(RemoveMachinetype);
		machinetypesTab.editlist.SetTable(MACHINETYPES);
		machinetypesTab.editlist.AddKey(MTYPE_ID);
		machinetypesTab.AddActiveColumn(MTYPE_ACTIVE);
		machinetypesTab.editlist.AddColumn(MTYPE_DESCRIPTION, "Omschrijving").Sorting();
		machinetypesTab.editlist.SetOrderBy(MTYPE_DESCRIPTION);
		machinetypesTab.editlist.SetSortColumn(0, true);
		machinetypesTab.editlist.ColumnWidths("50 547");

		departmentsTab.EditRecord = THISBACK(EditDepartment);
		departmentsTab.RemoveRecord = THISBACK(RemoveDepartment);
		departmentsTab.editlist.SetTable(DEPARTMENTS);
		departmentsTab.editlist.AddKey(DEP_ID);
		departmentsTab.AddActiveColumn(DEP_ACTIVE);
		departmentsTab.editlist.AddColumn(DEP_SHORT, "Code").Sorting();
		departmentsTab.editlist.AddColumn(DEP_LONG, "Omschrijving").Sorting();
		departmentsTab.editlist.SetOrderBy(DEP_SHORT);
		departmentsTab.editlist.SetSortColumn(0,true);
		departmentsTab.editlist.ColumnWidths("50 75 472");

		issuesTab.RemoveRecord = THISBACK(RemoveIssue);
		issuesTab.editlist.SetTable(ISSUELOG);
		issuesTab.editlist.AddKey(ID);
		issuesTab.editlist.AddColumn(DAY, "Dag").Sorting();
		issuesTab.editlist.AddColumn(SHIFT, "Ploeg").Sorting().SetConvert(Single<ConvertShift>());
		issuesTab.editlist.AddColumn(MINUTES, "Duur").Sorting();
		issuesTab.editlist.AddColumn(PERSON, "Monteur").Sorting().SetConvert(Single<ConvertPersonId>());
		issuesTab.editlist.AddColumn(ACTIVITY, "Activiteit").Sorting().SetConvert(Single<ConvertActivityId>());
		issuesTab.editlist.AddColumn(ISSUETYPE, "Soort storing").Sorting().SetConvert(Single<ConvertIssuetypeId>());
		issuesTab.editlist.AddColumn(MACHINE, "Machine").Sorting().SetConvert(Single<ConvertMachineId>());
		issuesTab.editlist.AddColumn(ISSUE, "Storing").Sorting();
		//issuesTab.editlist.AddColumn(CAUSE, "Oorzaak").Sorting();
		//issuesTab.editlist.AddColumn(SOLUTION, "Oplossing").Sorting();
		//issuesTab.editlist.AddColumn(REMARK, "Opmerking").Sorting();
		issuesTab.editlist.ColumnWidths("72 39 36 89 59 74 114 114");
		issuesTab.editlist.MultiSelect();

		{
			WaitCursor waitcursor;
			Open();
			statusbar = "Inlezen database...";
			Sync();
			personsTab.editlist.Query();
			activitiesTab.editlist.Query();
			issuetypesTab.editlist.Query();
			linesTab.editlist.Query();
			machinesTab.editlist.Query();
			machinetypesTab.editlist.Query();
			departmentsTab.editlist.Query();
			issuesLoaded = false;
			statusbar = "";
		}
	}

	catch(SqlExc& e) {
		ExceptionMessage("setting up Manager", e);
		return;
	}
}
