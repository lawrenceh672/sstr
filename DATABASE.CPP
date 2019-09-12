#include "database.h"

const char *TabCmpSameName = "Same Rows in %s of %s and %s";
const char *TabCmpDiffName = "Different Rows in %s of %s and %s";

int IndexCount = 0;

/*
database code
*/
database::database()
{
int loop;

Tables = 0;
TabCount = 0;
db = NULL;
Handles = NULL;
for(loop = 0;loop < 4;loop++)
   tabcmp[loop] = NULL;
whereclauses = 0;
lowrow = 0;
highrow = 0;
currTabLength = 0;
}

int database::NewTable()
{
Table *tab;
int ret;
char buf[100];

tab = new Table;
Handles = (int *)realloc(Handles, sizeof(int *) * (Tables + 1));
tab->name = (char *)malloc(11);
strcpy(tab->name, "default666"); //Might as well give it a default name
ret = AddTable(tab);
return ret;
}

int database::NewTable(const char *name)
{
Table *tab;
int ret, size;

tab = new Table;
if(name != NULL)
   {
   tab->name = (char *)malloc((size = strlen(name) + 1)); //Allocate and copy
   memcpy(tab->name, name, size);
   }
else
   tab->name = NULL;

Handles = (int *)realloc(Handles, sizeof(int *) * (Tables + 1));
ret = AddTable(tab);
return ret;
}

int database::AddTable(Table *tb)
{
db = (Table **)realloc(db, sizeof(Table *) * (Tables + 1));
db[Tables] = tb;
Handles[Tables] = TabCount; //Initially the handles is just the array index
Tables++;
TabCount++;
return TabCount - 1;
}

Table *database::tab(int handle)
{
return db[FindIndex(handle)];
}

database::~database()
{
int loop;

for(loop = 0;loop < Tables;loop++)
   delete db[loop];
   //DropTable(Handles[loop]);

free(db);
free(Handles);
}

void database::PrintTable(int handle, FILE *out)
{
int ind;
ind = FindIndex(handle);
db[ind]->PrintTable(out);
}

void database::AutoCopy(int srch, int dsth)
{
int loop, iloop, il2, ol1, ri1, ri2, src, dst;
char *val;
src = FindIndex(srch);
dst = FindIndex(dsth);

if(src == -1 || dst == -1)
   return;

ri1 = db[dst]->rows;
db[dst]->NewRows(db[src]->rows); //add a new row for each row in the source table
ri2 = db[dst]->rows;

for(loop = 0;loop < db[src]->columns;loop++)
   {
   for(iloop = 0;iloop < db[dst]->columns;iloop++)
      {  //check every column in the dst to see if it matches the src
      if(db[src]->orderedCols[loop].isEqualTo(&db[dst]->orderedCols[iloop]) )
         { //now copy every row from this column (iloop) to the newrow
         for(il2 = ri1;il2 < ri2;il2++)
            {
            val = db[src]->GetData(il2 - ri1, loop);
            db[dst]->SetData(il2, iloop, val, strlenBytes(val)); //Using strlenBytes to overcome a possible NULL value
            }
         }
      }
   }
}

int database::BytesUsed()
{
int count = 0;

count += _msize(db);
count += _msize(Handles);
count += Tables * sizeof(Table);
return count;
}

int database::Mallocs()
{
int count = 0;
count++;
count++;
count += Tables;
return count;
}

void database::PrintStats(FILE *out)
{
int loop, bu = 0, ma = 0;
int butot = 0, matot = 0, totrows = 0, memtot = 0;

butot = BytesUsed() + sizeof(database);
matot = Mallocs();

for(loop = 0;loop < Tables;loop++)
   {
   db[loop]->PrintStats(out);
   bu = db[loop]->BytesUsed();
   ma = db[loop]->Mallocs();
   fprintf(out, "Bytes Used %d\n", bu);
   fprintf(out, "Memory Allocations %d\n", ma);
   fprintf(out, "Bytes / Malloc %.2f\n", ((float)bu) / ((float)ma));
   fprintf(out, "Total Efficiency %3.5f\n", ((float)bu) / ((float)(bu + (ma * 8))));
   fprintf(out, "Total memory gobbled: %d\n\n", bu + (ma * 8));
   butot += bu;
   matot += ma;
   PrintTable(Handles[loop], out);
   totrows += db[loop]->rows;
   }
fprintf(out, "\nTotal Bytes %d\n", butot);
fprintf(out, "Total mallocs %d\n", matot);
fprintf(out, "Total total memory gobbled thru malloc %d\n", memtot = (butot + (matot * 8)));
fprintf(out, "Bytes / Malloc %.2f\n", ((float)butot) / ((float)matot));
fprintf(out, "Total Efficiency %3.5f\n", ((float)butot) / ((float)(butot + (matot * 8))));
fprintf(out, "Size of Row: %d\nSize of Column: %d\nSize of Table: %d\nSize of Database: %d\nFindIndex iterations: %d\n",sizeof(Row), sizeof(Columns), sizeof(Table), sizeof(database), IndexCount);
fprintf(out, "qsort/bsearch Integer Comparisons: %d\nqsort/bsearch String Comparisons: %d\nNULLstrcmp Calls: %d\nstrlenBytes Calls %d\n", IntegerComparisons, StringComparisons, NULLstrcmCmp, strlenBytesCnt);
}

int database::ColIndex(int handle, const char *name)
{
return db[FindIndex(handle)]->ColIndex(name);
}

void database::DropTable(int handle)
{
int loop, ind;
Table **tp;

ind = FindIndex(handle);

delete db[ind];
for(loop = ind + 1;loop < Tables;loop++)
   {
   db[loop - 1] = db[loop]; //Adjust the array on the right one over
   Handles[loop - 1] = Handles[loop];
   }
Tables--;
tp = (Table**)malloc(Tables * sizeof(Table *));
memcpy(tp, db, Tables * sizeof(Table *));
free(db);
db = tp;
Handles = (int *)realloc(Handles, Tables * sizeof(int));
}

int database::FindIndex(int handle)
{
int loop;

for(loop = 0;loop < Tables;loop++)
   {
   if(Handles[loop] == handle)
      {
      IndexCount += loop;
      currTabLength = db[loop]->rows;
      return loop;
      break;
      }
   }
return -1;
}

int database::Count(int handle, char *name, char *val)
{
int ind, colc;

ind = FindIndex(handle);
colc = db[ind]->ColIndex(name);

return db[ind]->Count(val, 0, colc, db[ind]->rows);
}

int database::Count(int handle, int colc, char *val)
{
int ind;

ind = FindIndex(handle);

return db[ind]->Count(val, 0, colc, db[ind]->rows);
}

int database::DupeTable(int handle, const char *name)
{
int ind, loop, ret;
char buffer[500];
Table *tab;


ind = FindIndex(handle);

ret = NewTable(name);
tab = db[FindIndex(ret)];

//copy all the columns in
for(loop = 0;loop < db[ind]->columns;loop++)
   tab->AddColumn(db[ind]->cols[loop], db[ind]->Stat(loop, SQLCOLSIZE), db[ind]->Stat(loop, SQLPREC), db[ind]->Stat(loop, SQLTYPE), strlenBytes(db[ind]->cols[loop]));

tab->SortColumns(); //We want to be able to look up columns in this new table
return ret; //and the handle of the new table
}

void database::DropRow(int handle, int row)
{
int ind;

ind = FindIndex(handle);

if(ind != -1) db[ind]->DropRow(row);
}

void database::SetData(int handle, int rowindex, const char *name, char *val)
{
int ind, colind;
ind = FindIndex(handle);

if(ind != -1)
   {
   colind = db[ind]->ColIndex(name);
   if(colind != -1)
      db[ind]->SetData(rowindex, colind, val, strlenBytes(val)); //Once again using strlenBytes to handle null values
   }
}
void database::SetData(int handle, int rowindex, int colindex, char *val)
{
int ind;
ind = FindIndex(handle);

if(ind != -1)
   db[ind]->SetData(rowindex, colindex, val, strlenBytes(val));
}

char *database::GetData(int handle, int rowindex, int colind)
{
int ind;
ind = FindIndex(handle);

if(ind != -1)
   return db[ind]->GetData(rowindex, colind);
else
   return NULL;
}

char *database::GetData(int handle, int rowindex, const char *name)
{
int ind, colind;
ind = FindIndex(handle);

if(ind != -1)
   {
   colind = db[ind]->ColIndex(name);
   if(colind != -1)
      return db[ind]->GetData(rowindex, colind);
   }
return NULL;
}

int database::Stat(int handle, int ID)
{
int ind;
ind = FindIndex(handle);

if(ind == -1)
   return 0;

if(ID == ROWS)
   return db[ind]->rows;
if(ID == COLS)
   return db[ind]->columns;
if(ID == TABS)
   return Tables;
}

void database::Compare(int tab1h, int tab2h, FILE *out)
{
int tabi1, tabi2, tpssisset, clp, matchedcols = 0, setsize, ind, loop, ret, rowindex, sisrow, sisset, iloop, forind, pkloop, columns;
char *tp, buf[500], tf = 0;
unsigned short *matchcols;
unsigned char *tab2rows;
Table *tab1, *tab2;
Columns *tab1k, *tab2k;
PrimaryKey *pk;

tabi1 = FindIndex(tab1h);
tabi2 = FindIndex(tab2h);
tab1 = db[tabi1];
tab2 = db[tabi2];

//Ensure that tab1 has a foreign key for tab2
if( (forind = tab1->ForeignKeyExists(tab2)) == -1)
   return;
else
   pk = &tab2->PrimKey;

//Make the new tables
sprintf(buf, TabCmpSameName, tab1->name, tab1->name, tab2->name);
ret = NewTable(buf);
tabcmp[0] = db[FindIndex(ret)];
//tabcmp[0] = db[FindIndex(DupeTable(buf))];

sprintf(buf, TabCmpSameName, tab2->name, tab1->name, tab2->name);
ret = NewTable(buf);
tabcmp[1] = db[FindIndex(ret)];
//tabcmp[1] = db[FindIndex(DupeTable(buf))];

sprintf(buf, TabCmpDiffName, tab1->name, tab1->name, tab2->name);
ret = NewTable(buf);
tabcmp[2] = db[FindIndex(ret)];
//tabcmp[2] = db[FindIndex(DupeTable(buf))];

sprintf(buf, TabCmpDiffName, tab2->name, tab1->name, tab2->name);
ret = NewTable(buf);
tabcmp[3] = db[FindIndex(ret)];
//tabcmp[3] = db[FindIndex(DupeTable(buf))];

//lets do it according to columns i specify
tab1k = tab1->PrimKey.GetCol(&ind);
tab2k = pk->GetCol(&ind);

columns = 15 + 1 + ind; //15 columns I choose, and the key columns, and the extra cuz everything is doubled

matchcols = (unsigned short *)malloc(sizeof(unsigned short) * 2 * (columns + 20));
matchcols[0] = 9;
matchcols[1] = 2;

matchcols[2] = 10;
matchcols[3] = 3;

matchcols[4] = 11;
matchcols[5] = 4;

matchcols[6] = 16;
matchcols[7] = 10;

matchcols[8] = 17;
matchcols[9] = 11;

matchcols[10] = 22;
matchcols[11] = 16;

matchcols[12] = 23;
matchcols[13] = 22;

matchcols[14] = 13;
matchcols[15] = 6;

matchcols[16] = 18;
matchcols[17] = 12;

matchcols[18] = 19;
matchcols[19] = 13;

matchcols[20] = 20;
matchcols[21] = 14;

matchcols[22] = 21;
matchcols[23] = 15;

matchcols[24] = 12;
matchcols[25] = 5;

matchcols[26] = 14;
matchcols[27] = 8;

matchcols[28] = 15;
matchcols[29] = 9;

matchedcols = columns - 1;
//Just stick the key columns in the front for all tables
memmove(&matchcols[ind * 2], matchcols, (columns - ind - 1) * 2 * sizeof(unsigned short));
for(loop = 0;loop < ind*2;loop=loop+2)
   {
   matchcols[loop] = tab1k[loop/2].col;
   matchcols[loop + 1] = tab2k[loop/2].col;
   }

for(clp = 0;clp < matchedcols;clp++)
   {
   tabcmp[0]->AddColumn(&tab1->orderedCols[matchcols[clp * 2]]); //tab1 same
   tabcmp[1]->AddColumn(&tab2->orderedCols[matchcols[clp * 2 + 1]]); //tab2 same
   tabcmp[2]->AddColumn(&tab1->orderedCols[matchcols[clp * 2]]); //tab1 diff
   tabcmp[3]->AddColumn(&tab2->orderedCols[matchcols[clp * 2 + 1]]); //tab2 diff
   }

tabcmp[2]->AddColumn("DiffType", 8, 8, SQL_VARCHAR, 9);
tabcmp[3]->AddColumn("DiffType", 8, 8, SQL_VARCHAR, 9);

tab2rows = (unsigned char *)malloc(((tab2->rows >> 3) + 1) * sizeof(char));
for(clp = 0;clp < tab2->rows;clp += 8) //make a list of all tab2 rows. set them all to unread
   tab2rows[clp >> 3] = 255; //all 1's

for(loop = 0;loop < tab2->rows % 8;loop++)
   SetFlag(loop, &tab2rows[clp >> 3], 1);

setsize = tab2->rows;
rowindex = 0;
tab2->GroupBy(rowindex, setsize, tab2k, ind);

setsize = tab1->rows;
tab1->GroupBy(rowindex, setsize, tab1k, ind);

//According to the foreign key for tab2 in tab1, find the sister row(s)
for(loop = rowindex; loop <  tab1->rows; loop += setsize)
   {
   setsize = tab1->NextDiff(rowindex, setsize, tab1k, ind);
   sisrow = 0;
   sisset = tab2->rows;
   //Find the row(s) matching
   for(pkloop = 0;pkloop < ind;pkloop++)
      { //make sure each column of the key has a match in tab2
      sisrow = tab2->Find(tab1->GetData(loop, tab1k[pkloop].col), sisrow, tab2k[pkloop].col, sisset);
      if(sisrow == -1) //There is no sister row
         break;
      sisset = tab2->NextDiff(sisrow, sisset, tab2k, pkloop + 1);
      }
   if(sisrow != -1)
      {
      sisset = tab1->NextDiff(sisrow, sisset, tab2k, ind);
      for(iloop = sisrow;iloop < sisrow + sisset;iloop++) //Compare each matched row in tab2
         {
         for(clp = 0;clp < matchedcols;clp++) //with each matched column
            {
            if(NULLstrcmp(tab1->GetData(loop, matchcols[clp * 2]), tab2->GetData(iloop, matchcols[clp * 2 + 1])) != 0)
               {
               fprintf(out, "Column %d(%s) of row %d of %s differs from Column %d(%s) of row %d in %s.\n", matchcols[clp * 2], tab1->orderedCols[matchcols[clp * 2]].name, loop, tab1->name, matchcols[clp * 2 + 1], tab2->orderedCols[matchcols[clp * 2 + 1]].name, iloop, tab2->name);
               fprintf(out, "(%d)%s - (%d)%s.\n", strlenBytes(tab1->GetData(loop, matchcols[clp * 2])), tab1->GetData(loop, matchcols[clp * 2]), strlenBytes(tab2->GetData(iloop, matchcols[clp * 2 + 1])), tab2->GetData(iloop, matchcols[clp * 2 + 1]));
               fprintf(out, "Key Used:\n");
               for(pkloop = 0;pkloop < ind;pkloop++)
                  fprintf(out, "\tColumn %s = %s\n", tab1k[pkloop].name, tab1->GetData(loop, tab1k[pkloop].col));
               fprintf(out, "\n");
               tf = 1;
               }
            }
         if(tf) //it broke out before it finished comparing
            {
            ret = tabcmp[2]->NewRow(); //Set the row appropriately
            for(clp = 0;clp < matchedcols;clp++)
               tabcmp[2]->SetData(ret, clp, tab1->GetData(loop, matchcols[clp * 2]), strlenBytes(tab1->GetData(loop,matchcols[clp * 2])));
            tabcmp[2]->SetData(ret, matchedcols, "ColDiff", 9);
            ret = tabcmp[3]->NewRow();
            for(clp = 0;clp < matchedcols;clp++)
               {
               tabcmp[3]->SetData(ret, clp, tab2->GetData(iloop, matchcols[clp * 2 + 1]), strlenBytes(tab2->GetData(iloop, matchcols[clp * 2 + 1])));
               }
            tabcmp[3]->SetData(ret, matchedcols, "ColDiff", 9);
            SetFlag(iloop, tab2rows, 0);
            }
         else
            {
            ret = tabcmp[0]->NewRow();
            for(clp = 0;clp < matchedcols;clp++)
               tabcmp[0]->SetData(ret, clp, tab1->GetData(loop,matchcols[clp * 2]), strlenBytes(tab1->GetData(loop,matchcols[clp * 2])));
            ret = tabcmp[1]->NewRow();
            for(clp = 0;clp < matchedcols;clp++)
               {
               tabcmp[1]->SetData(ret, clp, tab2->GetData(iloop, matchcols[clp * 2 + 1]), strlenBytes(tab2->GetData(iloop, matchcols[clp * 2 + 1])));
               }
            SetFlag(iloop, tab2rows, 0);
            }
         tf = 0;
         }
      }
   else
      { //This row has no match and therefore is different
      fprintf(out, "Row %d of %s has no match in %s.\n", loop, tab1->name, tab2->name);
      fprintf(out, "Key Used:\n");
      for(pkloop = 0;pkloop < ind;pkloop++)
         fprintf(out, "\tColumn %s = %s\n", tab1k[pkloop].name, tab1->GetData(loop, tab1k[pkloop].col));
      fprintf(out, "\n");
      ret = tabcmp[2]->NewRow();
      for(clp = 0;clp < matchedcols;clp++)
         tabcmp[2]->SetData(ret, clp, tab1->GetData(loop,matchcols[clp * 2]), strlenBytes(tab1->GetData(loop,matchcols[clp * 2])));
      tabcmp[2]->SetData(ret, matchedcols, "No Match", 9);
      }
   }
//Put all the rows unconsidered in Table2 into tab2 differences table
for(loop = 0;loop < tab2->rows;loop++)
   {
   if(GetFlag(loop, tab2rows))
      {
      ret = tabcmp[3]->NewRow();
      for(clp = 0;clp < matchedcols;clp++)
         tabcmp[3]->SetData(ret, clp, tab2->GetData(loop, matchcols[clp * 2 + 1]), strlenBytes(tab2->GetData(loop, matchcols[clp * 2 + 1])));
      fprintf(out, "Row %d of %s has no match in %s.\n", loop, tab2->name, tab1->name);
      fprintf(out, "Key Used:\n");
      for(pkloop = 0;pkloop < ind;pkloop++)
         fprintf(out, "\tColumn %s = %s\n", tab2k[pkloop].name, tab2->GetData(loop, tab2k[pkloop].col));
      fprintf(out, "\n");
      tabcmp[3]->SetData(ret, matchedcols, "No Match", 9);
      }
   }
}

void database::SetWhere(int handle, int whereindex, int col, const char *val, int oper, int logic, int eval)
{
wcs[whereindex].AddTo(col, val, oper, logic, eval);
wcs[whereindex].TableIndex = FindIndex(handle);
}

int database::AddWhere()
{
WhereClause wp;

wcs = (WhereClause *)realloc(wcs, sizeof(WhereClause) * (whereclauses + 1));
memcpy(&wcs[whereclauses], &wp, sizeof(WhereClause));
whereclauses++;
return whereclauses - 1;
}

void database::DeleteWhere(int index)
{
if(index < 0 || index > whereclauses)
   return;

delete &wcs[index];
memmove(&wcs[index], &wcs[index + 1], sizeof(WhereClause) * (index + 1 - whereclauses));
}

int database::RunSelect(int index)
{
return db[wcs[index].TableIndex]->RunSelect(&wcs[index], lowrow, highrow);
}

void database::SelectRowSubSet(int r1, int r2)
{
lowrow = r1;
highrow = r2;
}

Table *database::ListTables()
{
static int loop = 0;

if(loop < Tables)
   {
   loop++;
   return db[loop - 1];
   }
else
   {
   loop = 0;
   return NULL;
   }
}
