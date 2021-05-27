#include "logboek.h"
#include "manager.h"

/////////////////////////
//
// Manager: Edit person
//

struct PersonDlg : public WithEditPersonLayout<TopWindow> {
	typedef PersonDlg CLASSNAME;

	SqlCtrls ctrls;

	PersonDlg();
};

PersonDlg::PersonDlg() {
	CtrlLayoutOKCancel(*this, "");
	ctrls(*this, PERSONS);
}

void ManagerDlg::EditPerson(int& id)
{
	PersonDlg dlg;
	if(!IsNull(id)) {
		dlg.Title("Bewerk monteur");
		if(!dlg.ctrls.Load(PERSONS, P_ID == id))
			return;
		if(dlg.Execute() != IDOK)
			return;
		SQL & dlg.ctrls.Update(PERSONS).Where(P_ID == id);
	}
	else {
		dlg.Title("Bewerk monteur (nieuw)");
		dlg.p_active.SetData(1);
		if(dlg.Execute() != IDOK)
			return;
		SQL & dlg.ctrls.Insert(PERSONS);
		id = SQL.GetInsertedId();
	}
	personsTab.editlist.ReQuery();
	dbCache.UpdateCache(typePersons);
}


/////////////////////////
//
// Manager: Edit activity
//

struct ActivityDlg : public WithEditActivityLayout<TopWindow> {
	typedef ActivityDlg CLASSNAME;
	SqlCtrls ctrls;
	ActivityDlg();
};

ActivityDlg::ActivityDlg() {
	CtrlLayoutOKCancel(*this, "");
	ctrls(*this, ACTIVITIES);
}

void ManagerDlg::EditActivity(int& id)
{
	ActivityDlg dlg;

	if(!IsNull(id)) {
		dlg.Title("Bewerk activiteit");
		if(!dlg.ctrls.Load(ACTIVITIES, ACT_ID == id))
			return;
		if(dlg.Execute() != IDOK)
			return;
		SQL & dlg.ctrls.Update(ACTIVITIES).Where(ACT_ID == id);
	}
	else {
		dlg.Title("Bewerk activiteit (nieuw)");
		dlg.act_active.SetData(1);
		if(dlg.Execute() != IDOK)
			return;
		SQL & dlg.ctrls.Insert(ACTIVITIES);
		id = SQL.GetInsertedId();
	}

	activitiesTab.editlist.ReQuery();
	dbCache.UpdateCache(typeActivities);
}


/////////////////////////
//
// Manager: Edit issuetype
//

struct IssuetypeDlg : public WithEditIssuetypeLayout<TopWindow> {
	typedef IssuetypeDlg CLASSNAME;
	SqlCtrls ctrls;
	IssuetypeDlg();
};

IssuetypeDlg::IssuetypeDlg() {
	CtrlLayoutOKCancel(*this, "");
	ctrls(*this, ISSUETYPES);
}

void ManagerDlg::EditIssuetype(int& id)
{
	IssuetypeDlg dlg;

	if(!IsNull(id)) {
		dlg.Title("Bewerk storingsoort");
		if(!dlg.ctrls.Load(ISSUETYPES, TYPE_ID == id))
			return;
		if(dlg.Execute() != IDOK)
			return;
		SQL & dlg.ctrls.Update(ISSUETYPES).Where(TYPE_ID == id);
	}
	else {
		dlg.Title("Bewerk storingsoort (nieuw)");
		dlg.type_active.SetData(1);
		if(dlg.Execute() != IDOK)
			return;
		SQL & dlg.ctrls.Insert(ISSUETYPES);
		id = SQL.GetInsertedId();
	}

	issuetypesTab.editlist.ReQuery();
	dbCache.UpdateCache(typeIssuetypes);
}


/////////////////////////
//
// Manager: Edit machine
//
// ty/catch are in caller functions

struct MachineDlg : public WithEditMachineLayout<TopWindow> {
	typedef MachineDlg CLASSNAME;
	SqlCtrls ctrls;
	MachineDlg();

	void FetchLines();
	void FetchMachinetypes();

	Callback1<int&> Edit_Line;
	Callback1<int&> Edit_Machinetype;
	void _EditLineCB(bool CreateNew);
	void _EditMachinetypeCB(bool CreateNew);
	void MakeLineToolBar(Bar& bar);
	void MakeMachinetypeToolBar(Bar& bar);
};

void MachineDlg::MakeLineToolBar(Bar& bar)
{
	bar.Add("Bewerken", LogboekImg::form_edit_16(), THISBACK1(_EditLineCB, false));
	bar.Add("Nieuw", LogboekImg::add_16(), THISBACK1(_EditLineCB, true));
}
void MachineDlg::MakeMachinetypeToolBar(Bar& bar)
{
	bar.Add("Bewerken", LogboekImg::form_edit_16(), THISBACK1(_EditMachinetypeCB, false));
	bar.Add("Nieuw", LogboekImg::add_16(), THISBACK1(_EditMachinetypeCB, true));
}

MachineDlg::MachineDlg() {
	CtrlLayoutOKCancel(*this, "");

	tool_line.Set(THISBACK(MakeLineToolBar));
	tool_machinetype.Set(THISBACK(MakeMachinetypeToolBar));

	FetchLines();
	FetchMachinetypes();

    line.WithDropChoice::SetConvert(Single<ConvertLineId>());
    line.DataPusher::SetConvert(Single<ConvertLineId>());
    machinetype.WithDropChoice::SetConvert(Single<ConvertMachinetypeId>());
    machinetype.DataPusher::SetConvert(Single<ConvertMachinetypeId>());

	ctrls(*this, MACHINES);
}

void MachineDlg::FetchLines()
{
	line.ClearList();
	SQL & Select(L_ID).From(LINES)
							.Where(L_ACTIVE == true)
							.OrderBy(LINE_NUMBER);
	while(SQL.Fetch()) {
		line.AddList(SQL[L_ID]);
	}
}

void MachineDlg::_EditLineCB(bool CreateNew)
{
	int id = Null;
	if (!CreateNew) id = line.GetData();
	Edit_Line(id);
	if (!IsNull(id)) {
		FetchLines();
		line.SetData(id);
	}
}

void MachineDlg::FetchMachinetypes()
{
	machinetype.ClearList();
	SQL & Select(MTYPE_ID).From(MACHINETYPES)
							.Where(MTYPE_ACTIVE == true)
							.OrderBy(MTYPE_DESCRIPTION);
	while(SQL.Fetch()) {
		machinetype.AddList(SQL[MTYPE_ID]);
	}
}

void MachineDlg::_EditMachinetypeCB(bool CreateNew)
{
	int id = Null;
	if (!CreateNew) id = machinetype.GetData();
	Edit_Machinetype(id);
	if (!IsNull(id)) {
		FetchMachinetypes();
		machinetype.SetData(id);
	}
}

void ManagerDlg::EditMachine(int& id)
{
	MachineDlg dlg;
	dlg.Edit_Line = THISBACK(EditLine);
	dlg.Edit_Machinetype = THISBACK(EditMachinetype);

	if(!IsNull(id)) {
		dlg.Title("Bewerk machine");
		if(!dlg.ctrls.Load(MACHINES, M_ID == id))
			return;
		if(dlg.Execute() != IDOK)
			return;
		SQL & dlg.ctrls.Update(MACHINES).Where(M_ID == id);
	}
	else {
		dlg.Title("Bewerk machine (nieuw)");
		dlg.m_active.SetData(1);
		if(dlg.Execute() != IDOK)
			return;
		SQL & dlg.ctrls.Insert(MACHINES);
		id = SQL.GetInsertedId();
	}

	machinesTab.editlist.ReQuery();
	dbCache.UpdateCache(typeLineMachines);
//	dbCache.UpdateCache(typeMachines);
}


/////////////////////////
//
// Manager: Edit machinetype
//

struct MachinetypeDlg : public WithEditMachinetypeLayout<TopWindow> {
	typedef MachinetypeDlg CLASSNAME;
	SqlCtrls ctrls;

	MachinetypeDlg() {
	CtrlLayoutOKCancel(*this, "");
	ctrls(*this, MACHINETYPES);
	};

};

void ManagerDlg::EditMachinetype(int& id)
{
	MachinetypeDlg dlg;

	if(!IsNull(id)) {
		dlg.Title("Bewerk machinetype");
		if(!dlg.ctrls.Load(MACHINETYPES, MTYPE_ID == id))
			return;
		if(dlg.Execute() != IDOK)
			return;
		SQL & dlg.ctrls.Update(MACHINETYPES).Where(MTYPE_ID == id);
	}
	else {
		dlg.Title("Bewerk machinetype (nieuw)");
		dlg.mtype_active.SetData(1);
		if(dlg.Execute() != IDOK)
			return;
		SQL & dlg.ctrls.Insert(MACHINETYPES);
		id = SQL.GetInsertedId();
	}

	machinetypesTab.editlist.ReQuery();
	dbCache.UpdateCache(typeMachinetypes);
}


/////////////////////////
//
// Manager: Edit line
//

struct LineDlg : public WithEditLineLayout<TopWindow> {
	typedef LineDlg CLASSNAME;
	SqlCtrls ctrls;
	LineDlg();

	void FetchDepartments();

	Callback1<int&> Edit_Department;
	void _EditDepartmentCB(bool CreateNew);
	void MakeDepartmentToolBar(Bar& bar);
};

void LineDlg::MakeDepartmentToolBar(Bar& bar)
{
	bar.Add("Bewerken", LogboekImg::form_edit_16(), THISBACK1(_EditDepartmentCB, false));
	bar.Add("Nieuw", LogboekImg::add_16(), THISBACK1(_EditDepartmentCB, true));
}

LineDlg::LineDlg() {
	CtrlLayoutOKCancel(*this, "");

    department.WithDropChoice::SetConvert(Single<ConvertDepartmentId>());
    department.DataPusher::SetConvert(Single<ConvertDepartmentId>());

	tool_department.Set(THISBACK(MakeDepartmentToolBar));

	FetchDepartments();
	ctrls(*this, LINES);
}

void LineDlg::FetchDepartments()
{
	department.ClearList();
	SQL & Select(DEP_ID).From(DEPARTMENTS)
							.Where(DEP_ACTIVE == true)
							.OrderBy(DEP_SHORT);
	while(SQL.Fetch()) {
		department.AddList(SQL[DEP_ID]);
	}
}

void LineDlg::_EditDepartmentCB(bool CreateNew) {
	int id = Null;
	if (!CreateNew) id = department.GetData();
	Edit_Department(id);
	if (!IsNull(id)) {
		FetchDepartments();
		department.SetData(id);
	}
}

void ManagerDlg::EditLine(int& id)
{
	LineDlg dlg;
	dlg.Edit_Department = THISBACK(EditDepartment);

	if(!IsNull(id)) {
		dlg.Title("Bewerk lijn");
		if(!dlg.ctrls.Load(LINES, L_ID == id))
			return;
		if(dlg.Execute() != IDOK)
			return;
		SQL & dlg.ctrls.Update(LINES).Where(L_ID == id);
	}
	else {
		dlg.Title("Bewerk lijn (nieuw)");
		dlg.l_active.SetData(1);
		if(dlg.Execute() != IDOK)
			return;
		SQL & dlg.ctrls.Insert(LINES);
		id = SQL.GetInsertedId();
	}

	linesTab.editlist.ReQuery();
	dbCache.UpdateCache(typeLines);
	dbCache.UpdateCache(typeLineMachines);
}


/////////////////////////
//
// Manager: Edit department
//

struct DepartmentDlg : public WithEditDepartmentLayout<TopWindow> {
	typedef DepartmentDlg CLASSNAME;
	SqlCtrls ctrls;
	DepartmentDlg();
};

DepartmentDlg::DepartmentDlg() {
	CtrlLayoutOKCancel(*this, "");
	ctrls(*this, DEPARTMENTS);
}

void ManagerDlg::EditDepartment(int& id)
{
	DepartmentDlg dlg;

	if(!IsNull(id)) {
		dlg.Title("Bewerk afdeling");
		if(!dlg.ctrls.Load(DEPARTMENTS, DEP_ID == id))
			return;
		if(dlg.Execute() != IDOK)
			return;
		SQL & dlg.ctrls.Update(DEPARTMENTS).Where(DEP_ID == id);
	}
	else {
		dlg.Title("Bewerk afdeling (nieuw)");
		dlg.dep_active.SetData(1);
		if(dlg.Execute() != IDOK)
			return;
		SQL & dlg.ctrls.Insert(DEPARTMENTS);
		id = SQL.GetInsertedId();
	}

	departmentsTab.editlist.ReQuery();
	dbCache.UpdateCache(typeDepartments);
}
