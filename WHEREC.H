#include "sstr.h"

#ifndef WHERECLAUSE
typedef struct wcinfo
{
int col; //column index
int oper; //=, !=
int logic; //and, or
char *val; //value needed for this column
int eval; //Which evaluation set it belongs to
};


class WhereClause
{
public:
WhereClause();
~WhereClause();
void AddTo(int col, const char *val, int oper, int logic, int eval);
int GetSet(int set);

wcinfo *wci;
int clauses;
//int evals;
int *evalsets;
int esets;
int TableIndex;
};
#define WHERECLAUSE
#endif

