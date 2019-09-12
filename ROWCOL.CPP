#include "sstr.h"
#include "rowcol.h"

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

char *Row::GetData(int col)
{
return data[col];
}

int Columns::isEqualTo(Columns *cs)
{
/*if(this->colsize != cs->colsize) No need to worry about that for now
   return 0;
if(this->type != cs->type)
   return 0;
if(this->precision != cs->precision)
   return 0;*/
if(NULLstrcmp(this->name, cs->name) != 0)
   return 0;
return 1;
}

