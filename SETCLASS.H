#include "sstr.h"

class Table;

#ifndef ROWSET
class RowSet
{
public:
RowSet(int rows, int size, Table *orig);
~RowSet();
void FlagRow(int row);
void UnFlagRow(int row);
int RowFlag(int row);
void And(RowSet *rs);
void Or(RowSet *rs);
void UnFlagAll() {memset(rowflags, 0, (setsize >> 3) + 1); flagged = 0;}
void FlagAll() {memset(rowflags, 255, (setsize >> 3) + 1); flagged = setsize;}
int isEqualTo(RowSet *rs);
void Resize(int row, int size);
void Copy(RowSet *rs);
void Compress(); //puts all the table rows at the beginning.

int rowindex;
int setsize;
Table *source;
int flagged;
protected:
unsigned char *rowflags;
};
#define ROWSET
#endif

