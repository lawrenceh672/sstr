#include "lhdb.h"
#include <time.h>

char *lastq = NULL;

char *Format(char *inp, int sqltype, int *size)
{
int stlen, loc;
static char date[20], timestamp[42], character[512];
static unsigned char ft = 1;
char *tp;

if(ft)
   {
   strcpy(date, "DATE('xxxx-xx-xx'),");
   strcpy(timestamp, "TIMESTAMP('yyyy-mm-dd hh:mm:ss.ssssss'),");
   }
else
   ft = 0;

character[0] = 0;

if(inp == NULL)
   {
   strcpy(character, "NULL,");
   *size = 5; //Duh
   return character;
   }

stlen = strlen(inp);
*size = stlen;

switch (sqltype)
   {
   case SQL_CHAR:
   case SQL_VARCHAR:
      character[0] = 39;
      if((tp = strchr(inp, 39)) == NULL)
         {
         memcpy(&(character[1]), inp, stlen);
         }
      else //need a '' to make a ' in the values statement
         {
         loc = tp - inp;
         memcpy(&(character[1]), inp, loc);
         character[loc + 1] = 39;
         character[loc + 2] = 39;
         memcpy(&(character[loc + 3]), &(inp[loc + 1]), stlen - loc);
         *size = *size + 1; //the extra '
         stlen++;
         }
      character[stlen + 1] = 39; //the ' character
      character[stlen + 2] = ',';
      character[stlen + 3] = 0;
      *size = *size + 3; // 'XX-XX', 3 extra chars
      return character;
      break;
   case SQL_TYPE_DATE:
      memcpy(&(date[6]), inp, stlen);
      *size = 19; //Fixed size for a date
      return date;
      break;
   case SQL_TYPE_TIMESTAMP:
      memcpy(&(timestamp[11]), inp, stlen);
      *size = 40;
      return timestamp;
   default:
      memcpy(character, inp, stlen);
      character[stlen] = ',';
      character[stlen + 1] = 0;
      *size = *size + 1; //just the ,
      return character;
      break;
   }
return NULL;
}

int RunQuery(SQLHANDLE hdbc, const char *stmt, Table *tab, int mode, FILE *fp)
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
static counter = 0;
static FILE *fps = NULL;
int loop, qlen;

qlen = strlenBytes(stmt);

if(fps == NULL)
   {
   fps = fp;
   fprintf(fps, "Query %d(%d): %s\n", counter, qlen, stmt);
   counter++;
   }
else
   {
   fprintf(fps, "Query %d(%d): %s\n", counter, qlen, stmt);
   counter++;
   }

free(lastq);
lastq = (char *)malloc(qlen);
strcpy(lastq, stmt);

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
if( rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
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
      tab->AddColumn(((const char *)(&name[0])), length[loop], precision, type, namelength + 1);
   }

tab->SortColumns(); //Otherwise there'll be hell to pay

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
         tab->AddData(((char *)(data[loop])), strlen(((char *)data[loop])) + 1, loop);
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
         form = Format(tab->rowdata[loop]->data[incols[iloop]], tab->Stat(iloop, SQLTYPE), &stlen);
         stmt = (char *)realloc(stmt, size + stlen + 4); //make room for the null and the ),(
         memcpy(&(stmt[size]), form, stlen + 1);
         size += stlen;
         }
      strcpy(&(stmt[size - 1]), "),("); //place it on top of the comma
      size = size + 2; //the new string size
      }
   sl = sl + step;
   stlen = size;
   stmt[stlen - 2] = 0; //slip in the 0 the null out the string just after the , in "),("
   RunQuery(hdbc, stmt, NULL, -1, NULL);  //NULL to indicate no table will hold the output and -1 mode cuz it has no meaning
   free(stmt);
   stmt = (char *)malloc(intsize);
   strcpy(stmt, initial);
   size = intsize; //start up again
   };
}


int CreateTable(SQLHANDLE hdbc, Table *tab, const char *name, const char *tabspace)
{
int loop, len, strlength = 0, tsl = 0, intsize;
char buf[128], *out = NULL, *initial = "CREATE TABLE ";

out = (char *)malloc((13 + strlen(name) + 1)* sizeof(char));
strcpy(out, initial);
strcat(out, name);
strcat(out, "(");
intsize = strlen(out);
strlength = intsize;

for(loop = 0;loop < tab->columns;loop++)
   {
   switch (tab->Stat(loop, SQLTYPE))
      {
      case SQL_TYPE_DATE:
         len = sprintf(buf, "%s DATE,",tab->cols[loop]);
         break;
      case SQL_TYPE_TIME:
         len = sprintf(buf, "%s TIME,",tab->cols[loop]);
         break;
      case SQL_TYPE_TIMESTAMP:
         len = sprintf(buf, "%s TIMESTAMP,",tab->cols[loop]);
         break;
      case SQL_INTEGER:
         len = sprintf(buf, "%s INTEGER,",tab->cols[loop]);
         break;
      case SQL_SMALLINT:
         len = sprintf(buf, "%s SMALLINT,",tab->cols[loop]);
         break;
      case SQL_CHAR:
         len = sprintf(buf, "%s CHAR(%d),",tab->cols[loop], tab->Stat(loop, SQLPREC));
         break;
      case SQL_VARCHAR:
         len = sprintf(buf, "%s VARCHAR(%d),",tab->cols[loop], tab->Stat(loop, SQLPREC));
         break;
      default:
         len = sprintf(buf, "%s Oops, didn't cover this data type,",tab->cols[loop]);
      }
   out = (char *)realloc(out, sizeof(char) * (strlength + len + 5));
   strcpy(&out[strlength], buf);
   strlength += len;
   }
strcpy(&out[strlength - 1], ") IN "); //put it on top the last ,
tsl = strlen(tabspace);
out = (char *)realloc(out, strlength + 5 + tsl);
strcpy(&out[strlength + 4], tabspace);
RunQuery(hdbc, out, NULL, -1, NULL);
}

double UserInp(SQLHANDLE hdbc, const char *title, database *db)
{
Table **tp = NULL, *next = NULL;
static FILE *fp = NULL;
int loop = 0, tabs = 0, choice, colloop, tabch, iloop;
char inp[512], *tok;
static unsigned char flags = 0;
static char *filename = NULL;
double start, finish;
/*
flag 0.  runthru flag.  If set, this func ignores itself
flag 1.  Invalid column selection
flag 2.  Continue loop through interface
flag 3.  Continue loop through table submenu
*/
PrimaryKey *pk;

start = (double)clock();
start = start/CLOCKS_PER_SEC;

if(GetFlag(0, &flags))
   return 1;
SetFlag(2, &flags, 1);
while((next = db->ListTables()) != NULL)
   {
   tp = (Table **)realloc(tp, sizeof(Table *) * (tabs + 1));
   tp[tabs] = next;
   tabs++;
   }
while(GetFlag(2, &flags))
   {
   printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
   printf("%s\n", title);
   for(loop = 0;loop < tabs;loop++)
      printf("%d. %s\n", loop, tp[loop]->name);
   printf("a. Save all Tables to file\n");
   printf("b. Last SQL statment\n");
   printf("c. Run to next checkpoint\n");
   printf("d. Run thru till end\n");
   printf("e. Stop execution\n");
   printf("f. Change output file(%s)\n", filename);
   printf("g. Close current file\n");
   printf("Enter Choice\n");
   gets(inp);
   if(AllNumbers(inp))
      choice = atoi(inp);
   else
      choice = tabs + 1;
   if(choice < tabs)
      {
      SetFlag(3, &flags, 1); //Start the loop up
      while(GetFlag(3, &flags))
         {
         printf("1. Save to File (%s)\n",filename);
         printf("2. Sort by column\n");
         printf("3. Count where (column) = (value)\n");
         printf("4. Find where (column) = (value)\n");
         printf("5. Change output file.\n");
         printf("6. Create table and output to database\n");
         printf("7. Back to main menu.\n");
         printf("Enter Choice\n");
         gets(inp);
         if(AllNumbers(inp))
            {
            if(atoi(inp) < 8)
               tabch = atoi(inp);
            else
               tabch = 666;
            }
         else
            tabch = 666;
         switch (tabch)
            {
            case 1:
               tp[choice]->PrintStats(fp);
               tp[choice]->PrintTable(fp);
               break;
            case 2:
               pk = new PrimaryKey();
               for(colloop = 0; colloop < tp[choice]->columns;colloop++)
                  {
                  for(iloop = 0;iloop < 3;iloop++)
                     {
                     if(colloop + iloop > tp[choice]->columns - 1)
                        break;
                     else
                        printf("%d. %s\t", colloop + iloop, tp[choice]->orderedCols[colloop + iloop].name);
                     }
                  colloop += iloop - 1;
                  printf("\n");
                  }
               printf("Enter the columns by number, separated by a space\n");
               gets(inp);
               tok = inp;
               while((tok = strtok(tok, " ")) != NULL)
                  {
                  if(!AllNumbers(tok))
                     {
                     printf("I'm not psychic.  You gotta type a number.\n");
                     SetFlag(1, &flags, 1);
                     break;
                     }
                  if(atoi(tok) > tp[choice]->columns)
                     {
                     printf("It might work better if you a picked a *valid* column number.\n");
                     SetFlag(1, &flags, 1);
                     break;
                     }
                  pk->AddToKey(&tp[choice]->orderedCols[atoi(tok)]);
                  tok = NULL;
                  }
               if(!GetFlag(1, &flags)) //Only if every single col selec was good
                  tp[choice]->GroupBy(0, tp[choice]->rows, pk);
               else
                  SetFlag(1, &flags, 0);
               delete pk;
               break;
            case 3:
               printf("N.Y.I\n");
               break;
            case 4:
               printf("N.Y.I\n");
               break;
            case 5:
               FileMan(&fp, &filename);
               break;
            case 6:
               printf("Enter Table name:\n");
               gets(inp);
               printf("Note: Using default Tablespace INCDATA\n");
               CreateTable(hdbc, tp[choice], inp, "INCDATA");
               Out(tp[choice], hdbc, 200, NULL, NULL, -1, inp);
               break;
            case 7:
               SetFlag(3, &flags, 0);
               break;
            default:
               printf("%s\n", funny());
            }
         }
      }
   else
      {
      if(inp[0] == 'a') //Output all files
         {
         db->PrintStats(fp);
         }
      else
         {
         if(inp[0] == 'b') //output last SQL statement
            {
            fprintf(fp, "%s\n", lastq);
            }
         else
            {
            if(inp[0] == 'c') //Run to next checkpoint
               {
               SetFlag(2, &flags, 0);
               }
            else
               {
               if(inp[0] == 'd') //Run to end
                  {
                  SetFlag(0, &flags, 1); //this sets the skip flag
                  SetFlag(2, &flags, 0); //And get out of this loop of course
                  free(filename);
                  free(tp);
                  fclose(fp);
                  }
              else
                  {
                  if(inp[0] == 'e')  //exit program
                     {
                     fclose(fp);
                     free(filename);
                     free(tp);
                     finish = (double)clock();
                     finish = finish/CLOCKS_PER_SEC;
                     return -1 * (finish - start);
                     }
                  else
                     {
                     if(inp[0] == 'f') //change filename
                        {
                        FileMan(&fp, &filename);
                        }
                     else
                        if(inp[0] == 'g') //close file
                           {
                           free(filename);
                           fclose(fp);
                           }
                        else       //Make fun of the user :-)
                           {
                           printf("%s\n", funny());
                           }
                        }
                     }
                  }
               }
            }
         }
      }
finish = (double)clock();
finish = finish/CLOCKS_PER_SEC;
return finish - start;
}


void FileMan(FILE **flp, char **fn)
{
char inp[512], *filename;
int hold = 0;
FILE *fp;
fp = *flp;
filename = *fn;

//Otherwise, open up a new file and give 'er hell
printf("The currently opened file is %s\n", filename);
printf("Enter a new file name, enter to keep adding to this one\n");
gets(inp);
if(inp[0] == 0) //Alright, nevermind
   return;

if(fp != NULL) fclose(fp);

fp = fopen(inp, "w");
if(fp == NULL)
   printf("Can't open\n");
free(filename);
filename = (char *)malloc((hold = strlenBytes(inp)));
memcpy(filename, inp, hold);
*fn = filename;
*flp = fp;
}
