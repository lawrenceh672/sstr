#include "table.h"
#include "lhdb.h"
#include "database.h"
#include "rowcol.h"
#include "table.h"

SQLCHAR server[SQL_MAX_DSN_LENGTH + 1] ;
SQLCHAR uid[MAX_UID_LENGTH + 1] ;
SQLCHAR pwd[MAX_PWD_LENGTH + 1] ;
FILE *fp = NULL;

int BCProc(Table *tb, int row);
int DCProc(Table *tb, int row, int sor);
int CPProc(Table *tb, int row, int cpso);
int LTProc(Table *tb, int row, int toto);

int SQL_API_FN storp(
               struct sqlchar *input_data,
               struct sqlda *input_sqlda,
               struct sqlda   *output_sqlda,
               struct sqlca   *ca)
{
//the local sqlca
struct sqlca sqlca;

// Delare CLI Variables
SQLHANDLE henv, hdbc, hstmt;
SQLRETURN rc ;
SQLINTEGER  ind  = 0;

char *Inputs[HVCOUNT];
char *HostV[8] = {":LEAVEYEAR", ":PLATOON", ":CHANGE_DATE", ":TOUR_START", ":SELECT"};
char *st4phv[2] = {":SELECT", ":STATION"};
char *st4prp[2];
char *decsi[3];
char *decshv[3] = {":SOR", ":CPSO", ":TOTO"};
char *rep[5], buffer[50];
char *ast = "*", *tp, *val, *select1, *station, *rrank;
char *runst, *selectHV[1], *selectRP[1], *sh1, *sr1;
char temp[32767];

int stlen, loop, outind, ldind, decst, rowindex, ret, ol1, leavemove, leaveact, rosterlist, iloop;
int pos, tptab, chg, cal, laststat, totstr, regnum, statrow, st4r, st4q, dec, st4t, sel2;
int ustat, toto, cpso, sor, bc, cplt, ltsff, sff, reg, reglm, todsta, rank_ab, todrank, sel1, sel3;
int rank, sel4, sel5, onleave, name, leave1, leave2, leave3, leave4, platoon, seqno, skills;
int tpvar1, tpvar2, deco, senior, bcdcrank, bcdctodrank, ltsff1, rego, statlm, st4s, statrost, stations;
int values, w1, rostrows, bctot, *leave_y;
Row *row;
Table *roster;
double start, finish, ts, tf;
database db;

if(DEBUG)
   {
   if( (fp = fopen("v:\\windows\\desktop\\mr.txt", "w")) == NULL) //A handy way to record the pertinent info
      {
      ret = -666;
      goto ext;
      }
   }
else
   fp = stdout;

//-----------------------------------------------------------------
// Setup CLI required environment
//-----------------------------------------------------------------
start = (double)clock();
start = start/CLOCKS_PER_SEC;

rc = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv ) ;

if ( rc != SQL_SUCCESS )
   return( terminate( henv, rc ) ) ;

rc = SQLAllocHandle( SQL_HANDLE_DBC, henv, &hdbc ) ;
if ( rc != SQL_SUCCESS )
   return( terminate( henv, rc ) ) ;

//-----------------------------------------------------------------
// Issue NULL Connect, since in CLI we need a statement handle
// and thus a connection handle and environment handle.
// A connection is not established, rather the current
// connection from the calling application is used
//-----------------------------------------------------------------

SQLConnect( hdbc, NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS ) ;

//translate all the variables in the sqlda to locals
stlen = input_sqlda->sqlvar[0].sqllen;

Inputs[TOUR_START] = (char *)calloc(1, sizeof(char) * (stlen + 1)); //the sqlda strings arent null terminated
strncpy(Inputs[TOUR_START], input_sqlda->sqlvar[0].sqldata, stlen);

stlen = input_sqlda->sqlvar[3].sqllen;

Inputs[CHANGE_DATE] = (char *)calloc(1, sizeof(char) * (stlen + 1)); //the sqlda strings arent null terminated
strncpy(Inputs[CHANGE_DATE], input_sqlda->sqlvar[3].sqldata, stlen);

leave_y = (int *)input_sqlda->sqlvar[2].sqldata;
_itoa(*leave_y, buffer, 10);
Inputs[LEAVEYEAR] = (char *)malloc(strlen(buffer) + 1);
strcpy(Inputs[LEAVEYEAR], buffer);

Inputs[PLATOON] = (char *)calloc(1,4);
strncpy(Inputs[PLATOON], (char *)input_sqlda->sqlvar[1].sqldata, input_sqlda->sqlvar[1].sqllen);

Inputs[SELECT] = (char *)malloc(2);
strcpy(Inputs[SELECT], "1");
fprintf(fp, "leave year = %s, change date = %s, tour start = %s, platoon = %s\n", Inputs[LEAVEYEAR], Inputs[CHANGE_DATE], Inputs[TOUR_START], Inputs[PLATOON]);

//Set up local variables
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

fprintf(fp, "Changed Date: %s\n Tour Start Date: %s\n Leave Year: %s\n Platoon: %s\n", rep[CHANGE_DATE], rep[TOUR_START], rep[LEAVEYEAR], rep[PLATOON]);

//Step 4 - Check for no records in rosterlist
strcat(temp, "SELECT COUNT(*) FROM USERID.ROSTERLIST WHERE ROSTER_DATE = :CHANGE_DATE AND SEQNO = 1 AND PLATOON = :PLATOON");
runst = HV(temp, HostV, rep, HVCOUNT); //Make the statement

decst = db.NewTable("Decision Statment Query");
rc = RunQuery(hdbc, runst, db.tab(decst), NEW, fp);

ret = atoi(db.tab(decst)->GetData(0,0)); //the answer is a single a single row, single column number
fprintf(fp, "Rows in rosterlist %d.\n", ret);
if(ret > 0)
   {
   fprintf(fp, "Exiting.\n");
   goto ext;
   }

free(runst);

db.DropTable(decst);
//Start construction of in memory table
//4A
strcpy(temp, "SELECT A.REG_NUMBER * 10 AS SEQNO, CAST((SUBSTR(RTRIM(A.LASTNAME || ',' || A.FIRSTNAME),1 ,15)) AS VARCHAR(15)) AS NAME, CAST(COALESCE(A.ROSTER_RANK, A.RANK_ABBREV) AS VARCHAR(4)) AS RANK, CAST(A.STATION AS INTEGER) AS STATION, A.FIRSTRESPONDER, A.HAZMAT, A.TECHRESCUE, A.QDRIVER, A.WATERRESCUE, A.EXTRICATION, A.FOAMPUMP,");
strcat(temp, "CAST(CASE WHEN A.DECLINED_ACT IS NOT NULL AND A.DECLINED_ACT = 'Y' THEN 'Y' ELSE 'N' END AS VARCHAR(4) ) AS DECLINED,CASE WHEN A.SENIORITY = 0 THEN CAST (NULL AS INTEGER) ELSE A.SENIORITY END AS SENIORITY, A.REG_NUMBER AS REGNO, A.PLATOON, CAST('N' AS CHAR(1)) AS CHANGEPLATOON, CAST('N' AS CHAR(1)) AS CALLIN, A.REG_NUMBER * 10 AS SELECT1, CAST('.' AS CHAR(1)) AS LEAVE1,CAST('.' AS CHAR(1)) AS LEAVE2,CAST('.' AS CHAR(1)) AS LEAVE3,CAST('.' AS CHAR(1)) AS LEAVE4, CAST(NULL AS VARCHAR(1)) AS ONLEAVE from userid.allpers A where REG_NUMBER < 4000 AND PLATOON = :PLATOON ");
strcat(temp, "AND BRANCH = 'FF' AND (RETIRED <> 'Y' OR RETIRED IS NULL) AND NOT EXISTS (SELECT * FROM USERID.PENDINGASSIGN P WHERE DATE(P.DATE_EFFECTIVE) <= :CHANGE_DATE");
strcat(temp, " AND P.REG_NUMBER = A.REG_NUMBER)");
runst = HV(temp, HostV, rep, 4); //Make the statement
outind = db.NewTable("Personnel Table"); //Put it in
rc = RunQuery(hdbc, runst, db.tab(outind), NEW, fp);
if(rc != SQL_SUCCESS)
   goto ext;
free(runst);
deco = db.ColIndex(outind, "DECLINED");
rego = db.ColIndex(outind, "REGNO");

//4B
strcpy(temp, "SELECT A.REG_NUMBER * 10, SUBSTR(RTRIM(A.LASTNAME || ',' || A.FIRSTNAME), 1, 15),P.NEWRANK_ABBREV,P.NEWSTATION,A.FIRSTRESPONDER,A.HAZMAT,A.TECHRESCUE,A.QDRIVER,A.WATERRESCUE, ");
strcat(temp, "A.EXTRICATION,A.FOAMPUMP,CASE WHEN A.DECLINED_ACT IS NOT NULL AND A.DECLINED_ACT = 'Y' THEN 'Y' ELSE 'N' END,  CASE WHEN A.SENIORITY = 0 THEN CAST(NULL AS INTEGER) ELSE A.SENIORITY END, A.REG_NUMBER, ");
strcat(temp," A.PLATOON,'N' AS CHANGEPLATOON ,'N' AS CALLIN, A.REG_NUMBER * 10 AS SELECT1, '.' AS LEAVE1, '.' AS LEAVE2, '.' AS LEAVE3, '.' AS LEAVE4, CAST(NULL AS VARCHAR(1)) AS ONLEAVE FROM  USERID.ALLPERS A,USERID.PENDINGASSIGN P WHERE A.REG_NUMBER < 4000 AND (A.RETIRED IS NULL OR A.RETIRED <> 'Y') AND P.REG_NUMBER = A.REG_NUMBER AND P.DATE_EFFECTIVE = (SELECT MAX(Q.DATE_EFFECTIVE) FROM USERID.PENDINGASSIGN Q ");
strcat(temp," WHERE  Q.REG_NUMBER = A.REG_NUMBER AND DATE(Q.DATE_EFFECTIVE) <= :CHANGE_DATE");
strcat(temp, " AND P.SEQ_NO = (SELECT MAX(R.SEQ_NO) FROM USERID.PENDINGASSIGN R WHERE R.REG_NUMBER = A.REG_NUMBER AND DATE(R.DATE_EFFECTIVE) = DATE(P.DATE_EFFECTIVE)) AND P.NEWBRANCH = 'FF' AND P.NEWPLATOON = :PLATOON");
strcat(temp, " AND (P.NEWRETIRED IS NULL OR P.NEWRETIRED <> 'Y'))");
runst = HV(temp, HostV, rep, 4); //Make the statement
rc = RunQuery(hdbc, runst, db.tab(outind), APPEND, fp);
if(rc != SQL_SUCCESS)
   goto ext;
free(runst);

//4C
strcpy(temp, "SELECT LEAVEYEAR, PLATOON, DATE(CHANGE_DATE) AS DATE, TIME(CHANGE_DATE) AS TIME, SEQ_NO, REG_NUMBER, REG2_NUMBER, CAST(DATE(CHANGE_DATE) - :TOUR_START AS INTEGER) AS XDAY, CAST(DATE(CHANGE_DATE) - :CHANGE_DATE AS INTEGER) AS CDAY FROM USERID.LEAVEDAY WHERE LEAVEYEAR = :LEAVEYEAR AND PLATOON = :PLATOON AND DATE(CHANGE_DATE) BETWEEN :TOUR_START AND (:TOUR_START + 3 DAYS)");
runst = HV(temp, HostV, rep, 4); //Make the statement
ldind = db.NewTable("LeaveDay"); //Put it in
rc = RunQuery(hdbc, runst, db.tab(ldind), NEW, fp);
if(rc != SQL_SUCCESS)
   goto ext;
free(runst);

//4D
strcpy(temp, "SELECT D.REG_NUMBER * 10 + 1,SUBSTR(RTRIM(A.LASTNAME || ',' || A.FIRSTNAME), 1, 15),COALESCE(A.ROSTER_RANK, A.RANK_ABBREV), A.STATION, A.FIRSTRESPONDER,A.HAZMAT, A.TECHRESCUE,A.QDRIVER, A.WATERRESCUE, A.EXTRICATION,A.FOAMPUMP, CASE WHEN A.DECLINED_ACT IS NOT NULL AND A.DECLINED_ACT = 'Y' THEN 'Y' ELSE 'N' END AS DECLINED, CASE WHEN A.SENIORITY = 0 THEN CAST(NULL AS INTEGER) ELSE A.SENIORITY END AS SENIORITY, D.REG2_NUMBER, A.PLATOON, 'Y' AS CHANGEPLATOON, 'N' AS CALLIN, D.REG_NUMBER * 10 + 1 AS SELECT1, '.' AS LEAVE1, '.' AS LEAVE2, '.' AS LEAVE3, '.' AS LEAVE4, CAST(NULL AS VARCHAR(1)) AS ONLEAVE FROM USERID.LEAVEDAY D, USERID.ALLPERS A WHERE D.LEAVEYEAR = :LEAVEYEAR AND D.PLATOON = :PLATOON AND DATE(D.CHANGE_DATE) = :CHANGE_DATE AND A.REG_NUMBER = D.REG2_NUMBER");
runst = HV(temp, HostV, rep, 4); //Make the statement
rc = RunQuery(hdbc, runst, db.tab(outind), APPEND, fp);
if(rc != SQL_SUCCESS)
   goto ext;
free(runst);

//4E
sel1 = db.ColIndex(ldind, "CDAY");
for(loop = 0;loop < db.tab(ldind)->rows;loop++)
   {
   rowindex = db.Find(outind, rego, db.tab(ldind)->GetData(loop, 5)); //Find the regnumber from userid.leaveday matching the regnumber in the personnel table
   if(rowindex != -1) //There is a personnal match
      {
      db.tab(outind)->SetData(rowindex, 18 + atoi(db.tab(ldind)->GetData(loop, 7)), ast, 2);
      if(atoi(db.GetData(ldind, loop, sel1)) == 0)
         db.tab(outind)->SetData(rowindex, 22, ast, 2); //the status for the person on the change date the onleave field
      }
   }

db.DropTable(ldind);

//4F
strcpy(temp, "SELECT * FROM USERID.LEAVEMOVE WHERE PLATOON = :PLATOON AND LEAVEYEAR = :LEAVEYEAR AND DATE(CHANGE_DATE) = :CHANGE_DATE");
runst = HV(temp, HostV, rep, 4); //Make the statement
leavemove = db.NewTable("LeaveMove"); //Put it in
rc = RunQuery(hdbc, runst, db.tab(leavemove), NEW, fp);
if(rc != SQL_SUCCESS)
   goto ext;
free(runst);

//4G
strcpy(temp, "SELECT D.REG_NUMBER * 10 + 2 AS SEQNO, SUBSTR(RTRIM(A.LASTNAME || ',' || A.FIRSTNAME), 1, 15) AS NAME,COALESCE(A.ROSTER_RANK, A.RANK_ABBREV) AS RANK, A.STATION,A.FIRSTRESPONDER, A.HAZMAT, A.TECHRESCUE, A.QDRIVER, A.WATERRESCUE, A.EXTRICATION,A.FOAMPUMP,A.DECLINED_ACT, CASE WHEN A.SENIORITY = 0 THEN CAST(NULL AS INTEGER) ELSE A.SENIORITY END AS SENIORITY, D.REG_NUMBER AS REGNO, A.PLATOON, CAST('N' AS CHAR(1)) AS CHANGEPLATOON, CAST('Y' AS CHAR(1)) AS CALLIN, CAST('.' AS CHAR(1)) AS LEAVE1,CAST('.' AS CHAR(1)) AS LEAVE2,CAST('.' AS CHAR(1)) AS LEAVE3,CAST('.' AS CHAR(1)) AS LEAVE4, CAST(NULL AS VARCHAR(1)) AS ONLEAVE, CAST(NULL AS VARCHAR(6)) AS SKILLS FROM USERID.LEAVEMOVE D, USERID.ALLPERS A WHERE D.LEAVEYEAR = :LEAVEYEAR AND D.PLATOON = :PLATOON AND DATE(D.CHANGE_DATE) = :CHANGE_DATE AND A.REG_NUMBER = D.REG_NUMBER AND A.PLATOON <> :PLATOON AND NOT EXISTS (SELECT * FROM USERID.LEAVEDAY E WHERE E.LEAVEYEAR = :LEAVEYEAR AND E.PLATOON = :PLATOON AND DATE(E.CHANGE_DATE) = :CHANGE_DATE AND E.REG2_NUMBER = D.REG_NUMBER)");
sel1 = db.NewTable("4G");
runst = HV(temp, HostV, rep, 4); //Make the statement
rc = RunQuery(hdbc, runst, db.tab(sel1), NEW, fp);
if(rc != SQL_SUCCESS)
   goto ext;
free(runst);
db.AutoCopy(sel1, outind);
db.DropTable(sel1);

//4H
strcpy(temp, "SELECT LEAVEYEAR,PLATOON,REG_NUMBER,SEQ_NO,LEAVETYPE,DATE(LEAVESTART) AS LSDATE, TIME(LEAVESTART) AS LSTIME,STATION,LEAVETAKEN,STARTTIME, ENDTIME, CAST(DATE(LEAVESTART) - :TOUR_START AS INTEGER) AS XDAY, CAST(DATE(LEAVESTART) - :CHANGE_DATE AS INTEGER) AS CDAY FROM USERID.LEAVEACT WHERE LEAVEYEAR = :LEAVEYEAR AND PLATOON = :PLATOON AND DATE(LEAVESTART) BETWEEN DATE(:TOUR_START) AND (DATE(:TOUR_START) + 3 DAYS)");
runst = HV(temp, HostV, rep, 4); //Make the statement
leaveact = db.NewTable("LeaveAct"); //Put it in
rc = RunQuery(hdbc, runst, db.tab(leaveact), NEW, fp);
if(rc != SQL_SUCCESS)
   goto ext;
free(runst);

//4I
sel1 = db.ColIndex(leaveact, "CDAY");
for(loop = 0;loop < db.tab(leaveact)->rows;loop++)
   {
   rowindex = db.Find(outind, rego, db.tab(leaveact)->GetData(loop, 2)); //Find the regnumber from userid.leaveact matching the regnumber in the personnel table
   if(rowindex != -1)
      {
      db.tab(outind)->SetData(rowindex, 18 + atoi(db.tab(leaveact)->GetData(loop, 11)), db.tab(leaveact)->GetData(loop, 4), 2);//change the leavex field to leavetype field of the leaveact table
      if(atoi(db.GetData(leaveact, loop, sel1)) == 0)
         db.tab(outind)->SetData(rowindex, 22, db.tab(leaveact)->GetData(loop, 4), 2); //the status for the person on the change date the onleave field
      }
   }

db.DropTable(leaveact);

//Prepare to insert records into userid.rosterlist
//Get the table format for userid.rosterlist
strcpy(temp, "SELECT ROSTER_DATE,PLATOON,SEQNO,SELECT1,SELECT2,SELECT3,SELECT4,SELECT5,NAME,RANK,STATION, CAST(DECLINED AS VARCHAR(4)) AS DECLINED,SENIORITY,REGNO,CHANGEPLATOON,CALLIN,TODAYRANK,TODAYSTATION,LEAVE1,LEAVE2,LEAVE3,LEAVE4,SKILLS,ONLEAVE FROM USERID.ROSTERLIST WHERE NAME = '666'");

rosterlist = db.NewTable("RosterList"); //get the column info
rc = RunQuery(hdbc, temp, db.tab(rosterlist), NEW, fp);
if(rc != SQL_SUCCESS)
   goto ext;

//Put into Rosterlist corresponding rows from the outind table
db.Sort(outind, db.ColIndex(outind, "REGNO")); //this way there rows will line up row for row, and the find in the late step won't put them out of whack
db.AutoCopy(outind, rosterlist);

chg = db.ColIndex(rosterlist, "CHANGEPLATOON"); //14
cal = db.ColIndex(rosterlist, "CALLIN");    //15
reg = db.ColIndex(rosterlist, "REGNO"); //13
reglm = db.ColIndex(leavemove, "REG_NUMBER");     //4
todsta = db.ColIndex(rosterlist, "TODAYSTATION"); //17
statrost = db.ColIndex(rosterlist, "STATION");
rank_ab = db.ColIndex(leavemove, "RANK_ABBREV"); //6
todrank = db.ColIndex(rosterlist, "TODAYRANK"); //16
sel1 = db.ColIndex(rosterlist, "SELECT1"); //3
sel2 = db.ColIndex(rosterlist, "SELECT2");//4
sel3 = db.ColIndex(rosterlist, "SELECT3");   //5
sel4 = db.ColIndex(rosterlist, "SELECT4");      //6
sel5 = db.ColIndex(rosterlist, "SELECT5");
rank = db.ColIndex(rosterlist, "RANK"); //9
onleave = db.ColIndex(rosterlist, "ONLEAVE");
platoon = db.ColIndex(rosterlist, "PLATOON");
seqno = db.ColIndex(rosterlist, "SEQNO");
leave1 = db.ColIndex(rosterlist, "LEAVE1");
leave2 = db.ColIndex(rosterlist, "LEAVE2");
leave3 = db.ColIndex(rosterlist, "LEAVE3");
leave4 = db.ColIndex(rosterlist, "LEAVE4");
name = db.ColIndex(rosterlist, "NAME");
skills = db.ColIndex(rosterlist, "SKILLS");
statlm = db.ColIndex(leavemove, "STATION");
senior = db.ColIndex(rosterlist, "SENIORITY");
dec = db.ColIndex(rosterlist, "DECLINED");

//4J
//Now set up the seqno(col 2) in rosterlist properly properly
for(loop = 0;loop < db.tab(rosterlist)->rows;loop++)
   {
   _itoa(loop + 1, buffer, 10);
   stlen = strlen(buffer) + 1;
   db.SetData(rosterlist, loop, 2,  buffer);
   }

//4K && 4l
for(loop = 0;loop < db.tab(rosterlist)->rows;loop++)
   {
   val = db.tab(rosterlist)->GetData(loop, chg);
   tp = db.tab(rosterlist)->GetData(loop, cal);
   if(NULLstrcmp(val, "N") == 0) //NX
      {
      rowindex = db.Find(leavemove, reglm, db.GetData(rosterlist, loop, reg)); //Find the regnumber from userid.leavemove matching the regnumber in the rosterlist table
      if(rowindex != -1) //the regnumber does have an entry in the leavemove table
         {
         db.SetData(rosterlist, loop, todsta, (val = db.GetData(leavemove, rowindex, statlm))); //Copy the station data in
         }
      else  //NX no matching leavemove record
         db.tab(rosterlist)->SetData(loop, todsta, NULL, 0);
      if(NULLstrcmp(tp, "N") == 0) //NN
         {
         if(rowindex != -1) //the regnumber does have an entry in the leavemove table
            db.SetData(rosterlist, loop, todrank, (val = db.GetData(leavemove, rowindex, rank_ab))); //Copy the rank_abbrev data in
         else
            db.SetData(rosterlist, loop, todrank, NULL); //otherwise its NULL. It already is, as a result of new table formed, and the autocopy should'nt have touched this column.  But this is better.  After all we wouldn't want unexpected bugs popping up after I leave would we?  I mean who would want to go through this code, with all the hard coded numbers(I guess I did.  I replaced them :-) ) and do anything with it?  Even more appropriate, who would want to read these big comments?  Why are you?  Should'nt you be working?!                                                                                                               What?   I've got nothing more to say..... :-)
         }
      else    //NY
         {
         db.tab(rosterlist)->SetData(loop, todrank, NULL, 0); //automatically NULL
         }
      }
   else //YX
      {
      db.tab(rosterlist)->SetData(loop, todrank, NULL, 0); //Set tod rank to NULL if either change or call is Y
      rowindex = db.Find(leavemove, reglm, db.tab(rosterlist)->GetData(loop, reg)); //Find the regnumber from userid.leavemove matching the regnumber in the rosterlist table
      if(rowindex != -1) //the regnumber does have an entry in the leavemove table
         {
         db.SetData(rosterlist, loop, todsta, db.GetData(leavemove, rowindex, statlm)); //Copy the station data in
         }
      else //They don't have an entry in leavemove.  Look for the person replaced reg number
         {
         //Their reg number is the select1 field divided by 10  Chop off the last character
         if((val = db.GetData(rosterlist, loop, sel1)) != NULL)
            {
            strcpy(buffer, val);
            buffer[strlen(buffer) - 1] = 0; //Chop!
            rowindex = db.Find(leavemove, reglm, buffer); //Find the regnumber from userid.leavemove matching the replaced person's reg number
            }
         else
            {
            buffer[0] = 0; //it'll have to use the replaced person's station from the personnel table
            rowindex = -1;
            }
         if(rowindex != -1) //the regnumber does have an entry in the leavemove table
            {
            //Find the entry in leavemove for this reg number
            val = db.tab(leavemove)->GetData(rowindex, reglm);
            rowindex = db.Find(leavemove, reglm, val);
            val = db.GetData(leavemove, rowindex, statlm);
            db.tab(rosterlist)->SetData(loop, todsta, val, strlenBytes(val)); //Copy the station data in
            }
         else //The person being replaced doesn't have an entry in the leavemore table.  Use the station value from outind
            {
            rowindex = db.Find(rosterlist, reg, buffer); //Find the regnumber from outind matching the reg number of the replacee
            if(rowindex != -1) //the regnumber does have an entry in the outind table
               {
               db.SetData(rosterlist, loop, todsta, (val = db.GetData(rosterlist, rowindex, db.ColIndex(rosterlist, "STATION")))); //Copy the station data in
               }
            else
               {
               if((db.tab(rosterlist)->GetData(loop, 13) != NULL) && (db.tab(rosterlist)->GetData(loop, 1) != NULL))
                  fprintf(fp, "No station value could be obtained for %d, %s\n", atoi(db.tab(outind)->GetData(loop, 13)), db.tab(outind)->GetData(loop, 1));
               else
                  fprintf(fp, "No station value could be obtained and aparently this person doesn't exist\n");
               }
            }
         }
      }
   }

db.DropTable(leavemove);
//4M
db.Sort(rosterlist, reg); //thats the order of outind
for(loop = 0;loop < db.Stat(rosterlist, ROWS);loop++)
   {
   memset(buffer, 0, 50); //prepare it to hold the skills stuff
   iloop = 0;
   val = db.GetData(outind, loop, db.ColIndex(outind, "FIRSTRESPONDER"));
   if(val != NULL)
      {
      if(val[0] == '*')
         {
         buffer[iloop] = 'R';
         iloop++;
         }
      if(val[0] == 'S')
         {
         buffer[iloop] = 'S';
         iloop++;
         }
      }
   val = db.GetData(outind, loop, db.ColIndex(outind, "HAZMAT"));
   if(val != NULL)
      {
      if(val[0] == '1')
         {
         buffer[iloop] = 'H';
         iloop++;
         }
      else
         {
         if(val[0] == '2')
            {
            buffer[iloop] = 'I';
            iloop++;
            }
         else
            {
            if(val[0] == '3')
               {
               buffer[iloop] = 'I';
               iloop++;
               }
            }
         }
      }
   val = db.GetData(outind, loop, db.ColIndex(outind, "TECHRESCUE"));
   if(val != NULL)
      {
      buffer[iloop] = 'T';
      iloop++;
      }
   val = db.GetData(outind, loop, db.ColIndex(outind, "FOAMPUMP"));
   if(val != NULL)
      {
      buffer[iloop] = 'P';
      iloop++;
      }
   else
      {
      val = db.GetData(outind, loop, db.ColIndex(outind, "QDRIVER"));
      if(val != NULL)
         {
         if(val[0] == '2' || val[0] == '3' || val[0] == '4' || val[0] == '5')
            {
            buffer[iloop] = 'Q';
            iloop++;
            }
         }
      }
   val = db.GetData(outind, loop, db.ColIndex(outind, "WATERRESCUE" ));
   if(val != NULL)
      {
      if(val[0] == '1')
         {
         buffer[iloop] = 'W';
         iloop++;
         }
      else
         {
         if(val[0] == '2')
            {
            buffer[iloop] = 'X';
            iloop++;
            }
         else
            {
            if(val[0] == '3')
               {
               buffer[iloop] = 'X';
               iloop++;
               }
            }
         }
      }
   val = db.GetData(outind, loop, db.ColIndex(outind, "EXTRICATION"));
   if(val != NULL)
      {
      buffer[iloop] = 'E';
      iloop++;
      }
   iloop++;
   db.SetData(rosterlist, loop, skills, buffer); //Phew! That looked much nicer in SQL
   }
//4N
for(loop = 0;loop < db.Stat(rosterlist, ROWS);loop++)
   {
   val = db.GetData(rosterlist, loop, onleave);
   if(isBlank(val)) //isBlank checks for all ' ' in a string
      db.SetData(rosterlist, loop, sel2, NULL);
   else
      db.SetData(rosterlist, loop, sel2, db.GetData(rosterlist, loop, sel1));
   }
//4O
for(loop = 0;loop < db.Stat(rosterlist, ROWS);loop++)
   {
   val = Coalesce(db.GetData(rosterlist, loop, todsta), db.GetData(rosterlist, loop, statrost));
   if(val != NULL && (db.GetData(rosterlist, loop, sel1) != NULL))
      {
      iloop = atoi(val);
      iloop = iloop * 100000 + atoi(db.GetData(rosterlist, loop, sel1));
      _itoa(iloop, buffer, 10);
      db.SetData(rosterlist, loop, sel3, buffer);
      }
   //Set Select4 to be select3 'pending on Todayrank or Rank
   if( (val = db.GetData(rosterlist, loop, todrank)) == NULL)
      {
      if( (val = db.GetData(rosterlist, loop, rank)) == NULL)
         { //Therefore SELECT4 must SELECT3 cuz RANK and TODAYRANK are null
         db.SetData(rosterlist, loop, sel4, (val = db.GetData(rosterlist, loop, sel3)));
         }
      else
         {
         if( ((NULLstrcmp("sFF", val) != 0) || (NULLstrcmp("FF", val) != 0)) || (!isBlank(db.GetData(rosterlist, loop, onleave))))
            {
            db.SetData(rosterlist, loop, sel4, NULL);
            }
         else
            {
            db.SetData(rosterlist, loop, sel4, db.GetData(rosterlist, loop, sel3));
            }
         }
      }
   else
      {
      if( ((NULLstrcmp("sFF", val) != 0) || (NULLstrcmp("FF", val) != 0)) || (!isBlank(db.GetData(rosterlist, loop, onleave))))
         {
         db.SetData(rosterlist, loop, sel4, NULL);
         }
      else
         {
         db.SetData(rosterlist, loop, sel4, db.GetData(rosterlist, loop, sel3));
         }
      }
   }

//4P
values = 4; //Take that as a given.  This is the amount of different regno's for the number of people missing
//Contains a row for every station out there
strcpy(temp, "SELECT * FROM USERID.STATIONINFO");
ustat = db.NewTable("USERID.STATIONINFO");
rc = RunQuery(hdbc, temp, db.tab(ustat), NEW, fp);
if(rc != SQL_SUCCESS)
   goto ext;


ol1 = db.ColIndex(outind, "STATION");
tptab = db.DupeTable(outind, "Copy of Personnel Table"); //copy the column info over
db.AutoCopy(outind, tptab); //then the actual data

rowindex = db.Find(tptab, chg, "Y");
while(rowindex != -1) //find all the rows with this value
   {
   db.DropRow(tptab, rowindex);
   rowindex = db.Find(tptab, chg, "Y");
   }

rowindex = db.Find(tptab, cal, "Y"); //Now wipe out rows with a Y in callin
while(rowindex != -1)
   {
   db.DropRow(tptab, rowindex);
   rowindex = db.Find(tptab, cal, "Y");
   }

db.Sort(tptab, ol1);
tpvar2 = db.ColIndex(ustat, "TOTALSTRENGTH");
for(loop = 0;loop < db.Stat(tptab, ROWS);loop++)
   {
   station = db.GetData(tptab, loop, ol1);
   if(station != NULL) //Just in case
      {
      if(atoi(station) != laststat) //This is a new station
         {
         laststat = atoi(station);
         pos = db.Count(tptab, ol1, station);
         statrow = db.Find(ustat, 0, station);
         if((statrow != -1) && ((val = db.GetData(ustat, statrow, tpvar2)) != NULL))
            {
            totstr = atoi(val); //get the totalstrength
            for(iloop = 0;iloop < totstr - pos;iloop++)
               {
               if((totstr - pos) > (values - 1))
                  tpvar1 = values;
               else
                  tpvar1 = totstr - pos;
               //the reg number is 9000 + station*10 + totalstrength - current strength
               regnum = atoi(_itoa((9000 + (atoi(station) * 10) + tpvar1), buffer, 10));
               //Make the row to indicate a vacancy
               rowindex = db.NewRow(rosterlist);
               db.SetData(rosterlist, rowindex, reg, _itoa(regnum, buffer, 10));
               db.SetData(rosterlist, rowindex, sel1, _itoa(regnum * 10, buffer, 10)); //same thing
               db.SetData(rosterlist, rowindex, sel2, buffer);
               db.SetData(rosterlist, rowindex, sel3, _itoa(regnum *10 + (atoi(station) * 100000), buffer, 10));
               db.SetData(rosterlist, rowindex, sel4, NULL);
               db.SetData(rosterlist, rowindex, sel5, NULL);
               db.SetData(rosterlist, rowindex, leave1, "V");
               db.SetData(rosterlist, rowindex, leave2, "V");
               db.SetData(rosterlist, rowindex, leave3, "V");
               db.SetData(rosterlist, rowindex, leave4, "V");
               db.SetData(rosterlist, rowindex, onleave, "V");
               db.SetData(rosterlist, rowindex, name, "Vacancy");
               db.SetData(rosterlist, rowindex, platoon, "*");
               db.SetData(rosterlist, rowindex, rank, "-");
               db.SetData(rosterlist, rowindex, skills, "");
               db.SetData(rosterlist, rowindex, seqno, _itoa(1000 + UniqueInt(), buffer, 10));
               db.SetData(rosterlist, rowindex, station, db.GetData(ustat, statrow, 0));
               db.SetData(rosterlist, rowindex, chg, "N");
               db.SetData(rosterlist, rowindex, cal, "N");
               db.SetData(rosterlist, rowindex, statrost, station);
               }
            }
         loop += pos; //Don't need to look along those records no more, on account of the count function sorting it on station number
         }
      }
   }
//These are no longer required
db.DropTable(outind);
db.DropTable(tptab);

//4q
ol1 = db.ColIndex(ustat, "STATION");
rowindex = db.NewRows(rosterlist, (tpvar1 = db.Stat(ustat, ROWS)));
db.Sort(ustat, ol1);
stations = db.Stat(ustat, ROWS);
for(loop = 0;loop < stations;loop++)
   {
   station = db.GetData(ustat, loop, ol1); //get the first record to start counting from
   db.SetData(rosterlist, rowindex + loop, chg, "Y");
   db.SetData(rosterlist, rowindex + loop, cal, "Y");
   db.SetData(rosterlist, rowindex + loop, onleave, "Y");
   db.SetData(rosterlist, rowindex + loop, sel1, NULL);
   db.SetData(rosterlist, rowindex + loop, sel2, NULL);
   db.SetData(rosterlist, rowindex + loop, sel3, _itoa(atoi(station) * 100000, buffer, 10));
   db.SetData(rosterlist, rowindex + loop, sel4, buffer);
   db.SetData(rosterlist, rowindex + loop, sel5, NULL);
   db.SetData(rosterlist, rowindex + loop, name, strcat(strcpy(buffer, "Station "), station));
   db.SetData(rosterlist, rowindex + loop, platoon, "*");
   db.SetData(rosterlist, rowindex + loop, seqno, _itoa(2000 + UniqueInt(), buffer, 10));
   db.SetData(rosterlist, rowindex + loop, reg, _itoa(0, buffer, 10));
   db.SetData(rosterlist, rowindex + loop, rank, " ");
   db.SetData(rosterlist, rowindex + loop, statrost, _itoa(loop + 1, buffer, 10));
   db.SetData(rosterlist, rowindex + loop, skills, " ");
   db.SetData(rosterlist, rowindex + loop, leave1, " ");
   db.SetData(rosterlist, rowindex + loop, leave2, " ");
   db.SetData(rosterlist, rowindex + loop, leave3, " ");
   db.SetData(rosterlist, rowindex + loop, leave4, " ");
   db.SetData(rosterlist, rowindex + loop, senior, _itoa(0, buffer, 10));
   }

//4r
strcpy(temp, "SELECT M.STATION * 100000 + 90000 + M.SEQ_NO AS SEQNO,CAST('Person from ' || CHAR(M.REG_NUMBER) || ' ' || CASE WHEN M.FIRSTRESPONDER IS NOT NULL THEN 'R' ELSE '' END || CASE WHEN M.QDRIVER IS NOT NULL THEN 'Q' ELSE '' END || CASE WHEN M.TECHRESCUE IS NOT NULL THEN 'T' ELSE '' END || CASE WHEN M.HAZMAT IS NOT NULL THEN 'H' ELSE '' END || CASE WHEN M.WATERRESCUE IS NOT NULL THEN 'W' ELSE '' END || CASE WHEN M.EXTRICATION IS NOT NULL THEN 'E' ELSE '' END AS VARCHAR(15)) AS NAME, CAST (' ' AS VARCHAR(4)) AS RANK, M.STATION, CAST(' ' AS VARCHAR(4)) AS DECLINED,0 AS SENIORITY, CAST(NULL AS INTEGER) AS REGNO,CAST (NULL AS CHAR(1)) AS PLATOON, CAST('N' AS CHAR(1)) AS CHANGEPLATOON, CAST('N' AS CHAR(1)) AS CALLIN, CAST(NULL AS VARCHAR(4)) AS TODAYRANK, CAST(NULL AS INTEGER) AS TODAYSTATION,CAST('.' AS CHAR(1)) AS LEAVE1, CAST('.' AS CHAR(1)) AS LEAVE2, CAST('.' AS CHAR(1)) AS LEAVE3, CAST('.' AS CHAR(1)) AS LEAVE4, CAST(' ' AS VARCHAR(6)) AS SKILLS1, ' ' AS SKILLS2, ' ' AS SKILLS3,' ' AS SKILLS4, ' ' AS SKILLS5, ' ' AS SKILLS6, CAST(NULL AS VARCHAR(1)) AS ONLEAVE FROM USERID.LEAVEMOVE M WHERE (:SELECT IN (3,4)) AND M.LEAVEYEAR = :LEAVEYEAR AND M.PLATOON = :PLATOON AND DATE(CHANGE_DATE) = :CHANGE_DATE AND M.REG_NUMBER < 100");
st4r = db.NewTable("st4r");
runst = HV(temp, HostV, rep, 5); //use the first 5 variables b/c of the :SELECT
rc = RunQuery(hdbc, runst, db.tab(st4r), NEW, fp);
if(rc != SQL_SUCCESS)
   goto ext;
free(runst);
db.AutoCopy(st4r, rosterlist);

//4s
strcpy(temp, "SELECT M.REG_NUMBER * 100000 + 95000 + M.SEQ_NO AS SEQNO, CAST('Person to   ' || CHAR(M.STATION) || ' ' || CASE WHEN M.FIRSTRESPONDER IS NOT NULL THEN 'R' ELSE '' END || CASE WHEN M.QDRIVER IS NOT NULL THEN 'Q' ELSE '' END || CASE WHEN M.TECHRESCUE IS NOT NULL THEN 'T' ELSE '' END || CASE WHEN M.HAZMAT IS NOT NULL THEN 'H' ELSE '' END || CASE WHEN M.WATERRESCUE IS NOT NULL THEN 'W' ELSE '' END || CASE WHEN M.EXTRICATION IS NOT NULL THEN 'E' ELSE '' END AS VARCHAR(15)) AS NAME,CAST (' ' AS VARCHAR(4)) AS RANK, M.STATION, CAST(' ' AS VARCHAR(4)) AS DECLINED, 0 AS SENIORITY, CAST(' ' AS INTEGER) AS REGNO, CAST (NULL AS CHAR(1)) AS PLATOON, CAST('Y' AS CHAR(1)) AS CHANGEPLATOON, CAST('Y' AS CHAR(1)) AS CALLIN, CAST(NULL AS VARCHAR(4)) AS TODAYRANK, CAST(NULL AS INTEGER) AS TODAYSTATION, CAST('.' AS CHAR(1)) AS LEAVE1, CAST('.' AS CHAR(1)) AS LEAVE2, CAST('.' AS CHAR(1)) AS LEAVE3, CAST('.' AS CHAR(1)) AS LEAVE4, CAST(' ' AS VARCHAR(6)) AS SKILLS, ' ' AS SKILLS2, ' ' AS SKILLS3,' ' AS SKILLS4, ' ' AS SKILLS5, ' ' AS SKILLS6, CAST(NULL AS VARCHAR(1)) AS ONLEAVE FROM USERID.LEAVEMOVE M WHERE (:SELECT IN (3,4)) AND M.LEAVEYEAR = :LEAVEYEAR AND M.PLATOON = :PLATOON AND DATE(CHANGE_DATE) = :CHANGE_DATE AND M.REG_NUMBER < 100 ORDER BY 1");
st4s = db.NewTable("st4s");
runst = HV(temp, HostV, rep, 5); //use the first 5 variables b/c of the :SELECT
rc = RunQuery(hdbc, runst, db.tab(st4s), NEW, fp);
if(rc != SQL_SUCCESS)
   goto ext;
free(runst);
db.AutoCopy(st4s, rosterlist);
//4T
strcpy(temp, "SELECT LEAVEYEAR, PLATOON, DATE(CHANGE_DATE) AS CHANGE_DATE, SEQ_NO, UNIT_NAME FROM USERID.LEAVEOFFDUTY WHERE LEAVEYEAR = :LEAVEYEAR AND PLATOON = :PLATOON AND (DATE(CHANGE_DATE) = :CHANGE_DATE)");
st4t = db.NewTable("userid.leaveoffduty");
runst = HV(temp, HostV, rep, 4);
rc = RunQuery(hdbc, runst, db.tab(st4t), NEW, fp);
if(rc != SQL_SUCCESS)
   goto ext;

//Get the sum of the SOFFSTRENGTH
pos = 0;
for(loop = 0;loop < db.Stat(ustat, ROWS);loop++)
   {
   if(AllNumbers(val = db.GetData(ustat, loop, db.ColIndex(ustat, "SOFFSTRENGTH"))))
      pos += atoi(val);
   }
sor = pos;
cpso = pos + db.Stat(ustat, ROWS);
pos = 0;
for(loop = 0;loop < db.Stat(ustat, ROWS);loop++)
   {
   if(AllNumbers(db.GetData(ustat, loop, db.ColIndex(ustat, "OFFSTRENGTH"))))
      pos += atoi(val);
   }
toto = sor + pos;

//Now adjust the numbers to reflect status of the units
for(loop = 0;loop < db.Stat(st4t, ROWS);loop++)
   {
   val = db.GetData(st4t, loop, db.ColIndex(st4t, "UNIT_NAME"));
   if(val[0] == '5' && (strlen(val) == 2))
      {
      sor--;
      cpso--;
      }
   if((val[0] == '4') && (strlen(val) == 3) && (NULLstrcmp(val, "412") != 0) && (NULLstrcmp(val, "413") != 0))
      {
      cpso--;
      }
   toto--; //Every unit off duty reduces the total number of officers
   }
//Keep this all up to date
_itoa(sor, buffer, 10);
decsi[0] = (char *)malloc(strlen(buffer) * sizeof(char));
strcpy(decsi[0], buffer);

_itoa(cpso, buffer, 10);
decsi[1] = (char *)malloc(strlen(buffer) * sizeof(char));
strcpy(decsi[1], buffer);

_itoa(toto, buffer, 10);
decsi[2] = (char *)malloc(strlen(buffer) * sizeof(char));
strcpy(decsi[2], buffer);
//Drop the useless Tables
db.DropTable(ustat);
db.DropTable(st4r);
db.DropTable(st4s);
db.DropTable(st4t);

db.Sort(rosterlist, name);
tptab = db.DupeTable(rosterlist, "Declined Column Processing Table"); //so that the processing doesn't mess with the future processing
db.AutoCopy(rosterlist, tptab);
db.Sort(tptab, name);

//Declined Column
rostrows = db.Stat(rosterlist, ROWS);
//db.SaveOrder(rosterlist);
for(loop = 0;loop < rostrows;loop++)
   {
   if(db.GetData(tptab, loop, onleave) != NULL)
      {
      if(db.GetData(tptab, loop, dec) != NULL && NULLstrcmp(db.GetData(tptab, loop, dec), "Y") == 0)
         db.SetData(rosterlist, loop, dec, "Dec ");
      else
         db.SetData(rosterlist, loop, dec, "    ");
      }
   else
      {
      if((val = db.GetData(tptab, loop, todsta)) != NULL)
         {
         buffer[0] = '(';
         strcpy(&buffer[1], db.GetData(tptab, loop, statrost));
         strcat(buffer, ")");
         db.SetData(rosterlist, loop, dec, buffer);
         }
      else
         {
         if((val = db.GetData(tptab, loop, dec)) != NULL && (NULLstrcmp(val, "Y") == 0))
            db.SetData(rosterlist, loop, dec, "Dec ");
         else //Process the acting stuff
            {
            if(BCProc(db.tab(tptab), loop) == 1)
               db.SetData(rosterlist, loop, dec, "B/C ");
            else
               {
               if(DCProc(db.tab(tptab), loop, sor) == 1)
                  db.SetData(rosterlist, loop, dec, "D/C ");
               else
                  {
                  if(CPProc(db.tab(tptab), loop, cpso) == 1)
                    db.SetData(rosterlist, loop, dec, "CP  ");
                   else
                     {
                     if(LTProc(db.tab(tptab), loop, toto) == 1)
                        db.SetData(rosterlist, loop, dec, "LT  ");
                     else
                        db.SetData(rosterlist, loop, dec, "    ");
                     }
                  }
               }
            }
         }
      }
   }

for(loop = 0;loop < db.Stat(rosterlist, ROWS);loop++) //Put the roster date back in there
   db.SetData(rosterlist, loop, db.ColIndex(rosterlist, "ROSTER_DATE"), Inputs[CHANGE_DATE]);
Out(db.tab(rosterlist), hdbc, 200, NULL, NULL, -1, "USERID.ROSTERLIST");
finish = (double)clock();
finish = finish/CLOCKS_PER_SEC;
db.DropTable(rosterlist);
db.DropTable(tptab);
db.PrintStats(fp);

for(loop = 0;loop < 3;loop++)
   free(decsi[loop]);

fprintf(fp, "Size of Row: %d\nSize of Column: %d\nSize of Table: %d\nSize of Database: %d\nFindIndex iterations: %d\n",sizeof(Row), sizeof(Columns), sizeof(Table), sizeof(database), IndexCount);
fprintf(fp, "qsort/bsearch Integer Comparisons: %d\nqsort/bsearch String Comparisons: %d\nNULLstrcmp Calls: %d\nstrlenBytes Calls %d\n", IntegerComparisons, StringComparisons, NULLstrcmCmp, strlenBytesCnt);

db.PrintStats(fp);
_fcloseall();
/*-----------------------------------------------------------------*
 * Return to caller                                                *
 *-----------------------------------------------------------------*/

ext:
   rc = SQLEndTran( SQL_HANDLE_DBC, hdbc, SQL_COMMIT ) ;
   CHECK_HANDLE( SQL_HANDLE_DBC, hdbc, rc ) ;

   rc = SQLDisconnect( hdbc ) ;
   CHECK_HANDLE( SQL_HANDLE_DBC, hdbc, rc ) ;

   rc = SQLFreeHandle( SQL_HANDLE_DBC, hdbc ) ;
   CHECK_HANDLE( SQL_HANDLE_DBC, hdbc, rc ) ;

   rc = SQLFreeHandle( SQL_HANDLE_ENV,  henv ) ;
   if ( rc != SQL_SUCCESS )
      return( terminate( henv, rc ) ) ;

   return 1;
}

int BCProc(Table *tb, int row)
{
int iloop, rostrows, seniority;
char *val, *tp, decn, onlnull;
static int reg, senior, rank, dec, onleave, todrank, cal, tf = 0, bctot;

rostrows = tb->rows;

if(tf == 0)
   {
   reg = tb->ColIndex("REGNO"); //13
   todrank = tb->ColIndex("TODAYRANK"); //16
   rank = tb->ColIndex("RANK"); //9
   onleave = tb->ColIndex("ONLEAVE");
   senior = tb->ColIndex("SENIORITY");
   dec = tb->ColIndex("DECLINED");
   cal = tb->ColIndex("CALLIN");
   tf = 1;
   //Check if a BC already exists
   for(iloop = 0;iloop < rostrows;iloop++)
      {
      if(((NULLstrcmp(tb->GetData(iloop, rank), "B/C") == 0) || (NULLstrcmp(tb->GetData(iloop, todrank), "B/C") == 0)) && (NULLstrcmp(tb->GetData(iloop, onleave), NULL) == 0) && (NULLstrcmp(tb->GetData(iloop, cal), "N") == 0))
         {
         bctot = 1;
         break;
         }
      }
   }

val = Coalesce(tb->GetData(row, senior), tb->GetData(row, reg));
if(val == NULL || bctot == 1) //Catch those nasty Station rows and any other get the hell out of here conditions
   return 0;
seniority = atoi(val);

if(bctot == 0) //We don't have one
   { //Now get how many people are more senior
   for(iloop = 0;iloop < rostrows;iloop++)
      {
      tp = Coalesce(tb->GetData(iloop, senior), tb->GetData(iloop, reg));
      decn = NULLstrcmp(tb->GetData(iloop, dec), "N");
      onlnull = NULLstrcmp(tb->GetData(iloop, onleave), NULL);
      if(tp == NULL) //same thing
         continue;
      if((seniority > atoi(tp)) && decn == 0 && onlnull == 0) //Somebody with higher senior was found
         break;
      }
   if(iloop == rostrows) //No-one was found, so this guy is the Grand Poobah
      return 1;
   else //Go on to acting district chief processing
      return 0;
   }
else
   return 0;
}

int DCProc(Table *tb, int row, int sor)
{
int iloop, loop, rostrows, sentot = 0, seniority, ms = 0;
char *val, *tp;
int decn, onlnull, highrank, onlstar;
static int reg, senior, rank, dec, onleave, todrank, cal, tf = 0, dctot = 0;

rostrows = tb->rows;

if(tf == 0) //One time processing stuff
   {
   reg = tb->ColIndex("REGNO"); //13
   todrank = tb->ColIndex("TODAYRANK"); //16
   rank = tb->ColIndex("RANK"); //9
   onleave = tb->ColIndex("ONLEAVE");
   senior = tb->ColIndex("SENIORITY");
   dec = tb->ColIndex("DECLINED");
   cal = tb->ColIndex("CALLIN");
   tf = 1;
   //Count how many D/c's there are
   for(iloop = 0;iloop < rostrows;iloop++)
      {
      if(((NULLstrcmp(tb->GetData(iloop, rank), "B/C") == 0) || (NULLstrcmp(tb->GetData(iloop, rank), "B/C") == 0) || (NULLstrcmp(tb->GetData(iloop, rank), "D/C") == 0) || (NULLstrcmp(tb->GetData(todrank, iloop), "D/C") == 0)) && (NULLstrcmp(tb->GetData(iloop, onleave), NULL) == 0) && (NULLstrcmp(tb->GetData(iloop, cal), "N") == 0))
         {
         if(dctot < sor)
            dctot++;
         else
            break;
         }
      }
   }

val = Coalesce(tb->GetData(row, senior), tb->GetData(row, reg));
if(val == NULL) //Catch those nasty Station rows
   return 0;
seniority = atoi(val);
if(!((NULLstrcmp(tb->GetData(row, rank), "CP") == 0) || (NULLstrcmp(tb->GetData(row, rank), "LT") == 0)))
   {
   return 0; //he's not of the right rank for this job
   }

if(dctot < sor) //We don't have one
   { //Now get how many people are more senior
   for(iloop = 0;iloop < rostrows;iloop++)
      {
      tp = Coalesce(tb->GetData(iloop, senior), tb->GetData(iloop, reg));
      if(tp == NULL || iloop == row) //same thing
         continue;
      decn = NULLstrcmp(tb->GetData(iloop, dec), "N") == 0;
      onlnull = NULLstrcmp(tb->GetData(iloop, onleave), NULL) == 0;
      onlstar = NULLstrcmp(tb->GetData(iloop, onleave), "*") == 0;
      highrank = (NULLstrcmp(tb->GetData(iloop, rank), "B/C") == 0) || (NULLstrcmp(tb->GetData(iloop, rank), "D/C") == 0);
      if((seniority > atoi(tp) || highrank) && (decn || highrank) && (onlnull || onlstar)) //Somebody with higher senior was found
         {
         if(sentot < sor)
            sentot++;
         else
            break;
         }
      }
   if(sentot < sor) //He's high enough up to get upgraded
      {
      return 1;
      }
   else //Go on to Cap'n processing
      {
      return 0;
      }
   }
else
   {
   return 0;
   }
}

int CPProc(Table *tb, int row, int cpso)
{
int iloop, rostrows, sentot = 0, seniority, ms = 0;
char *val, *tp;
int decn, onlstar, onlnull, highrank;
static int reg, senior, rank, dec, onleave, todrank, cal, tf = 0, cptot = 0;
rostrows = tb->rows;

if(tf == 0)
   {
   reg = tb->ColIndex("REGNO"); //13
   todrank = tb->ColIndex("TODAYRANK"); //16
   rank = tb->ColIndex("RANK"); //9
   onleave = tb->ColIndex("ONLEAVE");
   senior = tb->ColIndex("SENIORITY");
   dec = tb->ColIndex("DECLINED");
   cal = tb->ColIndex("CALLIN");
   //Count how many capn's there are
   for(iloop = 0;iloop < rostrows;iloop++)
      {
      if(((NULLstrcmp(tb->GetData(iloop, rank), "B/C") == 0) || (NULLstrcmp(tb->GetData(iloop, todrank), "B/C") == 0) || (NULLstrcmp(tb->GetData(iloop, rank), "D/C") == 0) || (NULLstrcmp(tb->GetData(iloop, todrank), "D/C") == 0) || (NULLstrcmp(tb->GetData(iloop, rank), "CP") == 0) || (NULLstrcmp(tb->GetData(iloop, todrank), "CP") == 0)) && (NULLstrcmp(tb->GetData(iloop, onleave), NULL) == 0) && (NULLstrcmp(tb->GetData(iloop, cal), "N") == 0))
         {
         if(cptot < cpso)
            cptot++;
         else
            break;
         }
      }
   tf = 1;
   }

val = Coalesce(tb->GetData(row, senior), tb->GetData(row, reg));
if(val == NULL) //Catch those nasty Station rows
   return 0;
seniority = atoi(val);
if((NULLstrcmp(tb->GetData(row, rank), "sFF") == 0) || (NULLstrcmp(tb->GetData(row, rank), "LT") == 0))
   highrank = 1;
else
   return 0; //he's not of the right rank for this job

if(cptot < cpso) //We don't have one
   { //Now get how many people are more senior
   for(iloop = 0;iloop < rostrows;iloop++)
      {
      tp = Coalesce(tb->GetData(iloop, senior), tb->GetData(iloop, reg));
      if(tp == NULL || iloop == row) //same thing
         continue;
      decn = NULLstrcmp(tb->GetData(iloop, dec), "N") == 0;
      onlnull = NULLstrcmp(tb->GetData(iloop, onleave), NULL) == 0;
      onlstar = NULLstrcmp(tb->GetData(iloop, onleave), "*") == 0;
      highrank = (NULLstrcmp(tb->GetData(iloop, rank), "B/C") == 0) || (NULLstrcmp(tb->GetData(iloop, rank), "D/C") == 0) || (NULLstrcmp(tb->GetData(iloop, rank), "CP") == 0);
      if((seniority > atoi(tp) || highrank) && (decn || highrank) && (onlnull || onlstar)) //Somebody with higher senior was found
         {
         if(sentot < cpso)
            sentot++;
         else
            break;
         }
      }
   if(iloop == rostrows && sentot < cpso) //No-one was found, so this guy gets to be upgraded
      {
      return 1;
      }
   else //Go on to Cap'n processing
      {
      return 0;
      }
   }
else
   return 0;
}

int LTProc(Table *tb, int row, int toto)
{
int iloop, rostrows, sentot = 0, seniority, ms = 0;
char *val, *tp;
int decn, onlstar, onlnull, highrank;
static int reg, senior, rank, dec, onleave, todrank, cal, tf = 0, lttot;
rostrows = tb->rows;

if(tf == 0)
   {
   reg = tb->ColIndex("REGNO"); //13
   todrank = tb->ColIndex("TODAYRANK"); //16
   rank = tb->ColIndex("RANK"); //9
   onleave = tb->ColIndex("ONLEAVE");
   senior = tb->ColIndex("SENIORITY");
   dec = tb->ColIndex("DECLINED");
   cal = tb->ColIndex("CALLIN");
   //Count how many D/c's there are
   for(iloop = 0;iloop < rostrows;iloop++)
      {
      if(((NULLstrcmp(tb->GetData(iloop, rank), "B/C") == 0) || (NULLstrcmp(tb->GetData(iloop, todrank), "B/C") == 0) || (NULLstrcmp(tb->GetData(iloop, rank), "D/C") == 0) || (NULLstrcmp(tb->GetData(iloop, todrank), "D/C") == 0) || (NULLstrcmp(tb->GetData(iloop, rank), "CP") == 0) || (NULLstrcmp(tb->GetData(iloop, todrank), "CP") == 0)) && (NULLstrcmp(tb->GetData(iloop, onleave), NULL) == 0) && (NULLstrcmp(tb->GetData(iloop, cal), "N") == 0))
         {
         if(lttot < toto)
            lttot++;
         else
            break;
         }
      }
   tf = 1;
   }

val = Coalesce(tb->GetData(row, senior), tb->GetData(row, reg));
if(val == NULL) //Catch those nasty Station rows
   return 0;
seniority = atoi(val);
if(NULLstrcmp(tb->GetData(row, rank), "sFF") != 0)
   return 0; //he's not of the right rank for this job

if(lttot < toto) //We don't have one
   { //Now get how many people are more senior
   for(iloop = 0;iloop < rostrows;iloop++)
      {
      tp = Coalesce(tb->GetData(iloop, senior), tb->GetData(iloop, reg));
      if(tp == NULL || iloop == row) //same thing
         continue;
      decn = NULLstrcmp(tb->GetData(iloop, dec), "N") == 0;
      onlnull = NULLstrcmp(tb->GetData(iloop, onleave), NULL) == 0;
      onlstar = NULLstrcmp(tb->GetData(iloop, onleave), "*") == 0;
      highrank = (NULLstrcmp(tb->GetData(iloop, rank), "B/C") == 0) || (NULLstrcmp(tb->GetData(iloop, rank), "D/C") == 0);
      highrank = highrank || (NULLstrcmp(tb->GetData(iloop, rank), "CP") == 0);
      highrank = highrank || (NULLstrcmp(tb->GetData(iloop, rank), "LT") == 0);
      if((seniority > atoi(tp) || highrank) && (decn || highrank) && (onlnull || onlstar)) //Somebody with higher senior was found
         {
         if(sentot < toto)
            sentot++;
         else
            break;
         }
      }
   if(iloop == rostrows && sentot < toto) //No-one was found, so this guy gets to be upgraded
      {
      return 1;
      }
   else //Go on to Cap'n processing
      {
      return 0;
      }
   }
else
   return 0;
}
