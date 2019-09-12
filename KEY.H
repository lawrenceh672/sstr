#include "sstr.h"
#include "rowcol.h"

#ifndef KEYCLASS
class PrimaryKey
{
public:
PrimaryKey();
~PrimaryKey();
void AddToKey(Columns *cs);
Columns *GetCol(int *ret); //Return the number of columns in the key
int isEqualTo(PrimaryKey *pk);
int BytesUsed();
int Mallocs() {return 1;}

int nck; //number columns key
Columns *key; // A description of the columns
};

class ForeignKey : public PrimaryKey
{
public:
ForeignKey(PrimaryKey *pk);
ForeignKey();
~ForeignKey();
void *Foreign; //the handle of the foreign table
int *mtchcol; //the corresponding column of the parent table
};
#define KEYCLASS
#endif

