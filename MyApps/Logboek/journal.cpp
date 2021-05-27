#include "logboek.h"
#include "journal.h"
#include "journal.brc"

void JournalDlg::NavigatorBar(Bar& bar)
{
	l_date.SetLabel("Datum:");
	edit_date.WhenAction = THISBACK(doPreview);
	l_shift.SetLabel("Ploeg:");
	edit_shift.SetLabel("nacht\ndag\navond");
	edit_shift.WhenAction = THISBACK(doPreview);
	
	bar.Gap();
	bar.Add(l_date);
	bar.Gap(4);
	bar.Add(edit_date, 120);
	bar.Gap(20);
	bar.Add(l_shift);
	bar.Gap(4);
	bar.Add(edit_shift, /*170*/ edit_shift.GetStdSize().cx*3);
	bar.Gap();
	bar.Add("Eerste", LogboekImg::control_rewind_previous(), THISBACK(doDateFirst));
	bar.Add("Vorige", LogboekImg::control_rewind(), THISBACK(doDatePrev));
	bar.Add("Vandaag", LogboekImg::reload(), THISBACK(doDateNow));
	bar.Add("Volgende", LogboekImg::control_forward(), THISBACK(doDateNext));
	bar.Add("Laatste", LogboekImg::control_forward_next(), THISBACK(doDateLast));
}


bool JournalDlg::Key(dword key, int count) {
   if (key == K_ESCAPE) {
      Close();
      return true;
   }
   return TopWindow::Key(key, count);
}


JournalDlg::JournalDlg()
{
	CtrlLayout(*this, "Logboek rapportage");
	Sizeable().MinimizeBox().MaximizeBox().Maximize();
	
	toolbar.ButtonMinSize(Size(24,24));
	AddFrame(toolbar);
	toolbar.Set(THISBACK(NavigatorBar));
	b_print <<= THISBACK(doPrint);
	b_report <<= THISBACK(doReport);
	b_exit <<= THISBACK(Close);

	//templateQTF = "[ [ {{1810:1602:6588l4/25r4/25t4/15-2 [ Logboek van ##DATE, ##PLOEG]::l0/25r0/25t0/15-1 [ ]:: [ ]::l4/25r4/25-2 [0 ]::l0/25r0/25-1 [ ]:: [ ]::l4/25r4/25b4/15-2 [ [0 ##BT]]::l0/25r0/25b0/15-1 [ ]:: [ ]::^l4/25r4/25t4/15b4/15|4 [ ##MONTEUR&][1 &][ [/1 ##ACT (##TYPE)]]::l0/25r0/25 [> [/ Machine:]]::r4/25 [ ##MACHINE]::=r0/25t0/15b0/15 [ ]::^l4/25t4/15b4/15 [> [/ Melding:]]::l0/25r4/25 [ ##STORING]::=r0/25t0/15b0/15 [ ]::^l4/25t4/15b4/15 [> [/ Oorzaak:]]::l0/25r4/25 [ ##OORZAAK]::=r0/25t0/15b0/15 [ ]::^l4/25t4/15b4/15 [> [/ Oplossing]]::l0/25r4/25 [ ##OPLOSSING]::=r0/25t0/15b0/15 [ ]::^l4/25t4/15b4/15 [> [/ Opmerking:]]::l0/25r4/25 [ ##OPMERKING##ET]::=l4/25t0/15b0/15-2 [0 ]::l0/25r0/25-1 [ ]:: [ ]::l4/25r4/25b4/15-2 [ [/1 Totaal ##TOTAAL]]::l0/25r0/25b0/15-1 [ ]:: [ ]}}&][ ]]";
	
	templateQTF = LoadFile(ConfigFile("logboek.qtf"));
	if (templateQTF=="") {
		MemReadStream in1(bin_logboek_qtf, bin_logboek_qtf_length);
		templateQTF = in1.Get(in1.GetSize());
	}
	
	Show();
	doDateNow();
}

void JournalDlg::doDateNow()
{
	Time enter_time = GetSysTime();
	edit_date <<= enter_time;
	
	if (enter_time.hour >= 15) edit_shift = 2;
	else if (enter_time.hour >= 7) edit_shift = 1;
	else edit_shift = 0;

	doPreview();
}

void JournalDlg::doDateFirst()
{
	WaitCursor waitcursor;
	
	SQL & Select(DAY, SHIFT)
			.From(ISSUELOG)
			.OrderBy(DAY, SHIFT);
	if (SQL.Fetch()) {
		edit_date <<= (Date)SQL[DAY];
		edit_shift <<= (int)SQL[SHIFT];
		doPreview();
	}
}

void JournalDlg::doDateLast()
{
	WaitCursor waitcursor;
	
	SQL & Select(DAY, SHIFT)
			.From(ISSUELOG)
			.OrderBy(Descending(DAY), Descending(SHIFT));
	if (SQL.Fetch()) {
		edit_date <<= (Date)SQL[DAY];
		edit_shift <<= (int)SQL[SHIFT];
		doPreview();
	}
}

void JournalDlg::doDatePrev()
{
	WaitCursor waitcursor;
	
	Date d = edit_date.GetData();
	int ploeg = edit_shift.GetData();
	
	SQL & Select(DAY, SHIFT)
			.From(ISSUELOG)
			.OrderBy(Descending(DAY), Descending(SHIFT));
	while (SQL.Fetch()) {
		if ((Date)SQL[DAY] > d) continue;
		else {
			if ((Date)SQL[DAY] < d) ploeg = 2;
			if ((int)SQL[SHIFT] > ploeg) continue;
			else {
				if ((Date)SQL[DAY] == d && SQL[SHIFT] == ploeg) continue;
				else {
					d = SQL[DAY];
					ploeg = SQL[SHIFT];
					break;
				}
			}
		}
	}
	edit_date <<= d;
	edit_shift <<= ploeg;

	doPreview();
}

void JournalDlg::doDateNext()
{
	WaitCursor waitcursor;
	
	Date d = edit_date.GetData();
	int ploeg = edit_shift.GetData();
	
	SQL & Select(DAY, SHIFT)
			.From(ISSUELOG)
			.OrderBy(DAY, SHIFT);
	while (SQL.Fetch()) {
		if ((Date)SQL[DAY] < d) continue;
		else {
			if ((Date)SQL[DAY] > d) ploeg = 0;
			if ((int)SQL[SHIFT] < ploeg) continue;
			else {
				if ( (SQL[DAY] == d) && (SQL[SHIFT] == ploeg) ) continue;
				else {
					d = SQL[DAY];
					ploeg = SQL[SHIFT];
					break;
				}
			}
		}
	}
	edit_date <<= d;
	edit_shift <<= ploeg;

	doPreview();
}



void JournalDlg::doPrint()
{
	DefaultPrint(report, pg.GetFirst(), "Logboek");
}



void JournalDlg::doReport()
{
	try {
		rep.SetTemplate(templateQTF);
		rep.Perform();
	}
	catch (SqlExc& e) {
		ExceptionMessage("Generating report", e);
		return;
	}
}



void JournalDlg::doPreview()
{
	try {
		rep.SetTemplate(templateQTF, "Logboek rapportage");
		rep.SetDate(edit_date.GetData());
		rep.SetShift(edit_shift.GetData());
		rep.SetWhere( (DAY == edit_date.GetData()) && (SHIFT == edit_shift.GetData()) );
		report.Clear();
		report << rep.GetResult();
		pg.Set(report);
	}
	catch (SqlExc& e) {
		ExceptionMessage("Generating preview", e);
		return;
	}
}
