#include "sstr.h"
#include "table.h"
#include "wherec.h"
#include "key.h"

#ifndef DATABASE
class database
{
public:
database();
~database();
int AddTable(Table *tb);
int NewTable();
int NewTable(const char *name);
void DropTable(int handle);
void AutoCopy(int src, int dst); //copies data over from the src to dst where the column name and type and precision are equal
Table *tab(int handle);
void PrintTable(int handle, FILE *out);
void PrintStats(FILE *out);
int ColIndex(int handle, const char *name);
int Count(int handle, char *name, char *val); //count the rows with a value of val in column name
int Count(int handle, int colc, char *val);
int DupeTable(int handle, const char *name); //Duplicate and returns the duplicate handle
void DropRow(int handle, int row);
int NewRow(int handle) { return db[FindIndex(handle)]->NewRow(); }
void SetData(int handle, int rowindex, const char *name, char *val);
void SetData(int handle, int rowindex, int colindex, char *val);
int Find(int handle, int col, char *val) {int ind; return db[ind = FindIndex(handle)]->Find(val, 0, col, db[ind]->rows); }
void Sort(int handle, int col) {int val; db[val = FindIndex(handle)]->Sort(0, col, db[val]->rows);}
char *GetData(int handle, int rowindex, const char *name);
char *GetData(int handle, int rowindex, int colindex);
int Stat(int handle, int ID); //Gets handy info about a table
void Compare(int tab1h, int tab2h, FILE *out);
int AddForeignKey(int dst, int parent) {return db[FindIndex(dst)]->AddForeignKey(db[FindIndex(parent)]);}
void AddToPrimaryKey(int dst, int col) { db[FindIndex(dst)]->AddToPrimaryKey(col); }
Row *GetRow(int handle, int row) {return db[FindIndex(handle)]->GetRow(row);}
int AddRow(int handle, Row *inp) {return db[FindIndex(handle)]->AddRow(inp);}
int NewRows(int handle, int rows) {return db[FindIndex(handle)]->NewRows(rows);}
void SaveOrder(int handle) {db[FindIndex(handle)]->SaveOrder(); }
int LoadOrder(int handle) {return db[FindIndex(handle)]->LoadOrder(); }
void SetWhere(int handle, int whereindex, int col, const char *val, int oper, int logic, int eval);
int AddWhere();
void DeleteWhere(int index);
int RunSelect(int handle);
void SelectRowSubSet(int r1, int r2);
Table *ListTables();

int BytesUsed();
int Mallocs();

private:
int FindIndex(int handle); //Translates handles into array indices
Table **db;
int *Handles;
int Tables;
int TabCount;
Table *tabcmp[4];
WhereClause *wcs;
WhereClause wc;
int whereclauses;
int lowrow;
int highrow;
int currTabLength;
};
#define DATABASE
#endif
