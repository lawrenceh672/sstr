#include "setclass.h"
#include "table.h"

RowSet::RowSet(int row, int size, Table *orig)
{
rowflags = (unsigned char *)calloc(1, ((size >> 3) + 1) * sizeof(unsigned char));
rowindex = row;
setsize = size;
source = orig;
flagged = 0;
}

RowSet::~RowSet()
{
free(rowflags);
}

void RowSet::FlagRow(int row)
{
int offset;

offset = row - rowindex;

if(offset < 0 || offset > setsize)
   return;
else
   SetFlag(row - rowindex, rowflags, 1);
flagged++;
}

void RowSet::UnFlagRow(int row)
{
int offset;

offset = row - rowindex;
if(offset < 0 || offset > setsize)
   return;
else
   SetFlag(row - rowindex, rowflags, 0);
flagged--;
}

int RowSet::RowFlag(int row)
{
int offset;

offset = row - rowindex;
if(offset < 0 || offset > setsize)
   return ROWSET_BAD_INDEX;
else
   return GetFlag(offset, rowflags);
}


void RowSet::Resize(int row, int size)
{
unsigned char *tp;
int loop, lb, ub;

lb = min(rowindex, row);   //switch min/max on whether you want to limit the new set to an intersection or a union
ub = max(rowindex + setsize, row + size);

tp = (unsigned char *)calloc(1, (((ub-lb) >> 3) + 1) * sizeof(unsigned char *));

for(loop = lb;loop < ub;loop++)
   {
   if(RowFlag(loop) == 1)
      SetFlag(loop - lb,tp, 1);
   }
free(rowflags);
rowflags = tp;
rowindex = lb;
setsize = ub - lb;
}

void RowSet::And(RowSet *rs)
{
int loop;

if(!isEqualTo(rs))
   return;

Resize(rs->rowindex, rs->setsize);

for(loop = rowindex;loop < rowindex + setsize;loop++)
   {
   if(!rs->RowFlag(loop))
      {
      if(GetFlag(loop - rowindex, rowflags)) //changed from 1 to 0 cuz inp is 0
         {
         SetFlag(loop - rowindex, rowflags, 0);
         flagged--;
         }
      } //else nuthin.  Since the rs is flagged, this will be left the same no matter what
   }
}

void RowSet::Or(RowSet *rs)
{
int loop;

if(!isEqualTo(rs))
   return;

Resize(rs->rowindex, rs->setsize);

for(loop = rowindex;loop < rowindex + setsize;loop++)
   {
   if(rs->RowFlag(loop)) //if its one we just need to know if its set in the source set
      {
      if(!GetFlag(loop - rowindex, rowflags)) //from 0 to 1
         {
         SetFlag(loop - rowindex, rowflags, 1); //it wasn't set before
         flagged++;
         }
      } //else nuthin cuz if its 1, it'll stay one.  if its zero it'll stay zero.  Nothing changes
   }
}

int RowSet::isEqualTo(RowSet *rs)
{
if(source == rs->source)
   return 1;
else
   return 0;
}

void RowSet::Copy(RowSet *rs)
{
Resize(rs->rowindex, rs->setsize);
memcpy(rowflags, rs->rowflags, (setsize >> 3) + 1);
flagged = rs->flagged;
}

void RowSet::Compress()
{
int loop;

for(loop = rowindex;loop < rowindex + setsize;loop++)
   {
   if(RowFlag(loop))
      {
      source->SwapRows(loop - rowindex, loop); //Change the table structure
      SetFlag(loop - rowindex, rowflags, 1); //Modify the flag array accordingly
      SetFlag(loop, rowflags, 0);
      }
   }
}
