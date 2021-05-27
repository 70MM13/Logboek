#ifndef _logboek_manager_h_
#define _logboek_manager_h_

//////////////////////////////////////////////////
//
// definitions of layouts
//
#define LAYOUTFILE <Logboek/manager.lay>
#include <CtrlCore/lay.h>


//////////////////////////////////////////////////
//
// definition of tab content class
//
class ManagerTab : public WithManagerTabLayout<ParentCtrl>  {
	typedef ManagerTab CLASSNAME;

public:
	ManagerTab();
	void PopupMenu(Bar& bar);
	Callback1 <int&> EditRecord;
	Callback RemoveRecord;
	void AddActiveColumn(const SqlId& Id);
	void SetActive(Value v);

private:
	bool hasActiveColumn;
	SqlId ActiveId;
	void _AddRecord();
	void _EditRecord();
	void _RemoveRecord();
#ifdef _DEBUG
	void _WhenHeaderLayout(){
		infobox = Format("%s", editlist.GetColumnWidths());
	};
#endif
};


//////////////////////////////////////////////////
//
// definition of 'manager' class
//
class ManagerDlg : public WithManagerLayout<TopWindow> {
	typedef ManagerDlg CLASSNAME;

private:
	StatusBar statusbar;
	MenuBar menubar;
	void MainMenu(Bar& bar);
	void ManagerMenu(Bar& bar);
	void RecordMenu(Bar& bar);
	void ExtraMenu(Bar& bar);

	ManagerTab personsTab;
	ManagerTab activitiesTab;
	ManagerTab issuetypesTab;
	ManagerTab linesTab;
	ManagerTab machinesTab;
	ManagerTab machinetypesTab;
	ManagerTab departmentsTab;
	ManagerTab issuesTab; bool issuesLoaded;

	void WhenTabAction ();

public:
	ManagerDlg();

#ifdef flagIMPORT
	void import();
#endif

	//version Edit...CB is for CALLBACKs from list
	void EditPerson			(int& id);
	void EditActivity		(int& id);
	void EditIssuetype		(int& id);
	void EditMachine		(int& id);
	void EditMachinetype	(int& id);
	void EditLine			(int& id);
	void EditDepartment		(int& id);

	void RemovePerson();
	void RemoveActivity();
	void RemoveIssuetype();
	void RemoveLine();
	void RemoveMachine();
	void RemoveMachinetype();
	void RemoveDepartment();
	void RemoveIssue();
};

#endif
