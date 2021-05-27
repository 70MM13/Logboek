#include "logboek.h"
#include "manager.h"

/////////////////////////
//
// Manager: Removing functions
// try/catch are in caller functions

void ManagerDlg::RemovePerson()
{
	if (!personsTab.editlist.IsCursor()) return;
	Value id = personsTab.editlist.GetKey();
	SQL & Select(PERSON).From(ISSUELOG).Where(PERSON == id);
	if (SQL.Fetch()) {
		Exclamation(DeQtfLf("Kan niet verwijderd worden\nWordt nog gebruikt in database (Storingen)."));
		return;
	}
	else personsTab.editlist.DoRemove();
	dbCache.UpdateCache(typePersons);
}

void ManagerDlg::RemoveActivity()
{
	if (!activitiesTab.editlist.IsCursor()) return;
	Value id = activitiesTab.editlist.GetKey();
	SQL & Select(ACTIVITY).From(ISSUELOG).Where(ACTIVITY == id);
	if (SQL.Fetch()) {
		Exclamation(DeQtfLf("Kan niet verwijderd worden\nWordt nog gebruikt in database (Storingen)."));
		return;
	}
	else activitiesTab.editlist.DoRemove();
	dbCache.UpdateCache(typeActivities);
}

void ManagerDlg::RemoveIssuetype()
{
	if (!issuetypesTab.editlist.IsCursor()) return;
	Value id = issuetypesTab.editlist.GetKey();
	SQL & Select(ISSUETYPE).From(ISSUELOG).Where(ISSUETYPE == id);
	if (SQL.Fetch()) {
		Exclamation(DeQtfLf("Kan niet verwijderd worden\nWordt nog gebruikt in database (Storingen)."));
		return;
	}
	else issuetypesTab.editlist.DoRemove();
	dbCache.UpdateCache(typeIssuetypes);
}

void ManagerDlg::RemoveLine()
{
	if (!linesTab.editlist.IsCursor()) return;
	Value id = linesTab.editlist.GetKey();
	SQL & Select(LINE).From(MACHINES).Where(LINE == id);
	if (SQL.Fetch()) {
		Exclamation(DeQtfLf("Kan niet verwijderd worden\nWordt nog gebruikt in database (Machines)."));
		return;
	}
	else linesTab.editlist.DoRemove();
	dbCache.UpdateCache(typeLines);
	dbCache.UpdateCache(typeLineMachines);
}

void ManagerDlg::RemoveMachine()
{
	if (!machinesTab.editlist.IsCursor()) return;
	Value id = machinesTab.editlist.GetKey();
	SQL & Select(MACHINE).From(ISSUELOG).Where(MACHINE == id);
	if (SQL.Fetch()) {
		Exclamation(DeQtfLf("Kan niet verwijderd worden\nWordt nog gebruikt in database (Storingen)."));
		return;
	}
	else machinesTab.editlist.DoRemove();
	dbCache.UpdateCache(typeLineMachines);
//	dbCache.UpdateCache(typeMachines);
}

void ManagerDlg::RemoveMachinetype()
{
	if (!machinetypesTab.editlist.IsCursor()) return;
	Value id = machinetypesTab.editlist.GetKey();
	SQL & Select(MACHINETYPE).From(MACHINES).Where(MACHINETYPE == id);
	if (SQL.Fetch()) {
		Exclamation(DeQtfLf("Kan niet verwijderd worden\nWordt nog gebruikt in database (Machines)."));
		return;
	}
	else machinetypesTab.editlist.DoRemove();
	dbCache.UpdateCache(typeMachinetypes);
}

void ManagerDlg::RemoveDepartment()
{
	if (!departmentsTab.editlist.IsCursor()) return;
	Value id = departmentsTab.editlist.GetKey();
	SQL & Select(DEPARTMENT).From(LINES).Where(DEPARTMENT == id);
	if (SQL.Fetch()) {
		Exclamation(DeQtfLf("Kan niet verwijderd worden\nWordt nog gebruikt in database (Lijnen)."));
		return;
	}
	else departmentsTab.editlist.DoRemove();
	dbCache.UpdateCache(typeDepartments);
}

void ManagerDlg::RemoveIssue()
{
	if (!issuesTab.editlist.IsCursor()) return;
	if (issuesTab.editlist.GetSelectCount() > 1) {
		if (PromptYesNo(Format("%i records verwijderen?", issuesTab.editlist.GetSelectCount()))) {
			bool IAR = issuesTab.editlist.IsAskRemove();
			issuesTab.editlist.NoAskRemove();
			Vector <int> index_list;
			int i;
			for (i=0; i<issuesTab.editlist.GetCount(); i++) {
				if (issuesTab.editlist.IsSelected(i)) index_list.Add(i);
			}
			for (i=index_list.GetCount()-1; i>=0; i--) {
		    	issuesTab.editlist.SetCursor(index_list[i]);
				issuesTab.editlist.DoRemove();

			}
			issuesTab.editlist.AskRemove(IAR);
		}
	}
	else issuesTab.editlist.DoRemove();
}
