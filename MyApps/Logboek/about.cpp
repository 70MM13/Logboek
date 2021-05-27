#include <ide/version.h>
//#include <plugin/sqlite3/sqlite3.h>
//#include <plugin/sqlite3/lib/sqleet.h>
#include <plugin/sqlite3/lib/sqlite3mc_amalgamation.h>

#include "logboek.h"

using namespace Upp;

void Logboek::about()
{
	String VerInfo = "v2.0";

#ifdef flagADMIN
	VerInfo << " +Admin";
#endif
#ifdef flagIMPORT
	VerInfo << " +Import";
#endif
#ifdef _DEBUG
	VerInfo << " DEBUG";
#endif

	PromptOK(Format("[ [ Logboek “STEWAD” %s&][ Copyright ©2014 W.J.Kroes&][ Using Ultimate`+`+ %s and SQLite %s&][ Mem: %d Kb&][ %s]]", VerInfo, IDE_VERSION, sqlite3_version, MemoryUsedKb(), DeQtfLf(ConfigFile()) ));	//MemoryProfileInfo();
}
