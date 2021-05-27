#include "logboek.h"

// implementation of Images
#define IMAGEFILE <Logboek/logboek.iml>
#define IMAGECLASS LogboekImg
#include <Draw/iml_source.h>
 
MainConfig main_cfg;

INITBLOCK {
	//RegisterGlobalConfig("main-config");
}

GUI_APP_MAIN
{
	SetLanguage(LNG_('N', 'L', 'N', 'L'));
	UseHomeDirectoryConfig(true);
#ifdef _DEBUG
	StdLogSetup(LOG_FILE|LOG_TIMESTAMP|LOG_ROTATE(5)/*, ConfigFile("logboek.log")*/);
#else
	StdLogSetup(LOG_FILE|LOG_TIMESTAMP|LOG_APPEND|LOG_ROTATE(2)/*, ConfigFile("logboek.log")*/);
#endif
	LoadFromFile(main_cfg, ConfigFile());

	Logboek().Run();
	StoreToFile(main_cfg, ConfigFile());
}

void ExceptionMessage(String when, SqlExc& e)
{
	RLOG(Format("SQL exception while %s: %s", when, e));
	Exclamation("[ [ [* SQL exception while " + DeQtfLf(when) + ":]&][ [* " + DeQtfLf(e) + "]]]");
	SQL.Clear();
	SQL.Rollback();
}
