#include "logboek.h"
#include "report.h"
//#include "report.brc"

ReportGenerator::ReportGenerator()
{
	rep_title = "";
	rep_where.SetTrue();
	rep_default_date = Null;

	rep.RepGenReportStart	  = THISBACK(repCallbackReportStart);
	rep.RepGenReportVar       = THISBACK(repCallbackReportVar);
	rep.RepGenReportFinish    = THISBACK(repCallbackReportFinish);
	rep.RepGenCalculateStart  = THISBACK(repCallbackCalculateStart);
	rep.RepGenCalculateBody   = THISBACK(repCallbackCalculateBody);
	rep.RepGenCalculateFinish = THISBACK(repCallbackCalculateFinish);
}

void ReportGenerator::doPrint(bool defaultprinter)
{
	Report report;
	report.Clear();
	report << GetResult();
	if (defaultprinter) DefaultPrint(report, 1, rep_title);
	else Print(report, 1, rep_title);
}

void ReportGenerator::doRepSubstitute(bool body)
{
	LOG("\tdoRepSubstitute");

	Date ReportDate = rep_default_date;
	int shift = rep_default_shift;
	String ReportShift;
	String	ReportActivity		= "",
			ReportIssuetype		= "",
			ReportPerson		= "",
			ReportDepartment	= "",
			ReportLine			= "",
			ReportMachine		= "",
			ReportMachinetype	= "",
			ReportIssue			= "",
			ReportCause			= "",
			ReportSolution		= "",
			ReportRemark		= "";
	int ReportMinutes = 0;

	if (FetchResult)
	{
		ReportDate = SQL[DAY];
		shift = (int)SQL[SHIFT];
		ReportActivity		= DeQtfLf(TrimBoth(dbCache.Get(typeActivities, SQL[ACTIVITY])));
		ReportIssuetype		= DeQtfLf(TrimBoth(dbCache.Get(typeIssuetypes, SQL[ISSUETYPE])));
		ReportPerson		= DeQtfLf(TrimBoth(dbCache.Get(typePersons, SQL[PERSON])));
		ReportDepartment	= DeQtfLf(TrimBoth(SQL[DEP_SHORT].ToString()));
		ReportLine			= DeQtfLf(TrimBoth(SQL[LINE_NUMBER].ToString()));
		ReportMachine		= DeQtfLf(TrimBoth(SQL[MACHINE_DESCRIPTION].ToString()));
		ReportMachinetype	= DeQtfLf(TrimBoth(SQL[MTYPE_DESCRIPTION].ToString()));
		ReportIssue			= DeQtfLf(TrimBoth(SQL[ISSUE].ToString()));
		ReportCause			= DeQtfLf(TrimBoth(SQL[CAUSE].ToString()));
		ReportSolution		= DeQtfLf(TrimBoth(SQL[SOLUTION].ToString()));
		ReportRemark		= DeQtfLf(TrimBoth(SQL[REMARK].ToString()));
		ReportMinutes		= SQL[MINUTES];
	}

	switch (shift) {
		case 1 : ReportShift = "dagdienst"; break;
		case 2 : ReportShift = "late dienst"; break;
		case 3 : ReportShift = "nachtdienst"; break;
		default : ReportShift = "invalid shift";
	}

	// var
	if (!body) {
		rep.SubstVar("##DATE", AsString(ReportDate)); // AsString(edit_date.GetData()));
		rep.SubstVar("##DAG", Format("%Dy", DayOfWeek(ReportDate)));
		rep.SubstVar("##PLOEG", ReportShift);
	}
	
	// body
	if (body) {
		rep.SubstVar("##ACT",		ReportActivity);
		rep.SubstVar("##TYPE",		ReportIssuetype);
		rep.SubstVar("##MONTEUR",	ReportPerson);
		rep.SubstVar("##AFDELING",	ReportDepartment);
		rep.SubstVar("##LIJN",		ReportLine);
		rep.SubstVar("##MACHINE",	ReportMachine);
		rep.SubstVar("##MACHINETYPE", ReportMachinetype);
		rep.SubstVar("##STORING",	ReportIssue);
		rep.SubstVar("##OORZAAK",	ReportCause);
		rep.SubstVar("##OPLOSSING", ReportSolution);
		rep.SubstVar("##OPMERKING", ReportRemark);
		rep.SubstVar("##TIJD",		FormatElapsedMin(ReportMinutes));
	}
	
}

void ReportGenerator::repCallbackReportStart()
{
	LOG("repCallbackReportStart");

	counter = 0;

	SQL & Select(SqlAll())
			.From(ISSUELOG)
			.InnerJoin(MACHINES)		.On(MACHINE		.Of(ISSUELOG) == M_ID.Of(MACHINES))
			.InnerJoin(MACHINETYPES)	.On(MACHINETYPE	.Of(MACHINES) == MTYPE_ID.Of(MACHINETYPES))
			.InnerJoin(LINES)			.On(LINE		.Of(MACHINES) == L_ID.Of(LINES))
			.InnerJoin(DEPARTMENTS)		.On(DEPARTMENT	.Of(LINES)	  == DEP_ID.Of(DEPARTMENTS))
			.Where(rep_where);
	LOG(SQL.ToString());

	FetchResult = SQL.Fetch();
}

void ReportGenerator::repCallbackCalculateStart()
{
	LOG("repCallbackCalculateStart");
}

void ReportGenerator::repCallbackReportVar()
{
	LOG("repCallbackReportVar");
	doRepSubstitute();
}

void ReportGenerator::repCallbackReportFinish()
{
	LOG("repCallbackReportFinish");
	rep.SubstVar("##TOTAAL", AsString(counter));
}

void ReportGenerator::repCallbackCalculateBody()
{
	LOG("repCallbackCalculateBody");

	while (FetchResult) {
		rep.RepGenCalculateBodyBeforeSubst();  // <- VERY IMPORTANT: START OF CYCLE
		counter++;
		doRepSubstitute(true);
		rep.RepGenCalculateBodyAfterSubst(); // <- VERY IMPORTANT: END OF CYCLE

		FetchResult = SQL.Fetch();
	}
}

void ReportGenerator::repCallbackCalculateFinish()
{
	LOG("repCallbackCalculateFinish");
}
