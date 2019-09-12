#include "sstr.h"
#include "key.h"

PrimaryKey::PrimaryKey()
{
key = NULL;
nck = 0;
}

PrimaryKey::~PrimaryKey()
{
free(key);
}

void PrimaryKey::AddToKey(Columns *cs)
{
key = (Columns *)realloc(key, (nck + 1) * sizeof(Columns));
memcpy(&key[nck], cs, sizeof(Columns));
nck++;
}

int PrimaryKey::isEqualTo(PrimaryKey *pk)
{
int loop, pkc, iloop;
Columns *data;

data = pk->GetCol(&pkc);
if(pkc != nck) //they have a different number of columns in their primary keys
   return 0;

for(loop = 0;loop < nck;loop++)
   {
   for(iloop = 0;iloop < pkc;iloop++)
      {
      if(!key[loop].isEqualTo(&data[iloop]))
         {
         free(data);
         return 0;
         }
      }
   }
return 1;
}

Columns *PrimaryKey::GetCol(int *ret)
{
*ret = nck;
return key;
}

int PrimaryKey::BytesUsed()
{
return _msize(key);
}

ForeignKey::ForeignKey(PrimaryKey *pk)
{
nck = pk->nck;
this->key = (Columns *)malloc(sizeof(Columns) * nck);
memcpy(this->key, pk->key, nck * sizeof(Columns));
mtchcol = NULL;
}

ForeignKey::ForeignKey()
{
key = NULL;
nck = 0;
mtchcol = NULL;
}

ForeignKey::~ForeignKey()
{
free(mtchcol);
}

