#ifndef _Logboek_journal_h_
#define _Logboek_journal_h_

#include <CtrlLib/CtrlLib.h>
#include <Report/Report.h>

#include "report.h"

// definitions of layouts
#define LAYOUTFILE <Logboek/journal.lay>
#include <CtrlCore/lay.h>


class JournalDlg : public WithJournalLayout<TopWindow>
{
	public:
		typedef JournalDlg CLASSNAME;
		JournalDlg();

	private:
		bool Key ( dword key, int count );
		ToolBar toolbar;
		Label l_date;
		DropDate edit_date;
		Label l_shift;
		Switch edit_shift;

		void NavigatorBar ( Bar& bar );

		String templateQTF;
		ReportGenerator rep;
		Report report;
		
	/*	int ploeg;
		String ploegStr;
		Date ReportDate;
		int counter;
*/
		void doDateNow();
		void doDateFirst();
		void doDateLast();
		void doDatePrev();
		void doDateNext();
		void doPrint();
		void doReport();
		void doPreview();

};

#endif
