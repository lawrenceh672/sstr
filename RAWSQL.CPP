#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include "dbutils.h"          //CLI utilities
#include "sstr.h"             //for storing databases locally

#define MAXCOLS 255
#define DEBUG 1
#define LEAVEYEAR 0
#define PLATOON 1
#define CHANGE_DATE 2
#define TOUR_START 3
#define SELECT 4
#define HVCOUNT 5
#define WILDCARD -3

#define APPEND 23
#define NEW 85

extern SQLCHAR server[SQL_MAX_DSN_LENGTH + 1] ;
extern SQLCHAR uid[MAX_UID_LENGTH + 1] ;
extern SQLCHAR pwd[MAX_PWD_LENGTH + 1] ;

FILE *fp = NULL;

int RunQuery(SQLHANDLE hdbc, char *stmt, Table *tab, int mode);
char *HV(char *stmt, char **HV, char **replace, int vars); //Replaces host variables and returns the statement
void Out(Table *tab, SQLHANDLE hdbc, int inserts, int *incols, char **outcols, int count, const char *dbtab);
char *Format(char *inp, int sqltype);
int ReadInSQL(char *buf, const char *path);

main( int argc, char * argv[] )
{
//the local sqlca
struct sqlca sqlca;

// Delare CLI Variables
SQLHANDLE henv, hdbc, hstmt;
SQLRETURN rc;

char *Inputs[HVCOUNT] = {"1999", "2", "1999-05-10", "1999-05-06", "5"};
char *HostV[HVCOUNT] = {":LEAVEYEAR", ":PLATOON", ":CHANGE_DATE", ":TOUR_START", ":SELECT"};
char *rep[HVCOUNT], buffer[50];
char *ast = "*", *tp, *val, *select1;

char *runst;
char temp[8192];
int stlen, loop, outind, ldind, decst, rowindex, ret, ol1, leavemove, leaveact, rosterlist;
int regular, pending, changeday, callin, results, combined, vacancy, final, output;
int incols[14] = {14, 1, 2, 3, 11, 12, 13, 15, 16, 18, 19, 20, 21, 22};
char *outcols[14] = {"PLATOON", "NAME", "RANK", "STATION", "DECLINED", "SENIORITY", "REGNO", "CHANGEPLATOON", "CALLIN", "LEAVE1", "LEAVE2", "LEAVE3", "LEAVE4", "ONLEAVE"};

database *db;
db = new database;

if(DEBUG)
   {
   if( (fp = fopen("mr.txt", "w")) == NULL) //A handy way to record the pertinent info
      exit(-1);
   }
else
   fp = stdout;

INIT_UID_PWD;

// allocate an environment handle
fprintf(fp, "Allocating handle for enviroment...");
rc = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv ) ;
if ( rc != SQL_SUCCESS )
   return( terminate( henv, rc ) ) ;
fprintf(fp, "allocated.\n");

// allocate a connect handle, and connect
fprintf(fp, "allocating handle for connection....");
rc = DBconnect( henv, &hdbc ) ;
if ( rc != SQL_SUCCESS )
   return( terminate( henv, rc ) ) ;
fprintf(fp, "allocated.\n");

//Initialize variables
memset(temp, 0, 8192);
rep[LEAVEYEAR] = Inputs[LEAVEYEAR];

rep[PLATOON] = (char *)calloc(1, 4);
strcpy(rep[PLATOON], "'");
strcat(rep[PLATOON], Inputs[PLATOON]);
strcat(rep[PLATOON], "'");

rep[CHANGE_DATE] = (char *)calloc(1, 19);
strcpy(rep[CHANGE_DATE], "DATE('");
strcat(rep[CHANGE_DATE], Inputs[CHANGE_DATE]);
strcat(rep[CHANGE_DATE], "')");

rep[TOUR_START] = (char *)calloc(1, 19);
strcpy(rep[TOUR_START], "DATE('");
strcat(rep[TOUR_START], Inputs[TOUR_START]);
strcat(rep[TOUR_START], "')");

rep[SELECT] = (char *)calloc(1, 4);
strcat(rep[SELECT], Inputs[SELECT]);

//Show the Host variable values
fprintf(fp, "Changed Date: %s\n Tour Start Date: %s\n Leave Year: %s\n Platoon: %s\n", rep[CHANGE_DATE], rep[TOUR_START], rep[LEAVEYEAR], rep[PLATOON]);

//Step 4 - Check for no records in rosterlist
strcat(temp, "SELECT COUNT(*) FROM USERID.ROSTERLIST WHERE ROSTER_DATE = :CHANGE_DATE AND SEQNO = 1 AND PLATOON = :PLATOON");
runst = HV(temp, HostV, rep, HVCOUNT); //Make the statement

fprintf(fp, "Length of st4: %d.\n",strlen(runst));

decst = db->NewTable(); //Put it in
rc = RunQuery(hdbc, runst, db->tab(decst), NEW);

ret = atoi(db->tab(decst)->GetData(0,0)); //the answer is a single a single row, single column number
fprintf(fp, "Rows in rosterlist %d.\n", ret);
free(runst);

//Read in the original queries
//Regular
memset(temp, 0, 8192);
loop = ReadInSQL(temp, "d:\\ibmcppw\\working\\sstrdll\\regular.sql");
runst = HV(temp, HostV, rep, HVCOUNT); //Make the statement
fprintf(fp, "Regular length %d, %s\n", (stlen = strlen(runst)), runst);

regular = db->NewTable(); //Put it in
rc = RunQuery(hdbc, runst, db->tab(regular), NEW);
fprintf(fp, "Result of Query Regular:\n");
//db->PrintTable(regular, fp);
Out(db->tab(regular), hdbc, 250, NULL, NULL, -1, "FACCSDB.REGULAR");
free(runst);

//Read in the original queries
//Pending
memset(temp, 0, 8192);
loop = ReadInSQL(temp, "d:\\ibmcppw\\working\\sstrdll\\pending.sql");
runst = HV(temp, HostV, rep, HVCOUNT); //Make the statement
fprintf(fp, "Pending length %d, %s\n", (stlen = strlen(runst)), runst);

pending = db->NewTable(); //Put it in
rc = RunQuery(hdbc, runst, db->tab(pending), NEW);
fprintf(fp, "Result of Query Pending:\n");
//db->PrintTable(pending, fp);
Out(db->tab(pending), hdbc, 250, NULL, NULL, -1, "FACCSDB.PENDING");
free(runst);

//Read in the original queries
//Changeday
memset(temp, 0, 8192);
loop = ReadInSQL(temp, "d:\\ibmcppw\\working\\sstrdll\\changeday.sql");
runst = HV(temp, HostV, rep, HVCOUNT); //Make the statement
fprintf(fp, "Changeday length %d, %s\n", (stlen = strlen(runst)), runst);

changeday = db->NewTable(); //Put it in
rc = RunQuery(hdbc, runst, db->tab(changeday), NEW);
fprintf(fp, "Result of Query Changeday:\n");
//db->PrintTable(changeday, fp);
Out(db->tab(changeday), hdbc, 250, NULL, NULL, -1, "FACCSDB.CHANGEDAY");
free(runst);

//Read in the original queries
//Callin
memset(temp, 0, 8192);
loop = ReadInSQL(temp, "d:\\ibmcppw\\working\\sstrdll\\callin.sql");
runst = HV(temp, HostV, rep, HVCOUNT); //Make the statement
fprintf(fp, "Callin length %d, %s\n", (stlen = strlen(runst)), runst);

callin = db->NewTable(); //Put it in
rc = RunQuery(hdbc, runst, db->tab(callin), NEW);
fprintf(fp, "Result of Query Callin:\n");
//db->PrintTable(callin, fp);
Out(db->tab(callin), hdbc, 250, NULL, NULL, -1, "FACCSDB.CALLIN");
free(runst);

//Read in the original queries
//combined
memset(temp, 0, 8192);
loop = ReadInSQL(temp, "d:\\ibmcppw\\working\\sstrdll\\combined.sql");
runst = HV(temp, HostV, rep, HVCOUNT); //Make the statement
fprintf(fp, "combined length %d, %s\n", (stlen = strlen(runst)), runst);

combined = db->NewTable(); //Put it in
rc = RunQuery(hdbc, runst, db->tab(combined), NEW);
fprintf(fp, "Result of Query Combined:\n");
//db->PrintTable(combined, fp);
Out(db->tab(combined), hdbc, 250, NULL, NULL, -1, "FACCSDB.COMBINED");
free(runst);

//Read in the original queries
//results
memset(temp, 0, 8192);
loop = ReadInSQL(temp, "d:\\ibmcppw\\working\\sstrdll\\results.sql");
runst = HV(temp, HostV, rep, HVCOUNT); //Make the statement
fprintf(fp, "results length %d, %s\n", (stlen = strlen(runst)), runst);

results = db->NewTable(); //Put it in
rc = RunQuery(hdbc, runst, db->tab(results), NEW);
fprintf(fp, "Result of Query Results:\n");
//db->PrintTable(results, fp);
Out(db->tab(results), hdbc, 250, NULL, NULL, -1, "FACCSDB.RESULTS");
free(runst);

//Read in the original queries
//vacancy
memset(temp, 0, 8192);
loop = ReadInSQL(temp, "d:\\ibmcppw\\working\\sstrdll\\vacancy.sql");
runst = HV(temp, HostV, rep, HVCOUNT); //Make the statement
fprintf(fp, "vacancy length %d, %s\n", (stlen = strlen(runst)), runst);

vacancy = db->NewTable(); //Put it in
rc = RunQuery(hdbc, runst, db->tab(vacancy), NEW);
fprintf(fp, "Result of Query Vacancy:\n");
db->PrintTable(vacancy, fp);
Out(db->tab(vacancy), hdbc, 250, NULL, NULL, -1, "FACCSDB.VACANCY");
free(runst);

//Read in the original queries
//Final
memset(temp, 0, 8192);
loop = ReadInSQL(temp, "d:\\ibmcppw\\working\\sstrdll\\final.sql");
runst = HV(temp, HostV, rep, HVCOUNT); //Make the statement
fprintf(fp, "Final length %d, %s\n", (stlen = strlen(runst)), runst);

final = db->NewTable(); //Put it in
rc = RunQuery(hdbc, runst, db->tab(final), NEW);
fprintf(fp, "Result of Query Final:\n");
db->PrintTable(final, fp);
Out(db->tab(final), hdbc, 250, NULL, NULL, -1, "FACCSDB.FINAL");
free(runst);

//The output sql to tie it all together
memset(temp, 0, 8192);
loop = ReadInSQL(temp, "d:\\ibmcppw\\working\\sstrdll\\output.sql");
runst = HV(temp, HostV, rep, HVCOUNT); //Make the statement
fprintf(fp, "Output length %d, %s\n", (stlen = strlen(runst)), runst);

output = db->NewTable(); //Put it in
rc = RunQuery(hdbc, runst, db->tab(output), NEW);
fprintf(fp, "The result of results:\n");
db->PrintTable(output, fp);
free(runst);

goto ext; //no need for everything else
/*-----------------------------------------------------------------*
 * Return to caller                                                *
 *-----------------------------------------------------------------*/

ext:
   rc = SQLEndTran( SQL_HANDLE_DBC, hdbc, SQL_COMMIT ) ;
   CHECK_HANDLE( SQL_HANDLE_DBC, hdbc, rc ) ;

   fprintf(fp, "Disconnecting .....\n" ) ;
   rc = SQLDisconnect( hdbc ) ;
   CHECK_HANDLE( SQL_HANDLE_DBC, hdbc, rc ) ;

   rc = SQLFreeHandle( SQL_HANDLE_DBC, hdbc ) ;
   CHECK_HANDLE( SQL_HANDLE_DBC, hdbc, rc ) ;

   rc = SQLFreeHandle( SQL_HANDLE_ENV,  henv ) ;
   if ( rc != SQL_SUCCESS )
      return( terminate( henv, rc ) ) ;

   _fcloseall();
   return 1;
}

int RunQuery(SQLHANDLE hdbc, char *stmt, Table *tab, int mode)
{
SQLRETURN rc, freeh;
SQLHANDLE hstmt;
SQLSMALLINT resultcols;
SQLCHAR     name[32];
SQLSMALLINT namelength;
SQLSMALLINT type;
SQLUINTEGER length[MAXCOLS], precision;
SQLSMALLINT scale;
SQLINTEGER  displaysize;
SQLCHAR     *data[MAXCOLS];
SQLINTEGER  outlen[MAXCOLS];

int loop;

rc = SQLAllocHandle( SQL_HANDLE_STMT, hdbc, &hstmt ) ;
CHECK_HANDLE( SQL_HANDLE_DBC, hdbc, rc ) ;
if( rc != SQL_SUCCESS )
   {
   freeh = SQLFreeHandle( SQL_HANDLE_STMT, hstmt ) ;
   CHECK_HANDLE( SQL_HANDLE_STMT, hstmt, freeh ) ;
   if( freeh != SQL_SUCCESS )
      return freeh;
   return rc;
   }

rc = SQLExecDirect( hstmt, (SQLCHAR *)stmt, SQL_NTS ) ;
CHECK_HANDLE( SQL_HANDLE_STMT, hstmt, rc ) ;
if( rc != SQL_SUCCESS )
   {
   freeh = SQLFreeHandle( SQL_HANDLE_STMT, hstmt ) ;
   CHECK_HANDLE( SQL_HANDLE_STMT, hstmt, freeh ) ;
   if( freeh != SQL_SUCCESS )
      return freeh;
   return rc;
   }

if(tab == NULL) //just run the query and skeddle
   {
   rc = SQLFreeHandle( SQL_HANDLE_STMT, hstmt ) ;
   CHECK_HANDLE( SQL_HANDLE_STMT, hstmt, rc ) ;
   if( rc != SQL_SUCCESS )
      return rc;
   return rc;
   }


rc = SQLNumResultCols( hstmt, &resultcols ) ;
CHECK_HANDLE( SQL_HANDLE_STMT, hstmt, rc ) ;
if( rc != SQL_SUCCESS )
   {
   freeh = SQLFreeHandle( SQL_HANDLE_STMT, hstmt ) ;
   CHECK_HANDLE( SQL_HANDLE_STMT, hstmt, freeh ) ;
   if( freeh != SQL_SUCCESS )
      return freeh;
   return rc;
   }


for (loop = 0;loop < resultcols;loop++)
   {
   //Make CLI get the column info for me
   SQLDescribeCol(hstmt,(SQLSMALLINT)(loop + 1),name,sizeof(name),&namelength,&type,&length[loop],&scale,NULL) ;

   //get the display length for column
   SQLColAttribute(hstmt,(SQLSMALLINT)(loop + 1),SQL_DESC_DISPLAY_SIZE,NULL,0,NULL,&displaysize);
   precision = length[loop]; //so the length of the SQL data type is saved as well as the column width for the output
   length[loop] = max(displaysize,strlen((char *)name)) + 1; //One extra for the null terminator

   // allocate memory to bind column */
   data[loop] = (SQLCHAR *)malloc((int)length[loop]);

   //bind columns to program vars, converting all types to CHAR
   SQLBindCol( hstmt,(SQLSMALLINT)(loop + 1),SQL_C_CHAR,data[loop],length[loop],&outlen[loop]);

   if(mode != APPEND) //No need to make columns otherwise
      tab->AddColumn(((char *)(&name[0])), length[loop], precision, type);
   }

//Fetch the rows and put them into the table
rc = SQLFetch( hstmt ) ;
while ( (rc == SQL_SUCCESS) || (rc == SQL_SUCCESS_WITH_INFO) )
   {
   tab->NewRow(); //start the new row
   for (loop = 0;loop < resultcols;loop++)
      {
      if(outlen[loop] == SQL_NULL_DATA)
         tab->AddData(NULL, 0, loop); //Null, 5 chars(plus null term), column loop
      else
         tab->AddData(((char *)(data[loop])), length[loop], loop);
      }
   rc = SQLFetch( hstmt );
   } //end while

rc = SQLFreeHandle( SQL_HANDLE_STMT, hstmt ) ;
CHECK_HANDLE( SQL_HANDLE_STMT, hstmt, rc ) ;
if( rc != SQL_SUCCESS ) return rc;

for( loop = 0; loop < resultcols; loop++ ) //Release the bound variables
   free( data[loop] );

return rc; //Must've been alright (SQL_SUCCESS)
}

char *HV(char *stmt, char **HV, char **replace, int vars)
{
char *loc, temp[8192], *cur, iloop, *ret;
int loop, length;

loc = stmt;
cur = stmt;
memset(temp, 0, 8192);
length = strlen(temp);
ret = (char *)malloc(sizeof(SQLCHAR) * (length + 1)); //copy the initial string over
memcpy(ret, temp, length + 1);

for(loop = 0;loop < vars; loop++)
   {
   while((loc = strstr(loc, HV[loop])) != NULL) //Get em all
      {
      strncat(temp, cur, loc - cur); //copy over the amount of string passed over
      strcat(temp, replace[loop]); //tack on the replacement thing
      loc = loc + strlen(HV[loop]); //start looking again after the found substring
      cur = loc;
      }
   //copy over the remaining string to the buffer
   strcat(temp, cur);
   free(ret); //Get ready for the next replacement
   ret = (char *)malloc(sizeof(SQLCHAR) * strlen(temp) + 1); //for the null term
   strcpy(ret, temp);
   memset(temp, 0, 8192);
   loc = ret;
   cur = ret;
   }
return ret;
}

void Out(Table *tab, SQLHANDLE hdbc, int inserts, int *incols, char **outcols, int count, const char *dbtab)
{
char *stmt, *initial;
char *form, *tp, buffer[33];
int loop, iloop, size, stlen, sl = 0, step = 0, dbtabsz = 0, intsize;

dbtabsz = strlen(dbtab);

if(incols != NULL && outcols != NULL) //Probably don't need the case where the columns to receive the
   {                                  //insert need to be specified while a wildcard select on the
   initial = (char *)calloc(1, 500);  //input columns is needed
   strcat(initial, "INSERT INTO ");
   strcat(initial, dbtab);
   strcat(initial, "(");
   for(loop = 0;loop < count;loop++)
      {
      strcat(initial, outcols[loop]);
      strcat(initial, ",");
      }
   strcpy(&(initial[strlen(initial) - 1]), ") VALUES(");
   intsize = strlen(initial);
   size = intsize;
   stmt = (char *)malloc(intsize);
   strcpy(stmt, initial);
   }
else
   {
   initial = (char *)calloc(1, 21 + dbtabsz);
   strcat(initial, "INSERT INTO ");
   strcat(initial, dbtab);
   strcat(initial, " VALUES(");
   stmt = (char *)calloc(1, 21 + dbtabsz);
   strcpy(stmt, initial);
   intsize = size = strlen(initial); //the initial size
   incols = (int *)malloc(sizeof(int) * tab->columns);
   for(loop = 0;loop < tab->columns;loop++) //otherwise just select all columns for input
      incols[loop] = loop;
   count = tab->columns;
   }

while(sl < tab->rows) //Can't let the insert get too big
   {
   if(tab->rows - sl > inserts)
      step = inserts;
   else
      step = tab->rows - sl;
   for(loop = sl;loop < sl + step;loop++)
      {
      for(iloop = 0;iloop < count;iloop++)
         {
         form = Format(tab->rowdata[loop]->data[incols[iloop]], tab->type[incols[iloop]]);
         stlen = strlen(form);
         stmt = (char *)realloc(stmt, size + stlen + 4); //make room for the null and the ),(
         memcpy(&(stmt[size]), form, stlen + 1);
         size += stlen;
         }
      stlen = strlen(stmt);
      strcpy(&(stmt[stlen - 1]), "),("); //place it on top of the comma
      size = stlen + 2; //the new string size
      }
   sl = sl + step;
   stlen = strlen(stmt);
   stmt[stlen - 2] = 0; //slip in the 0 the null out the string just after the , in "),("
   RunQuery(hdbc, stmt, NULL, -1);  //NULL to indicate no table will hold the output and -1 mode cuz it has no meaning
   fprintf(fp, "%s\n", stmt);
   free(stmt);
   stmt = (char *)malloc(intsize);
   strcpy(stmt, initial);
   };
}

char *Format(char *inp, int sqltype)
{
int stlen, loc;
static char *date = "DATE('XX/XX/XXXX'),", *tp;
static char character[512];

character[0] = 0;

if(inp == NULL)
   {
   strcpy(character, "NULL,");
   return character;
   }

stlen = strlen(inp);

switch (sqltype)
   {
   case SQL_CHAR:
   case SQL_VARCHAR:
      character[0] = 39;
      if((tp = strchr(inp, 39)) == NULL)
         memcpy(&(character[1]), inp, stlen);
      else //need a '' to make a ' in the values statement
         {
         loc = tp - inp;
         memcpy(&(character[1]), inp, loc);
         character[loc + 1] = 39;
         character[loc + 2] = 39;
         memcpy(&(character[loc + 3]), &(inp[loc + 1]), stlen - loc);
         }
      character[stlen + 1] = 39; //the ' character
      character[stlen + 2] = ',';
      character[stlen + 3] = 0;
      return character;
      break;
   case SQL_TYPE_DATE:
      memcpy(&(date[6]), inp, stlen);
      return date;
      break;
   default:
      memcpy(character, inp, stlen);
      character[stlen] = ',';
      character[stlen + 1] = 0;
      return character;
      break;
   }
return NULL;
}

int ReadInSQL(char *buf, const char *path)
{
FILE *inp;
int loop = 0;

if( (inp = fopen(path, "r")) == NULL)
   return -1;

while(!feof(inp))
   {
   buf[loop] = (char)fgetc(inp);
   loop++;
   }
buf[loop - 1] = 0; //Take off the garbage characater at the end. 
return 1;
}
