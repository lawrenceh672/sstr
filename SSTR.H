#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <sqlda.h>
#include <sqlcli1.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAXCOLS 255
#define DEBUG 1
#define LEAVEYEAR 0
#define PLATOON 1
#define CHANGE_DATE 2
#define TOUR_START 3
#define SELECT 4
#define HVCOUNT 5
#define WILDCARD -3
#define getrandom( min, max ) ((rand() % (int)(((max)+1) - (min))) + (min))

#define APPEND 6
#define NEW 7

#define ROWS 8
#define COLS 9
#define TABS 10
#define SQLTYPE 11
#define SQLPREC 12
#define SQLCOLSIZE 13
#define TABLE_MODIFIED 0
#define TABLE_INVALID_DATA 1
#define ROW_OP_AND 14
#define ROW_OP_OR 15
#define ROW_NO_OP 16
#define EQUALS 17
#define NOT_EQUALS 18
#define LESS_THAN 19
#define GREATER_THAN 20
#define ROWSET_BAD_INDEX -2

extern int sortcol; //for the row compare functions
extern const char *TabCmpSameName;
extern const char *TabCmpDiffName;
extern int IntegerComparisons;
extern int StringComparisons;
extern int NULLstrcmCmp;
extern int IndexCount;
extern int strlenBytesCnt;

//Utils.cpp functions
extern int strlenBytes(const char *string);
extern void FreeCharPP(char **inp, int count);
extern int AllNumbers(char *inp);
extern int isBlank(char *inp);
extern char *Coalesce(char *arg1, char *arg2);
extern char *HV(char *stmt, char **HV, char **replace, int vars);
extern int ReadInFile(char *buf, const char *path);
extern int NULLstrcmp(const char *str1, const char *str2);
extern unsigned int UniqueInt();

extern int _Optlink cmpstrSort(const void *arg1, const void *arg2); //compare Row data based on sortcol global
extern int _Optlink cmpsitSort(const void *key, const void *element); //converts strings to integers

extern int _Optlink cmpstrBs(const void *arg1, const void *arg2); //Compare Row data for the binary search on sortcol global
extern int _Optlink cmpsitBs(const void *key, const void *element); //converts strings to integers

extern int _Optlink cmpstrCols(const void *key, const void *element); //Compare names in the Columns structure
extern int _Optlink cmpstrColsBs(const void *key, const void *element); //Compare names in the Columns structure for the binary search

extern void SetFlag(int index, unsigned char *arr, unsigned char inp);
extern unsigned char GetFlag(int index, unsigned char *arr);
extern void ltrim(char *inp, char delim);
extern void rtrim(char *inp, char delim);
extern int IGstrcmp(const char *str1, const char *str2);
extern char *Replacer(char *stmt, const char *HV, const char *replace);
