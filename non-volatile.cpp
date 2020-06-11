#include "pch.h"
#include "expr.h"
#include "CJsonObject.hpp"

using namespace std;
using namespace neb;
extern map<string,TABLE> tables;
extern RecordManager rm;
#define GETBLOCK(x)
void non_volatile_init(void){
    // GETBLOCK(s);
	string s(rm.bufmanager.getBlock((char *)"data.txt", 0)->data);
    CJsonObject root(s);
    int n=root.GetArraySize();
    for(int i=0;i<n;i++){
        string name=root[i]("name");
        int b,t;
        root[i].Get("file",b);
        root[i].Get("size",t);
        TABLE table(b,t);
        CJsonObject &col=root[i]["collum"];
        for(int j=0;j<col.GetArraySize();j++){
            string name=col[j]("name"),type;
            int o,size,u;
            col[j].Get("offset",o);
            col[j].Get("size",size);
            col[j].Get("type",type);
            col[j].Get("unique",u);

            ATTRIBUTE collum(type,o,size,u);
            table.collum[name]=collum;
            //table.order.push_back(table.collum.find(name));
        }
        CJsonObject &idx=root[i]["index"];
        for(int j=0;j<idx.GetArraySize();j++){
            string name=idx[j]("name"),atrb=idx[j]("atrb");
            int b;
            idx[j].Get("file",b);
            INDEX IDX(b,atrb);
            table.index[name]=IDX;
        }
		if(table.index.count("0"))table.primary_key=table.index["0"].indexed_attributes;
        tables[name]=table;
		for(int j=0;j<col.GetArraySize();j++){
			string nm=col[j]("name");
			tables[name].order.push_back(tables[name].collum.find(nm));
		}
    }
}
void non_volatile_flush(void){
    CJsonObject root;
    for(auto table_it=tables.begin();table_it!=tables.end();table_it++){
        CJsonObject table;
        table.Add("name",table_it->first);
        table.Add("file",table_it->second.table_address.block_number);
        table.Add("size",table_it->second.tuple_size);
        /*if(!table_it->second.primary_key.empty())
			table.Add("pk",table_it->second.primary_key[0]);
        else table.AddNull("pk");*/
        table.AddEmptySubArray("collum");
        for(int i=0;i<table_it->second.order.size();i++){
            auto atrb=table_it->second.order[i];
            CJsonObject col;
            col.Add("name",atrb->first);
            col.Add("type",atrb->second.type);
            col.Add("offset",atrb->second.offset);
            col.Add("size",atrb->second.size);
            col.Add("unique",atrb->second.unique);
            table["collum"].Add(col);
        }
        table.AddEmptySubArray("index");
        for(auto it=table_it->second.index.begin();it!=table_it->second.index.end();it++){
            CJsonObject index;
            index.Add("name",it->first);
            index.Add("file",it->second.root_node_address.block_number);
            index.Add("atrb",it->second.indexed_attributes[0]);
            table["index"].Add(index);
        }
        root.Add(table);
    }
    cout<<root.ToFormattedString()<<endl;
	auto des=rm.bufmanager.getBlock((char*)"data.txt",0)->data;
	memcpy(des,root.ToString().data(),root.ToString().length()+1);
}