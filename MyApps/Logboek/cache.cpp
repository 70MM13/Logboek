#include "logboek.h"

_dbCache dbCache;

void _dbCache::UpdateCache(cacheType type)
{
	String str;
	try {
		if(type==typeAll || type==typePersons) {
			Persons.Clear();
			SQL & Select(P_ID, FIRST_NAME, MID_NAME, LAST_NAME).From(PERSONS);
			while(SQL.Fetch()) {
				str = TrimRight(SQL[FIRST_NAME].ToString());
				str += TrimRight(" " + SQL[MID_NAME].ToString());
				str += TrimRight(" " + SQL[LAST_NAME].ToString());
				Persons.Add(SQL[P_ID], str);
			}
		}
		if(type==typeAll || type==typeActivities) {
			Activities.Clear();
			SQL & Select(SqlAll()).From(ACTIVITIES);
			while(SQL.Fetch()) {
				Activities.Add(SQL[ACT_ID], SQL[ACT_LONG].ToString());
			}
		}
		if(type==typeAll || type==typeIssuetypes) {
			Issuetypes.Clear();
			SQL & Select(SqlAll()).From(ISSUETYPES);
			while(SQL.Fetch()) {
				Issuetypes.Add(SQL[TYPE_ID], SQL[TYPE_LONG].ToString());
			}
		}
		if(type==typeAll || type==typeLines) {
			Lines.Clear();
			SQL & Select(SqlAll()).From(LINES);
			while(SQL.Fetch()) {
				str = SQL[LINE_NUMBER].ToString();
				str << " (" << SQL[LINE_DESCRIPTION].ToString() << ")";
				Lines.Add(SQL[L_ID], str);
			}
		}
		if(type==typeAll || type==typeDepartments) {
			Departments.Clear();
			SQL & Select(SqlAll()).From(DEPARTMENTS);
			while(SQL.Fetch()) {
				Departments.Add(SQL[DEP_ID], SQL[DEP_SHORT].ToString());
			}
		}
		if(type==typeAll || type==typeMachinetypes) {
			Machinetypes.Clear();
			SQL & Select(SqlAll()).From(MACHINETYPES);
			while(SQL.Fetch()) {
				Machinetypes.Add(SQL[MTYPE_ID], SQL[MTYPE_DESCRIPTION].ToString());
			}
		}
		if(type==typeAll || type==typeLineMachines) {
			LineMachines.Clear();
			SQL & Select(SqlAll()).From(MACHINES);
			while(SQL.Fetch()) {
				str = Get(typeLines, SQL[LINE]);
				str += " /" + TrimRight(" " + SQL[MACHINE_DESCRIPTION].ToString());
				LineMachines.Add(SQL[M_ID], str);
			}
		}
	}
	catch (SqlExc& e) {
		ExceptionMessage("caching lists", e);
	}
}

String _dbCache::Get(cacheType type, int i)
{
	if (IsNull(i)) return "<leeg>";
	switch (type) {
		case typePersons: return Persons.Get(i, "[N/A]");
		case typeActivities: return Activities.Get(i, "[N/A]");
		case typeIssuetypes: return Issuetypes.Get(i, "[N/A]");
		case typeLines:	return Lines.Get(i, Format("[N/A:%i]",i));
		case typeDepartments: return Departments.Get(i, Format("[N/A:%i]",i));
		case typeMachinetypes: return Machinetypes.Get(i, "[N/A]");
		case typeLineMachines: return LineMachines.Get(i, "[N/A]");
		default: // should not happen
	#ifdef _DEBUG
				ASSERT_(0, "_dbCache::Get(type, i): Undefined type");
	#endif
			return "[_dbCache::Get(?)]";
	}
}

