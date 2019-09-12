#include "sstr.h"
#include "table.h"

/*
Table Code
*/
Table::Table()
{
cols = NULL;
columns = 0;
rowdata = NULL;
rows = 0;
rowsize = 0;
lastcol = -1;
name = NULL;
sortedCols = NULL;
clct = 0;
fkc = 0;
orderedCols = NULL;
For = NULL;
flags = 0;
}

Table::~Table()
{
int loop;

FreeCharPP(cols, columns);

for(loop = 0;loop < rows;loop++)
   FreeCharPP(rowdata[loop]->data, columns);

for(loop = 0;loop < rows;loop++)
   delete rowdata[loop];

free(sortedCols);
free(rowdata);
free(orderedCols);
}

void Table::AddColumn(const char *name, int size, int prec, int sqltype, int namelength)
{
//allocate the new memory needed and copy over the old info
int loop;

cols = (char **)realloc(cols, sizeof(char *) * (columns + 1));
cols[columns] = (char *)malloc(sizeof(char)*namelength);

memcpy(cols[columns], name, namelength);

sortedCols = (Columns *)realloc(sortedCols, sizeof(Columns) * (columns + 1));
sortedCols[columns].name = cols[columns]; //Not sorted now, but soon
sortedCols[columns].col = columns;
sortedCols[columns].colsize = size;
sortedCols[columns].type = sqltype;
sortedCols[columns].precision = prec;

orderedCols = (Columns *)realloc(orderedCols, sizeof(Columns) * (columns + 1));
memcpy(orderedCols, sortedCols, sizeof(Columns) * (columns + 1));

columns++;
rowsize += size; //length in bytes of the row
}

void Table::SortColumns()
{
qsort(sortedCols, columns, sizeof(Columns), cmpstrCols); //The index for the binary search
}

void Table::AddData(char *val, int size, int col)
{
int loop;

if(col < columns) //within table and column bounds
   {
   if(val == NULL)
      {
      rowdata[rows - 1]->data[col] = NULL; //the universal null string
      }
   else
      {
      rowdata[rows - 1]->data[col] = (char *)malloc(sizeof(char) * size + 1); //Allocate and copy in the string
      memcpy(rowdata[rows  - 1]->data[col], val, size);
      }
   SetFlag(TABLE_INVALID_DATA, &flags, 0);
   }
else
   SetFlag(TABLE_INVALID_DATA, &flags, 1);

}

int Table::NewRow()
{
SetFlag(TABLE_MODIFIED, &flags, 1);
rowdata = (Row **)realloc(rowdata, (rows + 1) * sizeof(Row **));
rowdata[rows] = new Row(columns);

rows++;
return rows - 1;
}

int Table::NewRows(int newrows)
{
int loop;
SetFlag(TABLE_MODIFIED, &flags, 1);
if(newrows == 0)
   return rows;

rowdata = (Row **)realloc(rowdata, (rows + newrows) * sizeof(Row *));
for(loop = 0;loop < newrows;loop++)
   rowdata[rows+loop] = new Row(columns);

rows += newrows;
return rows - newrows; //from the beginning of the new bunch
}

void Table::SetData(int row, int col,const char *val, int size)
{
int index, loop;

if(row < rows && col < columns && row > -1 && col > -1)
   {
   if(val != NULL)
      {
      free (rowdata[row]->data[col]);
      rowdata[row]->data[col] = (char *)malloc(size); //Allocate and copy
      memcpy(rowdata[row]->data[col], val, size);
      }
   else
      {
      free (rowdata[row]->data[col]);
      rowdata[row]->data[col] = NULL;
      }
   SetFlag(TABLE_INVALID_DATA, &flags, 0);
   }
else
   SetFlag(TABLE_INVALID_DATA, &flags, 1);
}

int Table::Find(char *val, int row, int col, int setsize)
{
int loop = 0;
void *bsval;
Row *retval;
int tp;

tp = Stat(col, SQLTYPE);

clct = 0;

if(col != lastcol) //Don't want to keep sorting it over and over
   {
   Sort(row, col, setsize); //Sort it up
   lastcol = col;
   }

sortcol = col; //binary search needs this info uptodate as well
if(tp == SQL_CHAR || tp == SQL_VARCHAR || tp == SQL_TYPE_DATE || tp == SQL_TYPE_TIME || tp == SQL_TYPE_TIMESTAMP)
   bsval = bsearch(val, &rowdata[row], setsize, sizeof(Row *), cmpstrBs);

if(tp == SQL_INTEGER || tp == SQL_SMALLINT)
   bsval = bsearch(val, &rowdata[row], setsize, sizeof(Row *), cmpsitBs);

if(bsval == NULL) //If nothing, then its not there
   return -1;

//Otherwise find the index

retval = *((Row **)bsval); //Cast it into the correct form

for(loop = 0;loop < rows;loop++)
   {
   if(retval == rowdata[loop]) //If it has the same address
      { //Now go back and find the first instance of it(if it has multiple occurences)
      if(loop > 0)
         {
         while(NULLstrcmp(rowdata[loop - 1]->data[col], val) == 0 && loop > row) //the whole point of the row/setsize pair is to limit its abilities to go hunting thru stuff
            {
            loop--;
            clct++;
            if(loop == 0) //Can't go back any farther or past the start row
               return loop;
            }
         }
      return loop;
      }
   }
return -1; //That was...odd...
}

void Table::Sort(int row, int col, int setsize)
{
sortcol = col;
int tp;
tp = Stat(col, SQLTYPE);

if(tp == SQL_CHAR || tp == SQL_VARCHAR || tp == SQL_TYPE_DATE || tp == SQL_TYPE_TIME || tp == SQL_TYPE_TIMESTAMP)
   qsort(&rowdata[row], setsize, sizeof(Row *), cmpstrSort);
if(tp == SQL_INTEGER || tp == SQL_SMALLINT)
   qsort(&rowdata[row], setsize, sizeof(Row *), cmpsitSort);
}

void Table::PrintStats(FILE *out)
{
int loop, iloop;

fprintf(out, "Table %s\n", name);
fprintf(out, "%d Rows\n", rows);
fprintf(out, "%d Columns\n", columns);
fprintf(out, "%d RowSize\n", rowsize);

for(loop = 0;loop < columns;loop++)
   {
   switch (Stat(loop, SQLTYPE))
      {
      case SQL_TYPE_DATE:
         fprintf(out, "%d %s DATE\n",loop, cols[loop]);
         break;
      case SQL_TYPE_TIME:
         fprintf(out, "%d %s TIME\n",loop, cols[loop]);
         break;
      case SQL_TYPE_TIMESTAMP:
         fprintf(out, "%d %s TIMESTAMP\n",loop, cols[loop]);
         break;
      case SQL_INTEGER:
         fprintf(out, "%d %s INTEGER\n",loop, cols[loop]);
         break;
      case SQL_SMALLINT:
         fprintf(out, "%d %s SMALLINT\n",loop, cols[loop]);
         break;
      case SQL_CHAR:
         fprintf(out, "%d %s CHAR(%d)\n",loop, cols[loop], Stat(loop, SQLPREC));
         break;
      case SQL_VARCHAR:
         fprintf(out, "%d %s VARCHAR(%d)\n",loop, cols[loop], Stat(loop, SQLPREC));
         break;
      default:
         fprintf(out, "%d %s Oops, didn't cover this data type\n",loop, cols[loop]);
      }
   }
if(fkc > 0)
   {
   fprintf(out, "%d Foreign Keys.\n", fkc);
   for(loop = 0;loop < fkc;loop++)
      {
      fprintf(out, "Key %d for %s.\n", loop, ((Table *)For[loop]->Foreign)->name);
      for(iloop = 0;iloop < For[loop]->nck;iloop++)
         fprintf(out, "\tColumn %d %s\n", iloop, For[loop]->key[iloop].name);
      }
   }
if(PrimKey.nck != 0) //It has a primary key
   {
   fprintf(out, "\nPrimary Key:\n");
   for(iloop = 0;iloop < PrimKey.nck;iloop++)
      fprintf(out, "\tColumn %d %s\n", iloop, PrimKey.key[iloop].name);
   }
fprintf(out, "\n");
}

int Table::BytesUsed()
{
int loop, iloop, count = 0;

for(loop = 0;loop < columns;loop++)
   {
   count += _msize(cols[loop]); //every column name
   for(iloop = 0;iloop < rows;iloop++)
      {
      count += _msize(rowdata[iloop]->data[loop]); //every entry in the table
      }
   }
count += _msize(orderedCols);
count += _msize(sortedCols);
count += _msize(rowdata);
count += sizeof(Row) * rows;
count += _msize(cols); //Column names
count += _msize(name); //the name
count += _msize(For);
for(loop = 0;loop < fkc;loop++)
   count += For[loop]->BytesUsed();
return count;
}

int Table::Mallocs()
{
//A direct copy where every _msize is replaced by a + 1 to the count
int loop, iloop, count = 0;

for(loop = 0;loop < columns;loop++)
   {
   count++;
   for(iloop = 0;iloop < rows;iloop++)
      {
      if(rowdata[iloop]->data[loop] != NULL) //obviously no mem allocated if its null
         count++;
      }
   }
count ++;
count ++;
count ++;
count += rows;
count ++;
count ++;
count ++;
for(loop = 0;loop < fkc;loop++)
   count += For[loop]->Mallocs();
return count;
}

int Table::ColIndex(const char *name)
{
void *bsval;
Columns *retval;

//Theres an array of Columns structures that contain the name and and index of the column in question
//the index(col) value is the actual array index
bsval = bsearch(name, sortedCols, columns, sizeof(Columns), cmpstrColsBs);

if(bsval != NULL)
   {
   retval = (Columns *)bsval;
   return retval->col;
   }
else
   return -1;
}

int Table::Count(char *val, int row, int col, int setsize)
{
int loop, cnt, rowindex = -1;

rowindex = Find(val, row, col, setsize);

if(rowindex == -1) //If there be not a single row with that value, forget it
   {
   clct = -1; //for Evaluate's use
   return 0;
   }
else
   cnt = clct + 1; //thats how many Find looked over after it found the first member of the row subset, plus the first actual member which wouldn't have been counted had the Find function luckily landed on the first one

loop = rowindex + clct + 1; //No need to check that which is checked already.  Look to the row after the one found initially in the find function
if(loop >= rows)
   return cnt; //Don't want an exception now do we?
while(NULLstrcmp(rowdata[loop]->data[col], val) == 0) //Since its just been sorted we can go down the rows until the value changes
   {
   cnt++;
   loop++;
   if(loop == rows) //Catch it at the end
      {
      clct = loop - cnt;
      return cnt;
      }
   }
clct = loop - cnt; //save the last row for internal processing
return cnt;
}

void Table::DropRow(int row)
{
int loop;

//Clear up the memory that the row was occupying
FreeCharPP(rowdata[row]->data, columns);
delete rowdata[row];

for(loop = row + 1;loop < rows;loop++)
   rowdata[loop - 1] = rowdata[loop]; //Adjust the array on the right one over

SetFlag(TABLE_MODIFIED, &flags, 1); //The table's rows have been changed
rows--;
}

char *Table::GetData(int row, int col)
{
if(row == -1 || col == -1 || row > rows || col > columns)
   return NULL;
else
   return rowdata[row]->data[col];
}

int Table::Stat(int col, int type)
{
if(col > columns)
   return -1;

switch(type)
   {
   case SQLTYPE:
      return orderedCols[col].type;
      break;
   case SQLPREC:
      return orderedCols[col].precision;
      break;
   case SQLCOLSIZE:
      return orderedCols[col].colsize;
      break;
   default:
      return 0;
   }
}

int Table::AddForeignKey(Table *pk)
{
int loop;

ForeignKey *fk;

fk = new ForeignKey(&pk->PrimKey);
fk->Foreign = pk;

For = (ForeignKey **)malloc((fkc + 1) * sizeof(ForeignKey *));
For[fkc] = fk;

For[fkc]->mtchcol = (int *)realloc(For[fkc]->mtchcol, sizeof(int *) * pk->PrimKey.nck);
for(loop = 0;loop < pk->PrimKey.nck;loop++) //match up the column to the parent table
   For[fkc]->mtchcol[loop] = ColIndex(pk->PrimKey.key[loop].name);

fkc++;
return fkc;
}

int Table::ForeignKeyExists(Table *pk)
{
int loop;

for(loop = 0;loop < fkc;loop++)
   {
   if(For[loop]->Foreign == pk)
      return loop;
   }
return -1;
}

void Table::GroupBy(int rowindex, int setsize, PrimaryKey *pk)
{
Columns *key;
int keys;

key = pk->GetCol(&keys);
GroupBy(rowindex, setsize, key, keys);
}

void Table::GroupBy(int rowindex, int setsize, Columns *key, int keys)
{
static int depth = 0;
char *val;
int keyc, subset;


if(depth >= keys)
   return; //We've gone TOO far

subset = rowindex + setsize;
Sort(rowindex, key[depth].col, setsize);
while(rowindex < subset)
   {
   val = GetData(rowindex, key[depth].col); //Get the data from the first set of stuff in the Primary Key
   keyc = Count(val, rowindex, key[depth].col, setsize);
   depth++;
   GroupBy(rowindex, keyc, key, keys);
   depth--;
   rowindex += keyc;  //Go up to the next set of stuff and prepare to group that too
   setsize -= keyc;
   };
}

int Table::NextDiff(int row, int setsize, PrimaryKey *pk)
{
Columns *key;
int keys;

key = pk->GetCol(&keys);
return NextDiff(row, setsize, key, keys);
}

int Table::NextDiff(int row,int setsize, Columns *cs, int dep)
{
char **val;
int loop = 0, keyl;

val = (char **)malloc(sizeof(char *) * dep); //the original row value
for(keyl = 0;keyl < dep;keyl++)
   val[keyl] = GetData(row, cs[keyl].col);

loop = row + 1;
while(loop < row + setsize && loop < rows)
   {
   for(keyl = 0;keyl < dep;keyl++)
      {
      if(NULLstrcmp(val[keyl], GetData(loop, cs[keyl].col)) != 0)
         return loop - row; //the sum total of rows in the current set
      }
   loop++;
   }
return loop - row;
}

int Table::AddRow(Row *inp)
{
int row, loop;

if(inp == NULL)
   return -1;
if( (_msize(inp->data) / sizeof(char *)) > columns )  //column count check
   return -1;

row = NewRow();

for(loop = 0;loop < columns;loop++)
   SetData(row, loop, inp->data[loop], strlenBytes(inp->data[loop]));
return row;
}

void Table::SaveOrder()
{
int size;
SetFlag(TABLE_MODIFIED, &flags, 0); //it will be safe to load/unload a saved order
size = _msize(rowdata);
oldorder = (Row **)malloc(size);
memcpy(oldorder, rowdata, size);
}

int Table::LoadOrder()
{
int size;

if(!GetFlag(TABLE_MODIFIED, &flags)) //if the table hasn't been modified since its last save
   {
   size = _msize(oldorder);
   memcpy(rowdata, oldorder, size);
   }
else
   return -1;
return 0;
}

int Table::SwapRows(int r1, int r2)
{
Row *tp;

if(r1 < rows && r2 < rows)
   {
   tp = rowdata[r1];
   rowdata[r1] = rowdata[r2];
   rowdata[r2] = tp;
   return 1;
   }
return 0;
}

int Table::RunSelect(WhereClause *wc, int row, int size)
{
PrimaryKey pk;
int jump;
Columns *cs;
int keys, logical, evaloop, totalrows = 0, keyloop, iloop, loop;
unsigned char *fa;
RowSet *rs, *ors;

for(loop = 0;loop < wc->clauses;loop++)
   pk.AddToKey(&orderedCols[wc->wci[loop].col]);

cs = pk.GetCol(&keys);

rs = new RowSet(row, size, this);
rs->FlagAll();
ors = new RowSet(row, size, this);
ors->FlagAll();
//Evaluate all the evaluation sets.  Narrow down the row range.  do next evaluation set.
//Depending on operation type either do a intersection or a union of the two (or whatever other ops are supported)
//Take result set and do it with next evaluation set
//output row ranges of result set

for(loop = 0;loop < wc->esets - 1;loop++) //-1 just cuz
   {
   rs->Compress(); //To save extra processing on large tables.  Don't hurt small ones neither
   ors->Copy(rs);
   evaloop = wc->GetSet(loop); //Gets all whereclause info in this evaluation set
   logical = Evaluate(&wc->wci[evaloop], wc->evalsets[evaloop], rs); //Flags all the rows that are in it
   switch(wc->wci[evaloop + wc->evalsets[evaloop] - 1].logic) //find the last operator to change the set
      {
      case ROW_OP_AND:
         rs->And(ors);
         break;
      case ROW_OP_OR:
         rs->Or(ors);
         break;
      case ROW_NO_OP: //theoretically this only occurs at the end of statement processing
         rs->Compress();
         break;
      }
   }

for(loop = rs->rowindex;loop < rs->setsize + rs->rowindex;loop++)
   {
   if(rs->RowFlag(loop))
      {
      SwapRows(totalrows, loop);
      totalrows++;
      }
   }
return totalrows;
}

int Table::Evaluate(wcinfo *clause, int clausecount, RowSet *rs)
{
int loop, iloop, rowindex, setsize;
RowSet *tp;

tp = new RowSet(rs->rowindex, rs->setsize, this); //the temporary row set that fits
tp->UnFlagAll();

for(iloop = 0;iloop < clausecount;iloop++)
   {
   setsize = Count(clause[iloop].val, 0, clause[iloop].col, rows);
   rowindex = clct; //set in Count
   for(loop = rowindex;loop < rowindex + setsize;loop++)
      {
      switch(clause[iloop].oper)
         {
         case EQUALS:
            if(Compare(loop, clause[iloop].col, clause[iloop].val) == 0)
               {
               tp->FlagRow(loop);
               }
            break;
         case NOT_EQUALS:
            if(Compare(loop, clause[iloop].col, clause[iloop].val) != 0)
               {
               tp->FlagRow(loop);
               }
             break;
         case LESS_THAN:
            if(Compare(loop, clause[iloop].col, clause[iloop].val) < 0)
               {
               tp->FlagRow(loop);
               }
            break;
         case GREATER_THAN:
            if(Compare(loop, clause[iloop].col, clause[iloop].val) > 0)
               {
               tp->FlagRow(loop);
               }
            break;
         default:
            break;
         }
      //Now according to the logical operation compose this RowSet with the last rowset
      if(iloop > 0)
         {
         switch(clause[iloop].logic)
            {
            case ROW_OP_AND:
               rs->And(tp);
               break;
            case ROW_OP_OR:
               rs->Or(tp);
               break;
            }
         } //end logical
      } //row processing
   rs->Copy(tp);
   tp->UnFlagAll();
   } //clause processing
return clause[clausecount - 1].logic;
}

int Table::Compare(int row, int col, const char *val)
{
return NULLstrcmp(GetData(row, col), val);
}

void Table::PrintTable(FILE *out)
{
int loop, iloop, colm, indl, *maxsize;
maxsize = (int *)calloc(1, columns * sizeof(int));

for(loop = 0;loop < columns;loop++)
   {
   maxsize[loop] = strlen(cols[loop]) + 1;
   for(iloop = 0;iloop < rows;iloop++)
      maxsize[loop] = max(maxsize[loop], strlenBytes(rowdata[iloop]->data[loop]) - 1); //using stringsize for null values
   maxsize[loop]++; //to give the output some breathing room
   fprintf(out, "%-*.*s",maxsize[loop],maxsize[loop],cols[loop]);
   }
fprintf(out, "\n");
for(iloop = 0;iloop < rows;iloop++)
   {
   for(loop = 0;loop < columns;loop++)
      {
      for(indl = 0;indl < loop;indl++)
         colm += sortedCols[loop].colsize;
      fprintf(out, "%-*.*s",maxsize[loop],maxsize[loop],rowdata[iloop]->data[loop]);
      }
   fprintf(out, "\n");
   }
fprintf(out, "\n");
free(maxsize);
}
