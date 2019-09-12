#include "sstr.h"

Table::Table()
{
cols = NULL;
columns = 0;
rowdata = NULL;
rows = 0;
rowsize = 0;
colsize = NULL;
type = NULL;
precision = NULL;
lastcol = -1;
}

Table::~Table()
{
int loop;

FreeCharPP(cols, columns);

for(loop = 0;loop < rows;loop++)
   FreeCharPP(rowdata[loop]->data, columns);

for(loop = 0;loop < rows;loop++)
   delete rowdata[loop];
free(rowdata);
free(type);
free(colsize);
free(precision);
}

Row::Row(int cols)
{
int loop;

data = (char **)malloc(sizeof(char *) * cols);

for(loop =0; loop < cols;loop++)
   data[loop] = NULL;
}

Row::~Row()
{
}

/*
Table Code
*/
void Table::AddColumn(char *name, int size, int prec, int sqltype)
{
//allocate the new memory needed and copy over the old info
int loop;

cols = (char **)realloc(cols, sizeof(char *) * (columns + 1));
cols[columns] = (char *)malloc(sizeof(char)*size);

memcpy(cols[columns], name, size);

colsize = (int *)realloc(colsize, sizeof(int *) * (columns + 1));
colsize[columns] = size;

//if(sqltype == SQL_CHAR)   //yuck!  Bad.
//   sqltype = SQL_VARCHAR;

type = (int *)realloc(type, sizeof(int *) * (columns + 1));
type[columns] =  sqltype;

precision = (int *)realloc(precision, sizeof(int *) * (columns + 1));
precision[columns] =  prec;

columns++;

rowsize += rowsize + size; //length in bytes of the row
}

void Table::AddData(char *val, int size, int col)
{
int loop;

if(col < columns && size <= colsize[col]) //within table and column bounds
   {
   if(val == NULL)
      {
      rowdata[rows - 1]->data[col] = NULL; //the universal null string
      }
   else
      {
      rowdata[rows - 1]->data[col] = (char *)malloc(sizeof(char) * size + 1);
      memcpy(rowdata[rows  - 1]->data[col], val, size);
      }
   }
}

void Table::NewRow()
{

rowdata = (Row **)realloc(rowdata, (rows + 1) * sizeof(Row **));
rowdata[rows] = new Row(columns);

rows++;
}

void Table::NewRows(int newrows)
{
int loop;

rowdata = (Row **)realloc(rowdata, (rows + newrows) * sizeof(Row **));
for(loop = 0;loop < newrows;loop++)
   rowdata[rows+loop] = new Row(columns);

rows += newrows;
}

void Table::SetData(int row, int col, char *val, int size)
{
int index, loop;

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
}

int Table::Find(char *val, int col)
{
int loop = 0;
void *bsval;
Row *retval;

if(col != lastcol) //Don't want to keep sorting it over and over
   {
   Sort(col); //Sort it up
   lastcol = col;
   }

if(type[col] == SQL_CHAR || type[col] == SQL_VARCHAR || type[col] == SQL_TYPE_DATE || type[col] == SQL_TYPE_TIME || type[col] == SQL_TYPE_TIMESTAMP)
   bsval = bsearch(val, rowdata, rows, sizeof(Row *), cmpstrBs);

if(type[col] == SQL_INTEGER || type[col] == SQL_SMALLINT)
   bsval = bsearch(val, rowdata, rows, sizeof(Row *), cmpsitBs);

if(bsval == NULL) //If nothing, then its not there
   return -1;

//Otherwise find the index

retval = *((Row **)bsval); //Cast it into the correct form

for(loop = 0;loop < rows;loop++)
   {
   if(retval == rowdata[loop]) //If it has the same address
      return loop;
   }
return -1; //That was...odd...
}

void Table::Sort(int col)
{
sortcol = col;

if(type[col] == SQL_CHAR || type[col] == SQL_VARCHAR || type[col] == SQL_TYPE_DATE || type[col] == SQL_TYPE_TIME || type[col] == SQL_TYPE_TIMESTAMP)
   qsort((Row *)rowdata, rows, sizeof(Row *), cmpstrSort);
if(type[col] == SQL_INTEGER || type[col] == SQL_SMALLINT)
   qsort((Row *)rowdata, rows, sizeof(Row *), cmpsitSort);
}

void Table::PrintStats(FILE *out)
{
int loop, iloop;

fprintf(out, "%d Rows\n%d Columns\n%d RowSize\n", rows, columns, rowsize);

for(loop = 0;loop < columns;loop++)
   {
   switch (type[loop])
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
         fprintf(out, "%d %s CHAR(%d)\n",loop, cols[loop], precision[loop]);
         break;
      case SQL_VARCHAR:
         fprintf(out, "%d %s VARCHAR(%d)\n",loop, cols[loop], precision[loop]);
         break;
      default:
         fprintf(out, "%d %s Oops, didn't cover this data type\n",loop, cols[loop]);
      }
   }
fprintf(out, "\n");
}

int Table::BytesUsed()
{
int loop, iloop, count = 0;

for(loop = 0;loop < columns;loop++)
   {
   count += _msize(cols[loop]);
   for(iloop = 0;iloop < rows;iloop++)
      {
      count += _msize(rowdata[iloop]->data[loop]);
      }
   }
count += _msize(type);
count += _msize(precision);
}

int Table::Mallocs()
{
int loop, iloop, count = 0;

for(loop = 0;loop < columns;loop++)
   {
   count++;
   for(iloop = 0;iloop < rows;iloop++)
      {
      count ++;
      }
   }
count ++;
count ++;
}

/*
database code
*/
database::database()
{
Tables = 0;
db = NULL;
}

int database::NewTable()
{
Table *tab;
int ret;

tab = new Table;
ret = AddTable(tab);
return ret;
}

int database::AddTable(Table *tb)
{
db = (Table **)realloc(db, Tables + 1);
db[Tables] = tb;
Tables++;
return Tables - 1;
}

Table *database::tab(int index)
{
return db[index];
}

database::~database()
{
int loop;

for(loop = 0;loop < Tables;loop++)
   delete db[loop];

free(db);
}

void database::PrintTable(int index, FILE *out)
{
int loop, iloop, colm, indl;

for(loop = 0;loop < db[index]->columns;loop++)
   {
   fprintf(out, "%-*.*s",db[index]->colsize[loop],db[index]->colsize[loop],db[index]->cols[loop]);
   }
fprintf(out, "\n");
for(iloop = 0;iloop < db[index]->rows;iloop++)
   {
   for(loop = 0;loop < db[index]->columns;loop++)
      {
      for(indl = 0;indl < loop;indl++)
         colm += db[index]->colsize[indl];
      fprintf(out, "%-*.*s",db[index]->colsize[loop],db[index]->colsize[loop],db[index]->rowdata[iloop]->data[loop]);
      }
   fprintf(out, "\n");
   }
fprintf(out, "\n");
}

void database::AutoCopy(int src, int dst)
{
int loop, iloop, il2, ol1, ri1, ri2;
char *val;

ri1 = db[dst]->rows;
db[dst]->NewRows(db[src]->rows); //add a new row for each row in the source table
ri2 = db[dst]->rows;

for(loop = 0;loop < db[src]->columns;loop++)
   {
   for(iloop = 0;iloop < db[dst]->columns;iloop++)
      {  //check every column in the dst to see if it matches the src
      if((strcmp(db[dst]->cols[iloop], db[src]->cols[loop])) == 0 && (db[dst]->type[iloop] == db[src]->type[loop]) && (db[dst]->precision[iloop] == db[src]->precision[loop]))
         { //now copy every row from this column (iloop) to the newrow
         for(il2 = ri1;il2 < ri2;il2++)
            {
            val = db[src]->GetData(il2 - ri1, loop);
            db[dst]->SetData(il2, iloop, val, StringSize(val)); //Using StringSize to overcome a possible NULL value
            }
         }
      }
   }
}

void database::SortTable(int index, int col)
{
db[index]->Sort(col);
}

void database::PrintStats(FILE *out)
{
int loop;

for(loop = 0;loop < Tables;loop++)
   {
   fprintf(out, "Table %s (%d)\n", name, loop);
   db[loop]->PrintStats(out);
   fprintf(out, "Bytes Used %d\n", db[index]->BytesUsed());
   fprintf(out, "Memory Allocations %d\n", db[index]->Mallocs());
   }
}
