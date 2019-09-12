#include "sstr.h"
#include "wherec.h"

WhereClause::WhereClause()
{
wci = NULL;
clauses = 0;
evalsets = NULL;
esets = 0;
}

WhereClause::~WhereClause()
{
free(wci);
free(evalsets);
}

int WhereClause::GetSet(int index)
{
int loop = 0, retind = 0;

while(wci[loop].eval != index)
   loop++;

return loop;
}

void WhereClause::AddTo(int col, const char *val, int oper, int logic, int eval)
{
int bytes;
static int lasteval = -1; //This clause is part of the current evaluation set

bytes = strlenBytes(((char *)val));

wci = (wcinfo *)realloc(wci, (clauses + 1) * sizeof(wcinfo));

wci[clauses].val = (char *)malloc(bytes);

memcpy(wci[clauses].val, val, bytes);
wci[clauses].col = col;
wci[clauses].oper = oper;
wci[clauses].logic = logic;
wci[clauses].eval = eval;

clauses++;
if(eval != lasteval) //each index in evalsets holds the number of clauses in each of the evaluation sets
   {
   esets++;
   evalsets = (int *)realloc(evalsets, sizeof(int) * esets);
   evalsets[esets - 1] = 1;
   lasteval = esets - 1;
   }
else
   {
   evalsets[esets - 1]++;
   lasteval = esets - 1;
   }
}
