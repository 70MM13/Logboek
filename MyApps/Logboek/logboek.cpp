#include "logboek.h"

#include "history.h"
#include "input.h"
#include "journal.h"

#ifdef flagADMIN
#include "manager.h"
#endif


// implementation of database
#include <Sql/sch_schema.h>
#include <Sql/sch_source.h>


#ifdef flagADMIN
bool CreateNewDatabase()
{
	return false;
}
#endif


void Logboek::MainMenu(Bar& bar)
{
	bar.Add("Bestand", THISBACK(FileMenu));
	bar.Add("Database", THISBACK(DatabaseMenu))
		.Help(main_cfg.database);
}


void Logboek::FileMenu(Bar& bar)
{
	bar.Add("Sluiten", THISBACK(Close));
}


void Logboek::DatabaseMenu(Bar& bar)
{
	bar.Add("Info", THISBACK(ShowDBInfo))
		.Help("Informatie over database");
	bar.Add("Openen", THISBACK(ChangeDB))
		.Help("Open andere database");
#ifdef flagADMIN
	bar.Add("Nieuw", THISBACK(NewDB))
		.Help("Creëer nieuwe database");
#endif
}


#ifdef flagADMIN
void Logboek::NewDB()
{
//	CreateNewDatabase();
	FileSelector fs;
	fs.Type("Database", "*.db");
	fs.Set(/*ConfigFile(*/"logboek.db"/*)*/);
	if (!fs.ExecuteSaveAs("Nieuwe database")) return;
	if (FileExists(fs.Get()))
		if (!DeleteFile(fs.Get())) {
			Exclamation(DeQtfLf(Format("Kan bestand niet verwijderen:\n\"%s\"", fs.Get())));
			return;
		}

	try
	{
		Sqlite3Session &sql_session = Single<Sqlite3Session>();
		WaitCursor waitcursor;

		CloseDB();

		if(!sql_session.Open(fs.Get())) {
			Exclamation("SQLite can't create or open database file");
			return;
		}

		SQL = sql_session;

		SqlSchema sch(SQLITE3);
		All_Tables(sch);
		SqlPerformScript(sch.Upgrade());
		SqlPerformScript(sch.Attributes());
		SqlPerformScript(sch.ConfigDrop());
		SqlPerformScript(sch.Config());

		main_cfg.database = fs.Get();
 	}
	catch(SqlExc& e)
	{
		ExceptionMessage("Creating new database", e);
		return;
	}

	PromptOK(DeQtfLf(Format("Nieuwe database gemaakt:\n%s",fs.Get())));
	OpenDB();
}
#endif


void Logboek::ShowDBInfo()
{
	String info;
	info += "Database: " + main_cfg.database;
	info += "\n" + AsString(GetFileLength(main_cfg.database)) + " bytes";
	PromptOK(DeQtfLf(info));
}


void Logboek::ChangeDB()
{
	if (SelectDB()) OpenDB();
}


bool Logboek::SelectDB()
{
	FileSelector fs;
	fs.Type("Database", "*.db");
	fs.Set(ConfigFile("logboek.db"));
	if (!fs.ExecuteOpen("Selecteer database")) return false;
	else {
		main_cfg.database = fs.Get();
		return true;
	}
}


void Logboek::CloseDB()
{
	Sqlite3Session &sql_session = Single<Sqlite3Session>();
	sql_session.Close();
	if (sql_session.IsOpen()) Exclamation("Database niet gesloten");
	b_edit.Disable();
	b_his.Disable();
	b_inv.Disable();
	b_log.Disable();
	b_man.Disable();
}


bool Logboek::OpenDB()
{
	CloseDB();

	Sqlite3Session &sql_session = Single<Sqlite3Session>();
#ifdef _DEBUG
	sql_session.SetTrace();
#endif
	sql_session.LogErrors(true);
	sql_session.ThrowOnError();

	if(!sql_session.Open(main_cfg.database)) {
		Exclamation(Format("SQLite can't create or open database file\n\"%s\"",main_cfg.database));
		return false;
	}

	SQL = sql_session;

	try {
		SQL.ExecuteX("PRAGMA foreign_keys = ON");
		// verify if all tables and rows are there
		SQL & Select(ID, PERSON, ACTIVITY, ISSUETYPE, MACHINE, ISSUE, CAUSE, SOLUTION, REMARK, DAY, SHIFT, MINUTES).From(ISSUELOG);
		SQL & Select(P_ID, FIRST_NAME, MID_NAME, LAST_NAME).From(PERSONS);
		SQL & Select(ACT_ID, ACT_ACTIVE, ACT_SHORT, ACT_LONG).From(ACTIVITIES);
		SQL & Select(TYPE_ID, TYPE_ACTIVE, TYPE_SHORT, TYPE_LONG).From(ISSUETYPES);
		SQL & Select(M_ID, M_ACTIVE, LINE, MACHINETYPE, MACHINE_DESCRIPTION).From(MACHINES);
		SQL & Select(MTYPE_ID, MTYPE_ACTIVE, MTYPE_DESCRIPTION).From(MACHINETYPES);
		SQL & Select(L_ID, L_ACTIVE, DEPARTMENT, LINE_NUMBER, LINE_DESCRIPTION).From(LINES);
		SQL & Select(DEP_ID, DEP_ACTIVE, DEP_SHORT, DEP_LONG).From(DEPARTMENTS);
	}
	catch (SqlExc& e) {
		String str = "Bestand kan niet worden geopend als Logboek database\n";
		str << "\"" << main_cfg.database << "\"";
#ifdef _DEBUG
		str << "\n\n" << e;
#endif
		Exclamation("[ [* " + DeQtfLf(str) + "]]");
		return false;
	}

	dbCache.UpdateCache(typeAll);
	b_edit.Enable();
	b_his.Enable();
	b_inv.Enable();
	b_log.Enable();
#ifdef flagADMIN
	b_man.Enable();
#endif
	return true;
}


Logboek::Logboek()
{
	CtrlLayout(*this, "");
	Title("Storing administratie").Sizeable();

	AddFrame(statusbar);
	AddFrame(menubar);
	menubar.Set(THISBACK(MainMenu));
	menubar.WhenHelp = statusbar;

	statusbar.Temporary((Format("Logboek v2.0 (%s)", __DATE__)),5000);
	statusbar.WhenLeftDown = THISBACK(about);

	Icon(LogboekImg::SmallIcon());
	LargeIcon(LogboekImg::LargeIcon());

	b_edit <<= THISBACK(edit);
	b_his <<= THISBACK(historie);
	b_inv <<= THISBACK(invoeren);
	b_log <<= THISBACK(show_log);
	b_man <<= THISBACK(manager);
	b_exit <<= THISBACK(Close);

	CloseDB();
	Show();

	if(!FileExists(main_cfg.database)) {
		if (!SelectDB()) return;
	}
	OpenDB();
}


void Logboek::invoeren()
{
	InputDlg().Run();
}


void Logboek::show_log()
{
	JournalDlg().Run();
}


void Logboek::historie()
{
	HistoryDlg().Run();
}


void Logboek::edit()
{
	PromptOK( "edit");
}


void Logboek::manager()
{
#ifdef flagADMIN
	ManagerDlg().Run();
#endif
}
