#include "sstr.h"

#ifndef ROWCOL
class Columns
{
public:
int isEqualTo(Columns *cs);

char *name;
int col;
int colsize;
int type;
int precision;
//unsigned char IsNullable;  --FUTURE--
};


class Row
{
public:
Row(int cols);
~Row();
char *GetData(int col);

char **data;
};
#define ROWCOL
#endif
