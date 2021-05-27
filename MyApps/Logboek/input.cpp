#include "logboek.h"
#include "input.h"

//////////////////////////////////////////////////
//
// Base "invoer" dialog
//

InputDlg::InputDlg()
{
	Time enter_time = GetSysTime();

	CtrlLayout(*this, "Storing invoeren");
	AddFrame(status);
	b_invoeren <<= THISBACK(ConfirmStore);
	b_exit <<= THISBACK(ConfirmClose);

	edit_activity	.WhenAction = THISBACK(UpdateFromAny);
	edit_date		.WhenAction = THISBACK(UpdateFromAny);
	edit_department	.WhenAction = THISBACK(UpdateFromDepartment);
	edit_issuetype	.WhenAction = THISBACK(UpdateFromAny);
	edit_linenumber	.WhenAction = THISBACK(UpdateFromLinenumber);
	edit_machine	.WhenAction = THISBACK(UpdateFromAny);
	edit_monteur	.WhenAction = THISBACK(UpdateFromAny);
	edit_oorzaak	.WhenAction = THISBACK(UpdateFromAny);
	edit_oplossing	.WhenAction = THISBACK(UpdateFromAny);
	edit_opmerking	.WhenAction = THISBACK(UpdateFromAny);
	edit_ploeg		.WhenAction = THISBACK(UpdateFromAny);
	edit_probleem	.WhenAction = THISBACK(UpdateFromAny);
	edit_time_h		.WhenAction = THISBACK(UpdateFromTime);
	edit_time_m		.WhenAction = THISBACK(UpdateFromTime);
	edit_total_m	.WhenAction = THISBACK(UpdateFromTotalTime);

	edit_probleem.NoProcessTab();
	edit_probleem.NoEofLine();
	edit_oorzaak.NoProcessTab();
	edit_oorzaak.NoEofLine();
	edit_oplossing.NoProcessTab();
	edit_oplossing.NoEofLine();
	edit_opmerking.NoProcessTab();
	edit_opmerking.NoEofLine();

	edit_date <<= enter_time;

	if (enter_time.hour >= 15) edit_ploeg = 2;
	else if (enter_time.hour >= 7) edit_ploeg = 1;
	else edit_ploeg = 0;

	Show();
	UpdateAllLists();
	Reset();
}

//////////////////////////////////////////////////
//
// change other fields from updates
//

void InputDlg::UpdateFromAny()
{
	dirty = true;
}

void InputDlg::UpdateFromDepartment()
{
	dirty = true;
	try {
		UpdateLinenumberList();
	}
	catch (SqlExc& e) {
		ExceptionMessage("updating from department", e);
	}
}

void InputDlg::UpdateFromLinenumber()
{
	dirty = true;
	try {
		SQL & Select(SqlAll()) .From(LINES) .Where(L_ID ==edit_linenumber.GetData());
		if (SQL.Fetch()) edit_department.SetData(SQL[DEPARTMENT]);

		UpdateMachineList();
	}
	catch (SqlExc& e) {
		ExceptionMessage("updating from linenumber", e);
	}
}

void InputDlg::UpdateFromTime()
{
	dirty = true;
	int h = edit_time_h.GetData();
	int m = edit_time_m.GetData();

	if (h < 0) h = 0;
	if (m < 0) m = 0;

	edit_time_h.SetData(h);
	edit_time_m.SetData(m);
	edit_total_m.SetData(((h*60)+m));
}

void InputDlg::UpdateFromTotalTime()
{
	dirty = true;
	int h;
	int m;
	int total = edit_total_m.GetData();

	if (total < 0) total = 0;	// minimaal 0
	h = ffloor(total/60);		// aantal uren afgerond naar beneden
	m = total - (h*60);			// overgebleven minuten

	edit_time_h.SetData(h);
	edit_time_m.SetData(m);
	edit_total_m.SetData(total);
}


//////////////////////////////////////////////////
//
// Update dropdown listboxes from database
//

void InputDlg::UpdateAllLists()
{
	String str;
	try {
		// Dynamic lists
		UpdateLinenumberList();

		// Monteurs list
		edit_monteur.Clear();
		SQL & Select(SqlAll())	.From(PERSONS)
								.Where(P_ACTIVE == true)
								.OrderBy(FIRST_NAME, MID_NAME, LAST_NAME);
		while (SQL.Fetch()) {
			str = SQL[FIRST_NAME];
			if (!SQL[MID_NAME].IsNull()) str += " " + SQL[MID_NAME].ToString();
			if (!SQL[LAST_NAME].IsNull()) str += " " + SQL[LAST_NAME].ToString();
			edit_monteur.Add(SQL[P_ID], str);
		}

		// Activities list
		edit_activity.Clear();
		SQL & Select(SqlAll())	.From(ACTIVITIES)
								.Where(ACT_ACTIVE == true)
								.OrderBy(ACT_LONG);
		while(SQL.Fetch()) {
			edit_activity.Add(SQL[ACT_ID], SQL[ACT_LONG].ToString());
		}

		// Issuetypes list
		edit_issuetype.Clear();
		SQL & Select(SqlAll())	.From(ISSUETYPES)
								.Where(TYPE_ACTIVE == true)
								.OrderBy(TYPE_LONG);
		while(SQL.Fetch()) {
			edit_issuetype.Add(SQL[TYPE_ID], SQL[TYPE_LONG].ToString());
		}

		// Departments list
		SQL & Select(SqlAll()).From(DEPARTMENTS)
								.Where(DEP_ACTIVE == true)
								.OrderBy(DEP_SHORT);
		while(SQL.Fetch()) {
			edit_department.Add(SQL[DEP_ID],SQL[DEP_SHORT].ToString());
		}

	}
	catch (SqlExc& e) {
		ExceptionMessage("updating lists", e);
	}
}

void InputDlg::UpdateLinenumberList()
{
	SqlBool where;
	int i=0, id;

	edit_linenumber.Clear();

	if ((int)edit_department.GetData() > 0)
		where = (L_ACTIVE == true && DEPARTMENT == edit_department.GetData());
	else
		where = (L_ACTIVE == true);

	SQL & Select(SqlAll())	.From(LINES)
							.Where(where)
							.OrderBy(LINE_NUMBER);
	while(SQL.Fetch()) {
		id = SQL[L_ID];
		edit_linenumber.Add(id,Format("%s   : %s",SQL[LINE_NUMBER].ToString(),SQL[LINE_DESCRIPTION].ToString()));
		i++;
	}
	if (i==1) {
		edit_linenumber.SetData(id);
		UpdateMachineList();
	}
}

void InputDlg::UpdateMachineList()
{
	int i=0, id;

	edit_machine.Clear();
	if ((int)edit_linenumber.GetData() > 0) {
		SQL & Select(SqlAll())	.From(MACHINES)
								.Where(M_ACTIVE == true &&
								       LINE == edit_linenumber.GetData() )
								.OrderBy(MACHINE_DESCRIPTION);
		while (SQL.Fetch()) {
			id = SQL[M_ID]; //save last id
			edit_machine.Add(id, SQL[MACHINE_DESCRIPTION].ToString());
			i++;
		}
		if (i==1) edit_machine.SetData(id);
	}
}

void InputDlg::Reset(){
	edit_department.SetData(0);
	UpdateLinenumberList();

	edit_activity.SetData(0);
	edit_issuetype.SetData(0);
	edit_time_h.SetData(0);
	edit_time_m.SetData(0);
	edit_total_m.SetData(0);
	edit_probleem.Clear();
	edit_oorzaak.Clear();
	edit_oplossing.Clear();
	edit_opmerking.Clear();

	//edit_monteur.SetFocus();
	ActiveFocus(edit_monteur);
	dirty = false;
}



//////////////////////////////////////////////////
//
// Check fields and insert to database
//

void InputDlg::ConfirmStore()
{
	bool error = false;
	bool warning = false;
	String errorMsg = "De volgende fouten zijn gevonden:";
	int person_id;
	Value date;
	Value shift;
	Value total_minutes;
	int machine_id;
	int activity_id;
	int issuetype_id;


	person_id = edit_monteur.GetData();
	if (!(person_id>0)) {
		errorMsg += "\n - Monteur niet ingevoerd";
		error = true;
	}
	date = edit_date.GetData();
	if (date.IsNull()) {
		errorMsg += "\n - Datum niet ingevoerd";
		error = true;
	}
	shift = edit_ploeg.GetData();
	if (shift.IsNull()) {
		errorMsg += "\n - Ploeg niet ingevuld";
		error = true;
	}
	machine_id = edit_machine.GetData();
	if (!(machine_id>0)) {
		errorMsg += "\n - Machine niet ingevuld";
		error = true;
	}
	activity_id = edit_activity.GetData();
	if (!(activity_id>0)) {
		errorMsg += "\n - Type storing niet ingevuld";
		error = true;
	}
	issuetype_id = edit_issuetype.GetData();
	if (!(issuetype_id>0)) {
		errorMsg += "\n - Soort storing niet ingevuld";
		error = true;
	}
	total_minutes = edit_total_m.GetData();
	if (total_minutes.IsNull()) {
		errorMsg += "\n - tijd niet ingevuld";
		error = true;
	}

	if (!error) {
		errorMsg = "Waarschuwing:";

		if (!total_minutes.IsNull() && !((int)total_minutes>0)) {
			errorMsg += "\n - tijd is nul";
			warning = true;
		}
		if (!edit_probleem.IsDirty()) {
			errorMsg += "\n - Probleem niet ingevuld";
			warning = true;
		}
		if (!edit_oorzaak.IsDirty()) {
			errorMsg += "\n - Oorzaak niet ingevuld";
			warning = true;
		}
		if (!edit_oplossing.IsDirty()) {
			errorMsg += "\n - Oplossing niet ingevuld";
			warning = true;
		}
	}

	if (error) {
		Exclamation(DeQtfLf(errorMsg));
		return;
	}
	else if (!warning) errorMsg = "Geen problemen gevonden.";
	errorMsg += "\n\nData invoeren?";
	if(!PromptYesNo(DeQtfLf(errorMsg))) return;
	try {
		SQL & Insert(ISSUELOG)
						(PERSON, person_id)
						(ACTIVITY, activity_id)
						(ISSUETYPE, issuetype_id)
						(MACHINE, machine_id)
						(DAY, (Date)date)
						(SHIFT, shift)
						(MINUTES, total_minutes)
						(ISSUE, TrimBoth(edit_probleem.GetData()))
						(CAUSE, TrimBoth(edit_oorzaak.GetData()))
						(SOLUTION, TrimBoth(edit_oplossing.GetData()))
						(REMARK, TrimBoth(edit_opmerking.GetData()));
	}
	catch (SqlExc& e) {
		ExceptionMessage("inserting new record", e);
		return;
	}

	LOG(SQL.ToString());
	status.Temporary(Format("Record %d ingevoerd.", SQL.GetInsertedId()),5000);
	Reset();
}

void InputDlg::ConfirmClose()
{
	if (dirty)
		if (!PromptOKCancel("Wijzigingen worden niet opgeslagen!")) return;
	Break();
}