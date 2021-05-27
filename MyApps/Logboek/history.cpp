#include "logboek.h"
#include "history.h"


HistoryDetail::HistoryDetail()
{
	CtrlLayout(*this, "");
}

/*
	menu
*/
void HistoryDlg::MainMenu(Bar& bar)
{
	bar.Add("Bestand", THISBACK(FileMenu));
	bar.Add("Opties", THISBACK(OptionsMenu));
}

void HistoryDlg::FileMenu(Bar& bar)
{
	bar.Add("Print", THISBACK(Print));
	bar.Separator();
	bar.Add("Sluit", THISBACK(Close));
}

void HistoryDlg::OptionsMenu(Bar& bar)
{
	bar.Add("Niet-actieve waarden", THISBACK(ToggleNonactive))
		.Check(show_nonactive);
}

void HistoryDlg::ToggleNonactive()
{
	show_nonactive = !show_nonactive;
	UpdateAllLists();
	//return show_nonactive;
}



void HistoryDlg::UpdateFromAny()
{

}

void HistoryDlg::UpdateFromDepartment()
{
	if(!updating) {
		updating = true;
		try {
			SaveMachine();
			drop_machine.SetData(0);
			UpdateLineList();
			UpdateMachineList();
		}
		catch (SqlExc& e) {
			ExceptionMessage("updating from department", e);
		}
		updating = false;
	}
}

void HistoryDlg::UpdateFromLine()
{
	if(!updating) {
		updating = true;
		try {
			SaveMachine();
			UpdateMachineList();
			UpdateDepartmentList();
		}
		catch (SqlExc& e) {
			ExceptionMessage("updating from line", e);
		}
		updating = false;
	}
}

void HistoryDlg::UpdateFromMachine()
{
	if(!updating) {
		updating = true;
		try {
			UpdateLineList();
			if ((int)drop_machine.GetData() > 0) {
				SQL & Select(M_ID, LINE).From(MACHINES).Where(M_ID == drop_machine.GetData());
				if (SQL.Fetch()) if (drop_line.GetData() != SQL[LINE]) drop_line.SetData(SQL[LINE]);
			}
		}
		catch (SqlExc& e) {
			ExceptionMessage("updating from machine", e);
		}
		updating = false;
	}
}

void HistoryDlg::UpdateAllLists()
{
	String str;
	SqlBool where;

	updating = true;
	try {
		SaveMachine();
		drop_department.ClearValue();
		drop_machine.ClearValue();

		// Monteurs list
		if (!show_nonactive) where = (P_ACTIVE == true);
		drop_person.Clear();
		SQL & Select(SqlAll())	.From(PERSONS)
								.Where(where)
								.OrderBy(FIRST_NAME, MID_NAME, LAST_NAME);
		while (SQL.Fetch()) {
			str = SQL[FIRST_NAME];
			if (!SQL[MID_NAME].IsNull()) str += " " + SQL[MID_NAME].ToString();
			if (!SQL[LAST_NAME].IsNull()) str += " " + SQL[LAST_NAME].ToString();
			drop_person.Add(SQL[P_ID], str);
		}

		// Activities list
		if (!show_nonactive) where = (ACT_ACTIVE == true);
		drop_activity.Clear();
		SQL & Select(SqlAll())	.From(ACTIVITIES)
								.Where(where)
								.OrderBy(ACT_LONG);
		while(SQL.Fetch()) {
			drop_activity.Add(SQL[ACT_ID], SQL[ACT_LONG].ToString());
		}

		// Issuetypes list
		if (!show_nonactive) where = (TYPE_ACTIVE == true);
		drop_issuetype.Clear();
		SQL & Select(SqlAll())	.From(ISSUETYPES)
								.Where(where)
								.OrderBy(TYPE_LONG);
		while(SQL.Fetch()) {
			drop_issuetype.Add(SQL[TYPE_ID], SQL[TYPE_LONG].ToString());
		}

		// Departments list
		if (!show_nonactive) where = (DEP_ACTIVE == true);
		drop_department.Clear();
		SQL & Select(SqlAll())	.From(DEPARTMENTS)
								.Where(where)
								.OrderBy(DEP_SHORT);
		while(SQL.Fetch()) {
			drop_department.Add(SQL[DEP_ID],SQL[DEP_SHORT].ToString());
		}

		// Dynamic lists
		UpdateMachineList();
		UpdateLineList();
		UpdateDepartmentList();
	}
	catch (SqlExc& e) {
		ExceptionMessage("updating lists", e);
	}
	updating = false;
}

void HistoryDlg::UpdateDepartmentList()
{
	if ( (int)drop_line.GetData() > 0) {
		SQL & Select(SqlAll()) .From(LINES) .Where(L_ID == drop_line.GetData());
		if (SQL.Fetch()) if(drop_department.GetData() != SQL[DEPARTMENT]) drop_department.SetData(SQL[DEPARTMENT]);
	}
}

void HistoryDlg::UpdateLineList()
{
	SqlBool where;
	int i=0, id;
	int new_v = 0, old_v = drop_line.GetData();

	drop_line.ClearValue();
	drop_line.Clear();

 	if (!show_nonactive) where = (L_ACTIVE == true);
	if ((int)drop_department.GetData() > 0) {
		where = where && (DEPARTMENT == drop_department.GetData());
	}
	SQL & Select(SqlAll())	.From(LINES)
							.Where(where)
							.OrderBy(LINE_NUMBER);
	while(SQL.Fetch()) {
		id = SQL[L_ID];
		drop_line.Add(id, Format("%s   : %s",SQL[LINE_NUMBER].ToString(),SQL[LINE_DESCRIPTION].ToString()));
		if (id == old_v) new_v = id;
		i++;
	}
	if (i==1 && !IsNull(id)) new_v = id;
	drop_line.SetData(new_v);
}

void HistoryDlg::UpdateMachineList()
{
	SqlBool where;
	int i=0, id, new_ml=0;

	drop_machine.SetData(0);
	drop_machine.Clear();

	if (!show_nonactive) where = (M_ACTIVE == true);
	if ( (int)drop_department.GetData() > 0)
		where = where && (DEPARTMENT == drop_department.GetData());
	if ( (int)drop_line.GetData() > 0)
		where = where && LINE == drop_line.GetData();

	SQL & Select(SqlAll())	.From(MACHINES)
							.InnerJoin(LINES) .On(LINE.Of(MACHINES) == L_ID.Of(LINES))
							.Where(where)
							.OrderBy(MACHINE_DESCRIPTION);
	while (SQL.Fetch()) {
		id = SQL[M_ID];
		drop_machine.Add(id, SQL[MACHINE_DESCRIPTION].ToString());
		if (id == save_ml) new_ml = id;
		i++;
	}
	if (i==1 && !IsNull(id)) new_ml = id;
	drop_machine.SetData(new_ml);
}

void HistoryDlg::FetchResults()
{
	WaitCursor _;
	where.SetTrue();
	try {
		results.Clear();
		UpdateDetails();
		if ( !IsNull(drop_department.GetData()) )
			where = where && (DEPARTMENT == drop_department.GetData());
		if ( !IsNull(drop_line.GetData()) )
			where = where && (LINE == drop_line.GetData());
		if ( !IsNull(drop_machine.GetData()) )
			where = where && (MACHINE == drop_machine.GetData());
		if ( !IsNull(drop_activity.GetData()) )
			where = where && (ACTIVITY == drop_activity.GetData());
		if ( !IsNull(drop_issuetype.GetData()) )
			where = where && (ISSUETYPE == drop_issuetype.GetData());
		if ( !IsNull(drop_date_from.GetData()) )
			where = where && (DAY >= drop_date_from.GetData());
		if ( !IsNull(drop_date_to.GetData()) )
			where = where && (DAY <= drop_date_to.GetData());
		if ( !IsNull(drop_person.GetData()) )
			where = where && (PERSON == drop_person.GetData());

		SQL & Select(SqlAll())
			.From(ISSUELOG)
			.InnerJoin(MACHINES)		.On(MACHINE		.Of(ISSUELOG) == M_ID.Of(MACHINES))
			.InnerJoin(MACHINETYPES)	.On(MACHINETYPE	.Of(MACHINES) == MTYPE_ID.Of(MACHINETYPES))
			.InnerJoin(LINES)			.On(LINE		.Of(MACHINES) == L_ID.Of(LINES))
			.InnerJoin(DEPARTMENTS)		.On(DEPARTMENT	.Of(LINES)	  == DEP_ID.Of(DEPARTMENTS))
			.Where(where);
		while (SQL.Fetch()) {
			results.Add(SQL[ID], /* datum, soort, melding */
						SQL[DAY],
						Format("%s; %s", dbCache.Get(typeActivities, SQL[ACTIVITY]), dbCache.Get(typeIssuetypes, SQL[ISSUETYPE])),
						Format("%s", SQL[ISSUE]));
		}
	}
	catch (SqlExc& e) {
		ExceptionMessage("updating lists", e);
	}
	results.SetCursor(0);
}

void HistoryDlg::UpdateDetails0(int id)
{
	String text, tmp;
	try {
		SQL & Select(SqlAll())
			.From(ISSUELOG)
			.InnerJoin(MACHINES)		.On(MACHINE		.Of(ISSUELOG) == M_ID.Of(MACHINES))
			.InnerJoin(MACHINETYPES)	.On(MACHINETYPE	.Of(MACHINES) == MTYPE_ID.Of(MACHINETYPES))
			.InnerJoin(LINES)			.On(LINE		.Of(MACHINES) == L_ID.Of(LINES))
			.InnerJoin(DEPARTMENTS)		.On(DEPARTMENT	.Of(LINES)	  == DEP_ID.Of(DEPARTMENTS))
			.Where(ID == id);
		if (SQL.Fetch()) {
			text << "" << TrimBoth(SQL[DEP_SHORT].ToString());
			text << " lijn " << TrimBoth(SQL[LINE_NUMBER].ToString());
			text << ": " << TrimBoth(SQL[MACHINE_DESCRIPTION].ToString());
			text << " (" << TrimBoth(SQL[MTYPE_DESCRIPTION].ToString()) << ")";

			text << "\n\n" << TrimBoth(dbCache.Get(typeActivities, SQL[ACTIVITY]));
			text << "; " << TrimBoth(dbCache.Get(typeIssuetypes, SQL[ISSUETYPE]));
			text << " van " << Format("%Dy", DayOfWeek(SQL[DAY])) <<  " " << SQL[DAY].ToString() << " (";
			switch((int)SQL[SHIFT]) {
				case 0 : text << "ND"; break;
				case 1 : text << "DD"; break;
				case 2 : text << "LD"; break;
				default: text << SQL[SHIFT].ToString() << "?";
			}
			text << ") door " << TrimBoth(dbCache.Get(typePersons, SQL[PERSON]));
			text << " (" << FormatElapsedMin(SQL[MINUTES]).ToString() << ")";


			tmp = TrimBoth(SQL[ISSUE].ToString());
			if (tmp.GetLength() > 0) text << "\n\nMelding: " << tmp;
			tmp = TrimBoth(SQL[CAUSE].ToString());
			if (tmp.GetLength() > 0) text << "\nOorzaak: " << tmp;
			tmp = TrimBoth(SQL[SOLUTION].ToString());
			if (tmp.GetLength() > 0) text << "\nOplossing: " << tmp;
			tmp = TrimBoth(SQL[REMARK].ToString());
			if (tmp.GetLength() > 0) text << "\nOpmerking: " << tmp;
		}
	}
	catch (SqlExc& e) {
		ExceptionMessage("updating details", e);
	}
	details.textbox.SetEditable();
	details.textbox.Paste(text.ToWString());
	details.textbox.SetReadOnly();
}

inline void HistoryDlg::UpdateDetails()
{
	int i = results.GetCursor();
	int j = results.GetCount();
	details.textbox.Clear();
	if ((i >= 0) && (i < j)) {
		details.info.SetText(Format("Resultaat %i van %i",i+1, j));
		details.b_previous.Enable(i > 0);
		details.b_next.Enable(i <= j);
		UpdateDetails0(results.Get(i, 0));
	}
	else details.info.SetText("Geen resultaten");
}

void HistoryDlg::ShowDetails()
{
	tabs.Set(1);
}

void HistoryDlg::DetailPrevious()
{
	int i = results.GetCursor();
	if (i > 0) results.SetCursor(i - 1);
}

void HistoryDlg::DetailNext()
{
	int i = results.GetCursor();
	if (i <= results.GetCount()) results.SetCursor(i + 1);
}



void HistoryDlg::Print()
{
	String templateQTF = "^F[ [ {{5000:5000t/8b/8RN [%% ]:: [> [%%/ p. ]{:VALUE:PAGENUMBER:}[%%/  van ]{:VALUE:PAGECOUNT:}]}}]]^^[ [ Logboek historie&][ {{1810:1729:6461f8;FNg0;GNt/8b8/8RN-2 [ [0 ##BT]]::t/15b0/15R0-1 [ ]:: [ ]::^r8/25t/8b/8|4 [ [2 ##DATE, ##PLOEG]&][ [2 ##MONTEUR]&][1 &][ [/1 ##ACT (##TYPE)]]::r0/25 [> [/ Machine:]]::RN [ ##MACHINE]::=t/15b/15R0 [ ]::^t/8b/8 [> [/ Melding:]]::RN [ ##STORING]::=t/15b/15R0 [ ]::^t/8b/8 [> [/ Oorzaak:]]::RN [ ##OORZAAK]::=t/15b/15R0 [ ]::^t/8b/8 [> [/ Oplossing:]]::RN [ ##OPLOSSING]::=t/15b/15R0 [ ]::^t/8b/8 [> [/ Opmerking:]]::RN [ ##OPMERKING##ET]::=t/15b/15-2 [0 ]::R0-1 [ ]:: [ ]::RN-2 [ [/1 Totaal ##TOTAAL]]::R0-1 [ ]:: [ ]}}&][ ]]";
	rep.SetTemplate(templateQTF, "Logboek rapportage");
	//rep.SetDate(edit_date.GetData());
	//rep.SetShift(edit_shift.GetData());
	int i = results.GetCursor();
	if (i < 0) i = 0;
	int id = results.Get(i, 0);
	rep.SetWhere(/*where &&*/ (ID == id));
	rep.Perform();
}



HistoryDlg::HistoryDlg()
{
	CtrlLayout(*this, "Logboek historie");
	Sizeable().MinimizeBox().MaximizeBox();

	AddFrame(menubar);
	menubar.Set(THISBACK(MainMenu));

	s = ToolButton::StyleDefault();
	s.contrast[CTRL_NORMAL] = -64;

	b_res_department.Image(LogboekImg::cancel_16());
	b_res_department.SetStyle(s);
	b_res_line      .Image(LogboekImg::cancel_16());
	b_res_line		.SetStyle(s);
	b_res_machine   .Image(LogboekImg::cancel_16());
	b_res_machine	.SetStyle(s);
	b_res_activity  .Image(LogboekImg::cancel_16());
	b_res_activity  .SetStyle(s);
	b_res_issuetype .Image(LogboekImg::cancel_16());
	b_res_issuetype .SetStyle(s);
	b_res_date_from .Image(LogboekImg::cancel_16());
	b_res_date_from	.SetStyle(s);
	b_res_date_to   .Image(LogboekImg::cancel_16());
	b_res_date_to	.SetStyle(s);
	b_res_person	.Image(LogboekImg::cancel_16());
	b_res_person	.SetStyle(s);

	//show_nonactive   .WhenAction = THISBACK(UpdateAllLists);
	drop_department  .WhenAction = THISBACK(UpdateFromDepartment);
	drop_line        .WhenAction = THISBACK(UpdateFromLine);
	drop_machine     .WhenAction = THISBACK(UpdateFromMachine);
	b_res_department .WhenAction = THISBACK(ResetDepartment);
	b_res_line       .WhenAction = THISBACK(ResetLine);
	b_res_machine    .WhenAction = THISBACK(ResetMachine);
	b_res_activity   .WhenAction = THISBACK(ResetActivity);
	b_res_issuetype  .WhenAction = THISBACK(ResetIssuetype);
	b_res_date_from  .WhenAction = THISBACK(ResetDateFrom);
	b_res_date_to    .WhenAction = THISBACK(ResetDateTo);
	b_res_person     .WhenAction = THISBACK(ResetPerson);
	b_search         .WhenAction = THISBACK(FetchResults);

	UpdateAllLists();

	//results.HeaderObject().Absolute();
	results.AddIndex();
	results.AddColumn("datum",45);
	results.AddColumn("soort",70);
	results.AddColumn("melding",150);
	results.WhenCursor = THISBACK(UpdateDetails);
	results.WhenLeftDouble = THISBACK(ShowDetails);
	details.b_previous.WhenAction = THISBACK(DetailPrevious);
	details.b_next.WhenAction = THISBACK(DetailNext);
	details.textbox.NoEofLine();
	tabs.Add(results.SizePos(), "Resultaten");
	tabs.Add(details.SizePos(), "Details");
}
