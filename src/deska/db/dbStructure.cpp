#include "dbStructure.h"

#include <sstream>
#include <iostream>

namespace Deska {

namespace DB {

	//adds new attribute into structure	
	//design for use with db view with schema string tab_name, string att_name, string att_type
	void DbStructure::addAttribute(string tab_name, string att_name, string att_type){
		
		TAttribute name_type = pair<string,string>(att_name,att_type);
		TTables::iterator item = tables.find(tab_name);		
		//if this table is in tables, we should add this attribute into map of attributes
		if( item != tables.end()){			
			if(item->second.find(att_name) == item->second.end())			
				item->second.insert(name_type);
		}
		else{
			TAttributes atts;
			atts.insert(name_type);
			TTable tab = TTable(tab_name,atts);
			tables.insert(tab);
		}

	}


	//from names delimited by space makes list of string names
	TAttsNames parseAttsNames(string names){
		list<string> parsedNames;
		stringstream stream(names);
		string name;
		while( getline(stream, name, ' ') ){
			parsedNames.push_back(name);
		}
		return parsedNames;
	}

	//adds new structured four (name of table t1, attributes of t1, name of referenced table t2, attributes of t2)
	//design for use with db view with schema string tab_name, string atts_names, string reftab_name, string refatts_names
	void DbForeignKeys::addForeignKey(string tab_name, string atts_names, string reftab_name, string refatts_names){
		
		TAttsNames names = parseAttsNames(atts_names);
		TAttsNames refnames = parseAttsNames(refatts_names);
		TTabAttsNames reftabatts = TTabAttsNames(reftab_name,refnames);
		TAttsRefAtsPair arap= TAttsRefAtsPair(names,reftabatts);
		TAttsRefAtts attsrefatts;
		attsrefatts.insert(arap);

		TForeignKeys::iterator itable = foreignKeys.find(tab_name);
		if(itable != foreignKeys.end()){
			TAttsRefAtts::iterator iattsratts = itable->second.find(names);
			if(iattsratts == itable->second.end()){
				//exists reference from this table (tab_name), but not from this attributes (atts_names)
				itable->second.insert(arap);
			}			
		}
		else
		{			
			TForeignKey fkey = TForeignKey(tab_name,attsrefatts);
			foreignKeys.insert(fkey);
		}

	}

	vector<string> DbForeignKeys::getReferencedTables(string tab_name){		
		TForeignKeys::iterator ifk = foreignKeys.find(tab_name);		
		vector<string> result;
		for( TAttsRefAtts::iterator it = ifk->second.begin(); it != ifk->second.end(); ++it){
			//tabs name from atts,(tab_name,atts)
			result.push_back(it->second.first);
		}
		return result;
	}

	void testAddAttribute(){
		string dbOutput [4] [3] = {{"vendor","vname","name"},{"vendor","vid","id"},{"vendor","vdate","date"},{"hwmodel","hwtype","text"}};
		DbStructure dbst;
		for(int i=0;i<4;++i){
			dbst.addAttribute(dbOutput[i][0],dbOutput[i][1],dbOutput[i][2]);			
		}
	}

	void testAddForeignKey(){
		string dbOutput [4] [4] = {{"hwmodel","vendor","vendor","vname"},{"hwmodel","vendor guarantee","vendor","vendor guarantee"},{"vendor","vdate","dates","date"},{"hwmodel","hwtype","hwtypes","type"}};
		 DbForeignKeys dbfk;
		for(int i=0;i<4;++i){
			dbfk.addForeignKey(dbOutput[i][0],dbOutput[i][1],dbOutput[i][2],dbOutput[i][3]);		
		}		
		vector<string> vs = dbfk.getReferencedTables("vendor");
		vs = dbfk.getReferencedTables("hwmodel");
	}
	
}
}

int main(int argc, char **argv){
	Deska::DB::testAddAttribute();
	Deska::DB::testAddForeignKey();
	return 0;
}