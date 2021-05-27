#ifndef _logboek_logboek_h
#define _logboek_logboek_h

#include <CtrlLib/CtrlLib.h>
#include <SqlCtrl/SqlCtrl.h>
#include <DropGrid/DropGrid.h>
#include <plugin/sqlite3/Sqlite3.h>

#include "util.h"

using namespace Upp;

// definitions of layouts
#define LAYOUTFILE <Logboek/logboek.lay>
#include <CtrlCore/lay.h>

// definitions of Images
#define IMAGEFILE <Logboek/logboek.iml>
#define IMAGECLASS LogboekImg
#include <Draw/iml_header.h>

// definitions of database
#define SCHEMADIALECT <plugin/sqlite3/Sqlite3Schema.h>
#define MODEL <Logboek/logboek.sch>
#include <Sql/sch_header.h>

//extern Sqlite3Session sqlite3_session;

/*
	Global string arrays, caching named lists for db relations
*/

enum cacheType {
	typeAll,
	typePersons,
	typeActivities,
	typeIssuetypes,
	typeLines,
	typeDepartments,
	typeMachinetypes,
	typeLineMachines
};

struct _dbCache {
	ArrayMap<int, String> Persons;
	ArrayMap<int, String> Activities;
	ArrayMap<int, String> Issuetypes;
	ArrayMap<int, String> Lines;
	ArrayMap<int, String> Departments;
	ArrayMap<int, String> Machinetypes;
	ArrayMap<int, String> LineMachines;

	void UpdateCache(cacheType type);
	String Get(cacheType type, int i);
};

extern _dbCache dbCache;


/*
	convertion routines
*/

struct ConvertPersonId : public Convert {
	Value Format(const Value& v) const { return String().Cat() <<  dbCache.Get(typePersons, v); }
};

struct ConvertActivityId : public Convert {
	Value Format(const Value& v) const { return String().Cat() <<  dbCache.Get(typeActivities, v); }
};

struct ConvertIssuetypeId : public Convert {
	Value Format(const Value& v) const { return String().Cat() <<  dbCache.Get(typeIssuetypes, v); }
};

struct ConvertLineId : public Convert {
	Value Format(const Value& v) const { return String().Cat() <<  dbCache.Get(typeLines, v); }
};

struct ConvertDepartmentId : public Convert {
	Value Format(const Value& v) const { return String().Cat() <<  dbCache.Get(typeDepartments, v);	}
};

struct ConvertMachinetypeId : public Convert {
	Value Format(const Value& v) const { return String().Cat() << dbCache.Get(typeMachinetypes, v); }
};

struct ConvertMachineId : public Convert {
	Value Format(const Value& v) const { return String().Cat() << dbCache.Get(typeLineMachines, v); }
};

struct ConvertBool : public Convert {
	Value Format(const Value& v) const {
		//return v.ToString() + "(" + AsString(v.GetType()) + ")";

		if (v=="1" || v==true) return "●";
		if (v=="0" || v==false) return "";
		return v.ToString() + "?";

	}
};

struct ConvertShift : public Convert {
	Value Format(const Value& v) const {
		switch((int)v) {
			case 0 : return "ND";
			case 1 : return "DD";
			case 2 : return "LD";
			default: return v.ToString() + "?";
		}
	}
};


/*
	Custom error messaging
*/

void ExceptionMessage(String when, SqlExc& e);

/*
	Custom statusbar
*/

class myStatusBar: public StatusBar {
public:
	Callback WhenLeftDown;
	Callback WhenLeftDouble;
	Callback WhenRightDown;

    virtual void LeftDown(Point pos, dword flags) {
        WhenLeftDown();
    }
    virtual void LeftDouble (Point p, dword keyflags) {
        WhenLeftDown();
    }
    virtual void RightDown (Point p, dword keyflags) {
        WhenRightDown();
    }
};

/*
	Global functions
*/
bool CreateNewDatabase();

/*
	App class
*/

class Logboek : public WithLogboekLayout<TopWindow> {
public:
	typedef Logboek CLASSNAME;
	Logboek();

private:
//	Sqlite3Session sqlite3;

	myStatusBar statusbar;
	MenuBar menubar;
	void MainMenu(Bar& bar);
	void FileMenu(Bar& bar);
	void DatabaseMenu(Bar& bar);
	void ShowDBInfo();
	void ChangeDB();
	void NewDB();
	bool SelectDB();
	bool OpenDB();
	void CloseDB();

	void invoeren();
	void show_log();
	void historie();
	void edit();
	void manager();
	void about();
};


/*
	Config
*/
struct MainConfig {
	MainConfig() {
		database = "logboek.db";
	};

	String database;

	void Serialize(Stream& stream) {
		stream % database;
	};
};

extern MainConfig main_cfg;

#endif
