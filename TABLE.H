#include "setclass.h"
#include "sstr.h"
#include "rowcol.h"
#include "key.h"
#include "wherec.h"

#ifndef TABLE
class Table
{
public:
Table();
Table(int cols);
Table(int cols, const char *name);
~Table();
void AddColumn(const char *name, int size, int prec, int sqltype, int namelength);
void AddColumn(Columns *cs) {AddColumn(cs->name, cs->colsize, cs->precision, cs->type, strlen(cs->name) + 1);}
void AddData(char *data, int size, int col);
int NewRow(); //initializes the next row for AddData and returns the rowindex if necessary
int NewRows(int newrows); //Add in a bunch of new rows
int Find(char *val, int row, int col, int setsize); //Returns the row index, else -1
int ColIndex(const char *name);

char *GetData(int row, int col);
void SetData(int row, int col, const char *val, int size);
void Sort(int row, int col, int setsize); //A generic sort everything function
void PrintStats(FILE *out);
int BytesUsed();
int Mallocs();
int Count(char *val, int row, int col, int setsize);
void DropRow(int row);
int Stat(int col, int type); //Column info
int AddRow(Row *inp); //Copies the row over directly
void SortColumns(); //to avoid having to sort the column info over and over and over
void AddToPrimaryKey(int col) { PrimKey.AddToKey(&orderedCols[col]); }
int AddForeignKey(Table *inp);
int ForeignKeyExists(Table *inp);
void GroupBy(int startrow, int setsize, Columns *cs, int dep);
void GroupBy(int startrow, int setsize, PrimaryKey *pk);
int NextDiff(int row, int setsize, Columns *cs, int dep); //finds the next row with a different value in the columns cs, for dep columns
int NextDiff(int row, int setsize, PrimaryKey *pk);
Row *GetRow(int row) {return rowdata[row];}
void SaveOrder(); //To avoid continuous sorting
int LoadOrder();
int RunSelect(WhereClause *wc, int row, int size);
int SwapRows(int r1, int r2);
int Evaluate(wcinfo *clause, int clausecount, RowSet *rs);
int Compare(int row, int col, const char *val);
void PrintTable(FILE *fp);

char **cols; //names
int columns;
Row **rowdata;
Row **oldorder;
int rows;
int rowsize;
char *name;
int lastcol;  //Last column sorted by on this table
Columns *sortedCols; //For quicker indexing of columns by their names
Columns *orderedCols; //Ordered by their visual order
PrimaryKey PrimKey;
ForeignKey **For; //for how ever many foreign keys there are
int fkc; //foreign key count
int clct; //rows counted in the Find function while it was searching to the end of the set of rows
unsigned char flags;
};
#define TABLE
#endif
