#ifndef _Logboek_import_h_
#define _Logboek_import_h_

//#if defined(PLATFORM_WIN32) && defined(COMPILER_MSC)

using namespace Upp;

#define LAYOUTFILE <Logboek/import/import.lay>
//#define LAYOUTFILE "import.lay"
#include <CtrlCore/lay.h>


template <class PloegType>
PloegType TimeToPloeg(Time& t, PloegType ND, PloegType DD, PloegType LD) {
	if (t.hour >= 15) return LD;
	if (t.hour >= 7) return DD;
	return ND;
}


class importDlg : public WithImportLayout<TopWindow> {
	typedef importDlg CLASSNAME;

public:
	importDlg();
	~importDlg();

private:
	FileSelector fs;
	Thread work;

	bool FileOpenedOk;
	volatile Atomic ImportRunning;
	volatile Atomic StopImport;
	volatile Atomic pnum;
	volatile Atomic pnum_max;

	int num_Storingstypen;
	int num_Storingssoort;
	int num_Monteurs;
	int num_Afdeling_Lijnnummer;
	int num_Lijnnummer_Machinenummer;
	int num_Machinepark;
	int num_Storingen;
	
	void Log(const String& s);
	void openfile();
	void openfile_t();
	void start_stop();
	void import();
	void import_t();
	void update_progress();

	ArrayMap <int, int> monteur_kloknummer_index;
	int import_monteur(int kloknummer, String voornaam, String achternaam, bool actief);
	ArrayMap <String, int> storingstype_index;
	int import_storingstype(String code, String omschrijving, bool actief);
	ArrayMap <String, int> storingssoort_index;
	int import_storingssoort(String code, String omschrijving, bool actief);
	ArrayMap <String, int> afdeling_index;
	int import_afdeling(String afdeling, bool actief);
	ArrayMap <int, int> lijnnummer_index;
	int import_lijnnummer(int lijnnummer, String afdeling, String omschrijving, bool actief);
	ArrayMap <String, int> machinetypes_index;
	ArrayMap <String, int> line_vs_machinetype_index;
	int import_machinenummer(int lijnnummer, int machinenummer, String omschrijving, bool actief);
	ArrayMap <int, int> storing_index;
	int import_storing(int old_storing_id, int &person_id, Date &datum, int ploeg, int &total_minutes,
						int& machine_id, int& activity_id, int& issuetype_id,
						String& storing, String& oorzaak, String& oplossing, String& opmerking);
	 
};

void import_logboek_mdb ();


//#endif

#endif