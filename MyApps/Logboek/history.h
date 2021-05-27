#ifndef _Logboek_history_h_
#define _Logboek_history_h_

#include "report.h"

// definitions of layouts
#define LAYOUTFILE <Logboek/history.lay>
#include <CtrlCore/lay.h>

struct HistoryDetail : WithHistoryDetailLayout<TopWindow> {
	typedef HistoryDetail CLASSNAME;
	HistoryDetail();
};

class HistoryDlg : public WithHistoryLayout<TopWindow>
{
	private:
		ToolButton::Style s;
		MenuBar menubar;
		void MainMenu (Bar& bar);
		void FileMenu (Bar& bar);
		void OptionsMenu (Bar& bar);

		ReportGenerator rep;
	
		ArrayCtrl results;
		HistoryDetail details;
		int save_ml;
		bool updating;

		bool show_nonactive;
		SqlBool where;
		
		void ToggleNonactive();
		void SaveMachine() { save_ml = drop_machine.GetData(); };
		void UpdateFromAny();
		void UpdateFromDepartment();
		void UpdateFromLine();
		void UpdateFromMachine();
		void UpdateAllLists();
		void UpdateDepartmentList();
		void UpdateLineList();
		void UpdateMachineList();
		void ResetDepartment()  { drop_department.ClearValue(); };
		void ResetLine()        { drop_line.ClearValue(); };
		void ResetMachine()     { drop_machine.ClearValue(); };
		void ResetMachinetype() { drop_machinetype.ClearValue(); };
		void ResetPerson()      { drop_person.ClearValue(); };
		void ResetActivity()    { drop_activity.ClearValue(); };
		void ResetIssuetype()   { drop_issuetype.ClearValue(); };
		void ResetDateFrom()    { drop_date_from.Clear(); } ;
		void ResetDateTo()      { drop_date_to.Clear(); };
		
		void FetchResults();
		
		void UpdateDetails0(int id);
		void UpdateDetails();
		void ShowDetails();
		void DetailPrevious();
		void DetailNext();
		void Print();
		
	public:
		typedef HistoryDlg CLASSNAME;
		HistoryDlg();
};

#endif
