#include "sstr.h"
#include "table.h"
#include "dbutils.h"
#include "database.h"

extern int RunQuery(SQLHANDLE hdbc, const char *stmt, Table *tab, int mode, FILE *fp);
extern void Out(Table *tab, SQLHANDLE hdbc, int inserts, int *incols, char **outcols, int count, const char *dbtab);
extern char *Format(char *inp, int sqltype);
extern int ReadInSQL(char *buf, const char *path);
extern int CreateTable(SQLHANDLE hdbc, Table *tab, const char *name, const char *tabspace);
extern double UserInp(SQLHANDLE hdbc, const char *title, database *db);
extern void FileMan(FILE **flp, char **fn);
