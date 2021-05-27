#ifndef _logboek_report_h_
#define _logboek_report_h_

#include <RepGen/RepGen.h>

// definitions of layouts
#define LAYOUTFILE <Logboek/report.lay>
#include <CtrlCore/lay.h>


class ReportGenerator {

private:
	RepGen rep;
	String rep_title;
	SqlBool rep_where;
	Date rep_default_date;
	int rep_default_shift;

public:
	void SetWhere (SqlBool where) { rep_where = where; };
	void SetTemplate (String templateQTF, String title="") {
		rep.SetTemplate(templateQTF);
		rep_title = title;
	};
	void SetDate (Date date) { rep_default_date = date; };
	void SetShift (int shift) { rep_default_shift = shift; };
	
	String GetResult () { return rep.GetResult(); };
	String GetResult (SqlBool where) { SetWhere(where); return GetResult(); };
	String GetResult (String templateQTF) { SetTemplate(templateQTF); return GetResult(); };
	String GetResult (SqlBool where, String templateQTF) { SetWhere(where); SetTemplate(templateQTF); return GetResult(); };
	void doPrint(bool defaultprinter = false);
	void Perform () { rep.Perform(); };
	void Perform ( SqlBool where ) { SetWhere(where); Perform(); };

private:
	bool FetchResult;
	int counter;
	
	void doPreview();

	void doRepSubstitute(bool body=false);
	
	void repCallbackReportStart();
	void repCallbackReportVar();
	void repCallbackReportFinish();
	void repCallbackCalculateStart();
	void repCallbackCalculateBody();
	void repCallbackCalculateFinish();
	
	typedef ReportGenerator CLASSNAME;

public:
	ReportGenerator();
};

#endif
