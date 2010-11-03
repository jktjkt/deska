#ifndef DESKA_DB_DBSTRUCTURE_H
#define DESKA_DB_DBSTRUCTURE_H

#include <map>
#include <vector>
#include <list>

using namespace std;

namespace Deska {

namespace DB {

	//tables and attributes
	
	struct strcomp {
		bool operator() (const string &lhs, const string &rhs) const{
			return lhs.compare(rhs) < 0;
		}
	};

	//map<att_name,att_type>
	typedef map<string, string,strcomp> TAttributes;
	typedef pair<string,string> TAttribute;
	//map<tab_name,tab_atts>
	typedef map<string,TAttributes,strcomp> TTables;
	typedef pair<string,TAttributes> TTable;
	

	class DbStructure{
		TTables tables;		
	public:
		void addAttribute(string tab_name, string att_name, string att_type);		
	};


	// foreign keys typedef map<string>
	
	//list<att_name>
	typedef list<string> TAttsNames;
	
	//table and some of its attributes
	//pair<tab_name,atts_names>
	typedef pair<string,TAttsNames> TTabAttsNames;
	//attributes of given table referencing table and its attributes
	typedef map<TAttsNames,TTabAttsNames> TAttsRefAtts;
	typedef pair<TAttsNames,TTabAttsNames> TAttsRefAtsPair;
	
	struct TAttsNamescomp{
		bool operator()( const TAttsNames &a, const TAttsNames const &b){						
			if(a.size() == b.size()){
				TAttsNames anames = a;
				TAttsNames bnames = b;
				anames.sort();
				bnames.sort();
				TAttsNames::iterator itb = bnames.begin();
				for(TAttsNames::iterator ita = anames.begin(); ita!=anames.end();++ita){
					if( ita->compare(*itb) != 0){
						return ita->compare(*itb) < 0;
					}
					++itb;
				}
			}
			else
				return (a.size() - b.size()) < 0;
		}		
	};
	

	typedef map<string,TAttsRefAtts,strcomp> TForeignKeys;
	typedef pair<string,TAttsRefAtts> TForeignKey;

	class DbForeignKeys{
		TForeignKeys foreignKeys;
	public:
		void addForeignKey(string tab_name, string atts_names, string reftab_name, string refatts_names);
		vector<string> getReferencedTables(string tab_name);
	};
	
	
}

}

#endif