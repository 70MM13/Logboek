//#if defined(PLATFORM_WIN32) && defined(COMPILER_MSC)

#include "../logboek.h"
#include "import.h"

#include <ODBC/ODBC.h>

//OleDBSession odb;
ODBCSession odb;

/*
	import dialog class and functions
*/

importDlg::importDlg()
{
	CtrlLayout(*this, "Importeer STEWAD logboek (.MDB)");

	progress.Set(0,100);

	b_open <<= THISBACK(openfile);
	b_process <<= THISBACK(start_stop);
	b_exit <<= THISBACK(Close);

	ImportRunning = false;
	b_process.Disable();

	Log(Format("Import on %`", GetSysTime()));
	Log("Ready to open file");
}

importDlg::~importDlg()
{
	//work.ShutdownThreads();
	StopImport = 1;
	LOG ("~importDlg()");
	//while (ImportRunning) Sleep(10);


	//work.Wait();
}

void importDlg::update_progress()
{
	GuiLock gl;
	//Log("update");
	progress.Set(pnum, pnum_max);
	//progress.Sync();
	if (ImportRunning) SetTimeCallback(250, THISBACK(update_progress));
}

void importDlg::Log(const String& s)
{
	LOG("import: " + s);
	GuiLock gl;
	WString line;
	line << "[" << AsString(GetSysTime()).ToWString() << "] " << s.ToWString() << "\n";
	logtext.SetEditable();
	logtext.MoveTextEnd();
	logtext.Paste(line);
	logtext.SetReadOnly();
	logtext.Sync();
}

void importDlg::openfile()
{
	work.Run(THISBACK(openfile_t));
}

void importDlg::openfile_t()
{
	if (ImportRunning) return;
	StopImport = 0;
	ImportRunning = 1;
	fs.Type("mdb files", "*.mdb");


	if (!fs.ExecuteOpen("Open Storingsdata.MDB")) {
		Log("No file selected");
		ImportRunning = 0;
		return;
	}
	else {
		Log(Format("File selected: \"%`\"", fs.Get()));
	}

	pnum=0;
	pnum_max=0;
	update_progress();

	num_Storingstypen = 0;
	num_Storingssoort = 0;
	num_Monteurs = 0;
	num_Afdeling_Lijnnummer = 0;
	num_Lijnnummer_Machinenummer = 0;
	num_Machinepark = 0;
	num_Storingen = 0;

	b_process.Enable(false);
	FileOpenedOk = false;
	//Driver={Microsoft Access Driver (*.mdb)};Dbq=C:\mydatabase.mdb;Uid=Admin;Pwd=;
//	if(!odb.Open("","",fs.Get(),"Microsoft.Jet.OLEDB.4.0") ){
	if(!odb.Connect(Format("Driver={Microsoft Access Driver (*.mdb)};Dbq=%s;Uid=Admin;Pwd=;", fs.Get()) ) ) {
		Log("Can't create or open database file");
		ImportRunning = 0;
		return;
	}
	else Log(Format("Successfully connected %`", fs.Get()));

	Log(odb.GetLastError());

	#ifdef _DEBUG
		odb.SetTrace();
	#endif

	odb.ThrowOnError(true);
	Sql sqldb(odb);

	try {
		Log("Check:");
		sqldb.ExecuteX("SELECT * FROM Storingstypen");
		while(sqldb.Fetch()){
			num_Storingstypen++;
		}
		Log(Format("\t%d x Storingstypen", num_Storingstypen));

		sqldb.ExecuteX("SELECT * FROM Storingssoort");
		while(sqldb.Fetch()){
			num_Storingssoort++;
		}
		Log(Format("\t%d x Storingssoort", num_Storingssoort));

		sqldb.ExecuteX("SELECT * FROM Monteurs");
		while(sqldb.Fetch()){
			num_Monteurs++;
		}
		Log(Format("\t%d x Monteurs", num_Monteurs));

		sqldb.ExecuteX("SELECT * FROM Afdeling_Lijnnummer");
		while(sqldb.Fetch()){
			num_Afdeling_Lijnnummer++;
		}
		Log(Format("\t%d x Afdeling_Lijnnummer", num_Afdeling_Lijnnummer));

		sqldb.ExecuteX("SELECT * FROM Lijnnummer_Machinenummer");
		while(sqldb.Fetch()){
			num_Lijnnummer_Machinenummer++;
		}
		Log(Format("\t%d x Lijnnummer_Machinenummer", num_Lijnnummer_Machinenummer));

		sqldb.ExecuteX("SELECT * FROM Machinepark");
		while(sqldb.Fetch()){
			//LOG(Format("%s | %d | %s", sqldb[0], sqldb[1], sqldb[2]));
			num_Machinepark++;
		}
		Log(Format("\t%d x Machinepark", num_Machinepark));

		sqldb.ExecuteX("SELECT * FROM Storingen");
		while(sqldb.Fetch()){
			if(StopImport) break;
			num_Storingen++;
		}
		Log(Format("\t%d x Storingen", num_Storingen));

	}
	catch (SqlExc &e){
		Log("Sql exception: " + e);
		ImportRunning = 0;
		return;
	}

	if(StopImport) {
		ImportRunning = 0;
		return;
	}

	if (!(num_Storingstypen > 0	&& num_Storingssoort > 0
		&& num_Monteurs > 0		&& num_Afdeling_Lijnnummer > 0
		&& num_Lijnnummer_Machinenummer > 0		&& num_Machinepark > 0
		&& num_Storingen > 0))
	{
		Log	("One or more required tables not found");
		return;
	}

	Log("Database check ok - ready for import");
	b_process.Enable();
	FileOpenedOk=true;
	ImportRunning = 0;
}

void importDlg::start_stop()
{
	if (ImportRunning) {
		StopImport = true;
	}
	else {
		StopImport = false;
		import();
	}
}


/*
	import function
*/
void importDlg::import()
{
	work.Run(THISBACK(import_t));
}


void importDlg::import_t()
{
	ImportRunning = true;
	{
		GuiLock __;
		b_process.SetLabel("Stop");
	}

	int num;
	int imp;
	TimeStop timer;
	String msg;
	String logMsg;

	Date	t_Datum;
	Time	t_Aanvangstijd;
	int		t_Duur_in_minuten;
	int		t_Kloknummer;
	String	t_Monteur_achternaam;
	String	t_Monteur_voornaam;
	String	t_Storingssoort;
	String	t_Storingstype;
	String	t_Afdeling;
	int		t_Lijnnummer;
	int		t_Machinenummer;
	String	t_MachineOmschrijving;
	String	t_Aard;
	String	t_Oorzaak;
	String	t_Oplossing;
	String	t_Opmerking;
	int person_id;
	int machine_id;
	int activity_id;
	int issuetype_id;

	Log("Starting import");
	LOG(Format("Starting import at %`",GetSysTime()));

	update_progress();

	try {
		odb.ThrowOnError(true);
		Sql sqldb(odb);

		//Import: Monteurs
		timer.Reset();
		pnum =0;
		pnum_max = num_Monteurs;
		num = 0;
		imp=0;
		msg = "Importing table: Monteurs";
		Log (msg);
		LOG (msg);
		{
			GuiLock __;
			statusline.SetText(msg);
			statusline.Sync();
		}
		sqldb.ExecuteX("SELECT * FROM Monteurs");
		while(sqldb.Fetch()){
			if(StopImport) break;
			LOG(Format("%d | %s | %s",sqldb[0], sqldb[1], sqldb[2]));
			t_Kloknummer = sqldb[0];
			t_Monteur_voornaam = sqldb[1];
			t_Monteur_achternaam = sqldb[2];
			person_id = import_monteur(t_Kloknummer, t_Monteur_voornaam, t_Monteur_achternaam, true);
			if(0<person_id){
				LOG("\tSuccesfully imported");
				imp++;
			}
			num++;
			pnum = num;
			//progress.Set(num,num_Monteurs);
			//progress.Sync();
		}
		msg = Format("Imported %d/%d records from Monteurs", imp, num);
		msg += Format(" -- %.", FormatElapsedSec((int)timer.Seconds()));
		Log(msg);

		//Import: Storingstypen
		//
		timer.Reset();
		pnum=0;
		pnum_max = num_Storingstypen;
		num=0;
		imp=0;
		msg = "Importing table: Storingstypen";
		Log (msg);
		LOG (msg);
		{
			GuiLock __;
			statusline.SetText(msg);
			statusline.Sync();
		}
		sqldb.ExecuteX("SELECT * FROM Storingstypen");
		while(sqldb.Fetch()){
			if(StopImport) break;
			LOG(Format("%s | %s",sqldb[0], sqldb[1]));
			if (0<import_storingstype(sqldb[0], sqldb[1], true)) {
				LOG("\tSuccesfully imported");
				imp++;
			}
			num++;
			pnum = num;
			//progress.Set(num,num_Storingstypen);
			//progress.Sync();
		}
		msg = Format("Imported %d/%d records from Storingstypen", imp, num);
		msg += Format(" -- %.", FormatElapsedSec((int)timer.Seconds()));
		Log(msg);

		//Import: Storingssoort
		//
		timer.Reset();
		pnum=0;
		pnum_max=num_Storingssoort;
		num=0;
		imp=0;
		msg = "Importing table: Storingssoort";
		Log (msg);
		LOG (msg);
		{
			GuiLock __;
			statusline.SetText(msg);
			statusline.Sync();
		}
		sqldb.ExecuteX("SELECT * FROM Storingssoort");
		while(sqldb.Fetch()){
			if(StopImport) break;
			LOG(Format("%s | %s",sqldb[0], sqldb[1]));
			if (0<import_storingssoort(sqldb[0], sqldb[1],true )){
				LOG("\tSuccesfully imported");
				imp++;
			}
			num++;
			pnum=num;
			//progress.Set(num,num_Storingssoort);
			//progress.Sync();
		}
		msg = Format("Imported %d/%d records from Storingssoort", imp, num);
		msg += Format(" -- %.", FormatElapsedSec((int)timer.Seconds()));
		Log(msg);

		//Import: Afdeling_Lijnnummer
		//
		timer.Reset();
		pnum=0;
		pnum_max=num_Afdeling_Lijnnummer;
		num=0;
		imp=0;
		msg = "Importing table: Afdeling_Lijnnummer";
		Log (msg);
		LOG (msg);
		{
			GuiLock __;
			statusline.SetText(msg);
			statusline.Sync();
		}

		sqldb.ExecuteX("SELECT * FROM Afdeling_Lijnnummer");
		while(sqldb.Fetch()){
			if(StopImport) break;
			LOG(Format("%d | %s | %s",sqldb[0], sqldb[1], sqldb[2]));
			t_Lijnnummer = sqldb[0];
			t_Afdeling = sqldb[1];

			if (0<import_afdeling(t_Afdeling, true )){
				LOG("\tSuccesfully imported Afdeling");
			}
			if (0<import_lijnnummer(t_Lijnnummer, t_Afdeling, sqldb[2], true )){
				LOG("\tSuccesfully imported Lijnnummer");
				imp++;
			}

			num++;
			pnum=num;
			//progress.Set(num,num_Afdeling_Lijnnummer);
			//progress.Sync();
		}
		msg = Format("Imported %d/%d records from Afdeling_Lijnnummer", imp, num);
		msg += Format(" -- %.", FormatElapsedSec((int)timer.Seconds()));
		Log(msg);

		//Import: Lijnnummer_Machinenummer
		//
		timer.Reset();
		pnum=0;
		pnum_max=num_Lijnnummer_Machinenummer;
		num = 0;
		imp=0;
		msg = "Importing table: Lijnnummer_Machinenummer";
		Log (msg);
		{
			GuiLock __;
			statusline.SetText(msg);
			statusline.Sync();
		}
		sqldb.ExecuteX("SELECT * FROM Lijnnummer_Machinenummer");

		while(sqldb.Fetch()){
			//Sleep(10);
			if(StopImport) break;
			LOG(Format("%d | %d | %s",sqldb[0], sqldb[1], sqldb[2]));
			t_Lijnnummer = sqldb[0];
			t_Machinenummer = sqldb[1];
			t_MachineOmschrijving = sqldb[2];
			if(0<import_machinenummer(t_Lijnnummer, t_Machinenummer, t_MachineOmschrijving, true)){
				LOG("\tSuccesfully imported Machinenummer");
				imp++;
			}
			num++;
			pnum=num;
			//progress.Set(num,num_Lijnnummer_Machinenummer);
			//progress.Sync();
		}
		msg = Format("Imported %d/%d records from Lijnnummer_Machinenummer", imp, num);
		msg += Format(" -- %.", FormatElapsedSec((int)timer.Seconds()));
		Log(msg);

		//Import: Storingen
		//
		timer.Reset();
		pnum = 0;
		pnum_max = num_Storingen;
		num = 0;
		imp = 0;
		msg = "Importing table: Storingen";
		if (o_filter.Get()!=0) msg += " (FILTER ENABLED)";
		Log (msg);
		LOG (msg);
		{
			GuiLock __;
			statusline.SetText(msg);
			statusline.Sync();
		}
		sqldb.ExecuteX("SELECT * FROM Storingen");

		while(sqldb.Fetch()){
			if(StopImport) break;
			LOG(Format("\n%d\t| %s | %s | %s | %s | %d | %s | %s | %s | %s | %d | %s | %d",
				sqldb[0], AsString(sqldb[1]), sqldb[2], sqldb[3], sqldb[4], sqldb[5], AsString(sqldb[6]),
				sqldb[7], sqldb[8], sqldb[9], sqldb[10], AsString(sqldb[11]), sqldb[12]));

			// nummer en datum
			t_Datum = sqldb[1];
			logMsg =  Format("\t: %s", FormatDate(t_Datum, "DD/MM/YYYY"));
			// Aanvangstijd, Duur_in_minuten
			t_Aanvangstijd =  sqldb[11];
			t_Duur_in_minuten = sqldb[12];
			logMsg += Format(" %s (%s) %d min.", FormatTime(t_Aanvangstijd,"hh:mm:ss"),
											TimeToPloeg(t_Aanvangstijd, "ND", "DD", "LD"),
											t_Duur_in_minuten);

			// Monteur (kloknummer - achternaam, voornaam)
			t_Kloknummer = sqldb[10];
			t_Monteur_achternaam = sqldb[8];
			t_Monteur_voornaam = sqldb[9];
			logMsg += Format("\t: %d - %s, %s", t_Kloknummer, t_Monteur_achternaam, t_Monteur_voornaam);
			person_id = import_monteur(t_Kloknummer, t_Monteur_voornaam, t_Monteur_achternaam, false);
			if (!(0<person_id)) {
				LOG("\tWARNING: Monteur not found");
				continue; // may be error...
			}

			// code: Storingssoort
			t_Storingssoort = sqldb[3];
			logMsg += "\n\tsrt=" + t_Storingssoort;
			issuetype_id = import_storingssoort(t_Storingssoort, t_Storingssoort, false);
			if (!(0<issuetype_id)) {
				LOG("\tWARNING: Storingssoort not found");
				continue;
			}

			// code: Storingstype
			t_Storingstype = sqldb[2];
			logMsg += " typ=" + t_Storingstype;
			activity_id = import_storingstype(t_Storingstype, t_Storingstype, false);
			if (!(0<activity_id)) {
				LOG("\tWARNING: Storingstype not found");
				continue;
			}

			// Afdeling, Lijn, Machine, Machine_omschrijving
			t_Afdeling = sqldb[4];
			t_Lijnnummer = sqldb[5];
			t_Machinenummer = sqldb[6];
			t_MachineOmschrijving = sqldb[7];
			logMsg += Format("\tafd=%s; lijn=%d; mach=%d (%s)", t_Afdeling, t_Lijnnummer, t_Machinenummer, t_MachineOmschrijving);
			import_afdeling(sqldb[4], false);
			import_lijnnummer(t_Lijnnummer, t_Afdeling,Format("*Lijn %d (Auto-import)",t_Lijnnummer),false);
			machine_id = import_machinenummer(t_Lijnnummer, t_Machinenummer, t_MachineOmschrijving, false);
			if (!(0<machine_id)) {
				LOG("\tWARNING: Machinenummer not found");
				continue;
			}

			//meldingen
			t_Aard =		sqldb[13];
			t_Oorzaak =		sqldb[14];
			t_Oplossing =	sqldb[15];
			t_Opmerking =	sqldb[16];
			logMsg += Format ("\n\t\"%s\" -- \"%s\" -- \"%s\" -- \"%s\"",
								t_Aard.Left(30),
								t_Oorzaak.Left(30),
								t_Oplossing.Left(30),
								t_Opmerking.Left(30));

			//no filter, or types: PR, PR-ON, ST, W_LOG, WZH
			if((o_filter.Get()==0) |
					t_Storingstype == "PR" |
					t_Storingstype == "PR-ON" |
					t_Storingstype == "ST" |
					t_Storingstype == "W_LOG" |
					t_Storingstype == "WZH")
			{
				LOG(logMsg);
				if(0<import_storing(sqldb[0], person_id, t_Datum, TimeToPloeg(t_Aanvangstijd, 0, 1, 2), t_Duur_in_minuten,
								machine_id, activity_id, issuetype_id,
								t_Aard, t_Oorzaak, t_Oplossing, t_Opmerking))
				{
					imp++;
				}
			}
			else {
				LOG ("-> Skipped");
			}

			num++;
			pnum = num;
			//progress.Set(num,num_Storingen);
			//progress.Sync();
		}

		msg = Format("Imported %d/%d records from Storingen", imp, num_Storingen);
		msg += Format(" -- %.", FormatElapsedSec((int)timer.Seconds()));
		Log(msg);
		{
			GuiLock gl;
			statusline.SetText(msg);
		}
		Log("Import finished.");
	}
	catch (SqlExc &e){
		Log("Sql exception: " + e);
		Log("Import aborted.");
	}

	if(StopImport) {
		Log("User break, import aborted.");
	}

	{
		GuiLock gl;
		ImportRunning = 0;
		b_process.SetLabel("Start");
	}
}

void import_logboek_mdb () {
importDlg().Run();
}

/*
	Functions: import to main SQL db

*/

int importDlg::import_monteur(int kloknummer, String voornaam, String achternaam, bool actief)
{
	String mid_name = "";
	int pos;

	pos=achternaam.Find(',');
	if (pos>0) {
		mid_name = TrimBoth(achternaam.Mid(pos+1));
		achternaam.Trim(pos);
	}

	// kloknummer in cache?
	int c = monteur_kloknummer_index.Get(kloknummer, -1);
	if (0<c) return c;

	// kloknummer in database?
	SQL & Select(P_ID).From(PERSONS)
		.Where(P_NUMBER == kloknummer);
	if(SQL.Fetch()) {
		monteur_kloknummer_index.Add(kloknummer, (int)SQL[P_ID]);
		return SQL[P_ID];
	}

	// kloknummer/monteur toevoegen
	SQL & Insert(PERSONS)(P_NUMBER, kloknummer)
						(FIRST_NAME, TrimBoth(voornaam))
						(MID_NAME, mid_name)
						(LAST_NAME, TrimBoth(" " + achternaam))
						(P_ACTIVE, actief);
	LOG(SQL.ToString());
	if (SQL.WasError()){
		Log(Format("SQL error: %s",SQL.GetLastError()));
		return -1;
	}
	else {
		monteur_kloknummer_index.Add(kloknummer, SQL.GetInsertedId());
		return SQL.GetInsertedId();
	}
}

int importDlg::import_storingstype(String code, String omschrijving, bool actief)
{
	// code in cache?
	int c = storingstype_index.Get(code, -1);
	if (0<c) return c;

	// code in database?
	SQL & Select(ACT_ID).From(ACTIVITIES)
		.Where(ACT_SHORT == code);
	if(SQL.Fetch()) {
		storingstype_index.Add(code, SQL[ACT_ID]);
		return SQL[ACT_ID];
	}

	// importeren
	SQL & Insert(ACTIVITIES)	(ACT_SHORT, code)
								(ACT_LONG, TrimBoth(omschrijving))
								(ACT_ACTIVE, actief);
	LOG(SQL.ToString());
	if (SQL.WasError()){
		Log(Format("SQL error: %s",SQL.GetLastError()));
		return -1;
	}
	else {
		storingstype_index.Add(code, SQL.GetInsertedId());
		return SQL.GetInsertedId();
	}
}

int importDlg::import_storingssoort(String code, String omschrijving, bool actief)
{
	// code in cache?
	int c = storingssoort_index.Get(code, -1);
	if (0<c) return c;

	// code in database?
	SQL & Select(TYPE_ID).From(ISSUETYPES)
		.Where(TYPE_SHORT == code);
	if(SQL.Fetch()) {
		storingssoort_index.Add(code, SQL[TYPE_ID]);
		return SQL[TYPE_ID];
	}

	// importeren
	SQL & Insert(ISSUETYPES)	(TYPE_SHORT, code)
								(TYPE_LONG, TrimBoth(omschrijving))
								(TYPE_ACTIVE, actief);
	LOG(SQL.ToString());
	if (SQL.WasError()){
		Log(Format("SQL error: %s",SQL.GetLastError()));
		return -1;
	}
	else {
		storingssoort_index.Add(code, SQL.GetInsertedId());
		return SQL.GetInsertedId();
	}
}

int importDlg::import_afdeling(String afdeling, bool actief)
{
	// afdeling in cache?
	int c = afdeling_index.Get(afdeling, -1);
	if (0<c) return c;

	// afdeling in database?
	SQL & Select(DEP_ID).From(DEPARTMENTS)
		.Where(DEP_SHORT == afdeling);
	if(SQL.Fetch()) {
		afdeling_index.Add(afdeling, SQL[DEP_ID]);
		return SQL[DEP_ID];
	}

	// afdeling toevoegen
	SQL & Insert(DEPARTMENTS)	(DEP_SHORT, TrimBoth(afdeling))
								(DEP_ACTIVE, actief);
	LOG(SQL.ToString());
	if (SQL.WasError()){
		Log(Format("SQL error: %s", SQL.GetLastError()));
		return -1;
	}
	else {
		afdeling_index.Add(afdeling, SQL.GetInsertedId());
		return SQL.GetInsertedId();
	}

}

int importDlg::import_lijnnummer(int lijnnummer, String afdeling, String omschrijving, bool actief)
{
	/*int dep=0;
	SQL & Select(DEP_ID).From(DEPARTMENTS)
		.Where(DEP_SHORT == afdeling);
	if(SQL.Fetch()) dep = SQL[DEP_ID];
*/

	// vraag afdeling op
	int dep = import_afdeling(afdeling, actief);

	// lijnnummer in cache?
	int c = lijnnummer_index.Get(lijnnummer, -1);
	if (0<c) return c;

	// lijnnummer in database?
	SQL & Select(L_ID).From(LINES)
		.Where(LINE_NUMBER == lijnnummer);
	if(SQL.Fetch()) {
		lijnnummer_index.Add(lijnnummer, SQL[L_ID]);
		return SQL[L_ID];
	}

	// lijnnummer toevoegen
	SQL & Insert(LINES)	(LINE_NUMBER, lijnnummer)
						(DEPARTMENT, dep)
						(LINE_DESCRIPTION, TrimBoth(omschrijving))
						(L_ACTIVE, actief);
	LOG(SQL.ToString());
	if (SQL.WasError()){
		Log(Format("SQL error: %s",SQL.GetLastError()));
		return -1;
	}
	else {
		lijnnummer_index.Add(lijnnummer, SQL.GetInsertedId());
		return SQL.GetInsertedId();
	}

}

int importDlg::import_machinenummer(int lijnnummer, int machinenummer, String omschrijving, bool actief)
{
	int line=0;
	int mtId;
	//String mtDesc = Format("%d %s", machinenummer, omschrijving);
	String mtDesc = omschrijving;
	String mtLineMachine;

	// Get Index from MACHINETYPES, if not exist: create new and use new index
	//
	mtId = machinetypes_index.Get(mtDesc, -1);
	if (0>=mtId) {
		SQL & Select(MTYPE_ID).From(MACHINETYPES)
				.Where(MTYPE_DESCRIPTION == mtDesc);
		if(SQL.Fetch()) {
			mtId = SQL[MTYPE_ID];
		}
		else {
			SQL & Insert(MACHINETYPES) (MTYPE_ACTIVE, actief)
								(MTYPE_DESCRIPTION, TrimBoth(mtDesc));
			mtId = SQL.GetInsertedId();
		}
		machinetypes_index.Add(mtDesc, mtId);
	}

	LOG(SQL.ToString());
	if (SQL.WasError()){
		Log(Format("SQL error: %s",SQL.GetLastError()));
		return -1;
	}

	// find index for given line
	//
/*	SQL & Select(L_ID).From(LINES)
		.Where(LINE_NUMBER == lijnnummer);
	if(SQL.Fetch()) line = SQL[L_ID];
*/
	line = import_lijnnummer(lijnnummer, "auto-import-machinenummer", mtDesc, false);

	// lijn <-> machine relatie tbv cache
	mtLineMachine = Format("%d;%d", line, mtId);

	// in cache?
	int c = line_vs_machinetype_index.Get(mtLineMachine, -1);
	if (0<c) return c;

	// check if record allready exists and return number
	//
	SQL & Select(M_ID).From(MACHINES)
		.Where(LINE == line && MACHINETYPE==mtId);
	if(SQL.Fetch()) {
		line_vs_machinetype_index.Add(mtLineMachine, SQL[M_ID]);
		return SQL[M_ID];
	}

	// no record found, create new
	//
	SQL & Insert(MACHINES)	(LINE, line)
					//		(OLD_MACHINE_NUMBER,machinenummer)
							(MACHINE_DESCRIPTION, TrimBoth(omschrijving))
							(MACHINETYPE, mtId)
							(M_ACTIVE, actief);

	LOG(SQL.ToString());
	if (SQL.WasError()){
		Log(Format("SQL error: %s",SQL.GetLastError()));
		return -1;
	}
	else {
		line_vs_machinetype_index.Add(mtLineMachine, SQL.GetInsertedId());
		return SQL.GetInsertedId();
	}
}


int importDlg::import_storing(int old_storing_id, int &person_id, Date &datum, int ploeg, int &total_minutes,
								int& machine_id, int& activity_id, int& issuetype_id,
								String& storing, String& oorzaak, String& oplossing, String& opmerking)
{
	// in cache?
	int c = storing_index.Get(old_storing_id, -1);
	if (0<c) return c;

	// al in database?
	SQL & Select(ID).From(ISSUELOG)
		.Where(OLD_STORING_ID==old_storing_id);
	if (SQL.Fetch()) {
		storing_index.Add(old_storing_id, SQL[ID]);
		return SQL[ID];
	}

	// importeren
	LOG(Format("import_storing(%d, %s, %d, %d, %d, %d, %d, ...)", person_id, AsString(datum), ploeg,
				total_minutes, machine_id, activity_id, issuetype_id));

	SQL & Insert(ISSUELOG)
					(OLD_STORING_ID, old_storing_id)
					(PERSON, person_id)
					(ACTIVITY, activity_id)
					(ISSUETYPE, issuetype_id)
					(MACHINE, machine_id)
					(DAY, (Date)datum)
					(SHIFT, ploeg)
					(MINUTES, total_minutes)
					(ISSUE, TrimBoth(storing))
					(CAUSE, TrimBoth(oorzaak))
					(SOLUTION, TrimBoth(oplossing))
					(REMARK, TrimBoth(opmerking));

	LOG(SQL.ToString());
	if (SQL.WasError()){
		Log(Format("SQL error: %s",SQL.GetLastError()));
		return -1;
	}
	else {
		storing_index.Add(SQL.GetInsertedId());
		return SQL.GetInsertedId();
	}
}

//#endif