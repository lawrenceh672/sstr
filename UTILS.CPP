#include "sstr.h"
#include "rowcol.h"

int sortcol;
int IntegerComparisons = 0;
int StringComparisons = 0;
int NULLstrcmCmp = 0;
int strlenBytesCnt = 0;

int strlenBytes(const char *string)
{
int loop = 0;
strlenBytesCnt++;
if(string == NULL)
   return 0;

return strlen(string)+1; //For the null char
}

void FreeCharPP(char **inp, int count)
{
int loop;
if(inp == NULL)
   return;

for(loop = 0; loop < count; loop++)
   {
   free(inp[loop]);
   }
free(inp);
}

int AllNumbers(char *inp)
{
int loop = 0;
if(inp == NULL)
   return 0;

while(inp[loop] != 0)
   {
   if(inp[loop] < 48 || inp[loop] > 58)
      return 0; //Fail on the first non number character
   loop++;
   }
return 1;
}

int _Optlink cmpstrSort(const void *key, const void *element)
{
Row *keyR, *eleR;
keyR = *(Row **)key; //Get them into the correct form first
eleR = *(Row **)element;

StringComparisons++;

return ( NULLstrcmp(keyR->data[sortcol], eleR->data[sortcol]) );
}

int _Optlink cmpsitSort(const void *key, const void *element)
{
Row *keyR, *eleR;
int ks, es;
unsigned char kn, en;

keyR = *(Row **)key; //Get them into the correct form first
eleR = *(Row **)element;
kn = keyR->data[sortcol] == NULL;
en = eleR->data[sortcol] == NULL;

IntegerComparisons++;

if(kn && en)
   return 0;
if(kn)
   return -1;
if(en)
   return 1;

ks = atoi(keyR->data[sortcol]);
es = atoi(eleR->data[sortcol]);

if(ks < es)
   return -1;
if(ks > es)
   return 1;
if(ks == es)
   return 0;
}

int _Optlink cmpstrBs(const void *key, const void *element)
{
Row *eleR;
char *keyR;

StringComparisons++;

keyR = ((char *)key); //Get them into the correct form first
eleR = *(Row **)element;
return ( NULLstrcmp(keyR, eleR->data[sortcol]) );
}

int _Optlink cmpsitBs(const void *key, const void *element)
{
Row *eleR;
char *keyR;
int ks, es;
unsigned char kn, en;

IntegerComparisons++;

keyR = ((char *)key); //Get them into the correct form first
eleR = *(Row **)element;
kn = keyR == NULL;
en = eleR->data[sortcol] == NULL;

if(kn && en)
   return 0;
if(kn)
   return -1;
if(en)
   return 1;

ks = atoi(keyR);
es = atoi(eleR->data[sortcol]);

if(ks < es)
   return -1;
if(ks > es)
   return 1;
if(ks == es)
   return 0;
}

int isBlank(char *inp)
{
int loop = 0;
if(inp == NULL) //NULL is not blank
   return 0;

while(inp[loop] != 0)
   {
   if(inp[loop] != ' ')
      return 0;
   loop++;
   }
return 1; //Must be all spaces
}

char *Coalesce(char *arg1, char *arg2)
{
//The first argument not to be null
if(arg1 != NULL)
   return arg1;
return arg2;
}

int _Optlink cmpstrCols(const void *key, const void *element)
{
Columns *keyR, *eleR;

StringComparisons++;

keyR = (Columns *)key; //Get them into the correct form first
eleR = (Columns *)element;
return ( NULLstrcmp(keyR->name, eleR->name) );
}

int _Optlink cmpstrColsBs(const void *key, const void *element)
{
Columns *eleR;
char *keyR;

StringComparisons++;

keyR = ((char *)key); //Get them into the correct form first
eleR = (Columns *)element;

return ( NULLstrcmp(keyR, eleR->name) );
}

char *HV(char *stmt, char **HV, char **replace, int vars)
{
char *loc, temp[32767], *cur, iloop, *ret;
int loop, length;

loc = stmt;
cur = stmt;
temp[0] = 0;
length = strlen(temp);
ret = (char *)malloc(sizeof(SQLCHAR) * (length + 1)); //copy the initial string over
memcpy(ret, temp, length + 1);

for(loop = 0;loop < vars; loop++)
   {
   while((loc = strstr(loc, HV[loop])) != NULL) //Get em all
      {
      strncat(temp, cur, loc - cur); //copy over the amount of string passed over
      strcat(temp, replace[loop]); //tack on the replacement thing
      loc = loc + strlen(HV[loop]); //start looking again after the found substring
      cur = loc;
      }
   //copy over the remaining string to the buffer
   strcat(temp, cur);
   free(ret); //Get ready for the next replacement
   ret = (char *)malloc(sizeof(SQLCHAR) * strlen(temp) + 1); //for the null term
   strcpy(ret, temp);
   temp[0] = 0;
   loc = ret;
   cur = ret;
   }
return ret;
}

char *Replacer(char *stmt, const char *HV, const char *replace)
{
char *loc, temp[32767], *cur, iloop, *ret;
int loop, length;

loc = stmt;
cur = stmt;
temp[0] = 0;
ret = (char *)malloc(sizeof(SQLCHAR) * (length + 1)); //copy the initial string over
memcpy(ret, temp, length + 1);

while((loc = strstr(loc, HV)) != NULL) //Get em all
   {
   strncat(temp, cur, loc - cur); //copy over the amount of string passed over
   strcat(temp, replace); //tack on the replacement thing
   loc = loc + strlen(HV); //start looking again after the found substring
   cur = loc;
   }
//copy over the remaining string to the buffer
strcat(temp, cur);
return ret;
}


int ReadInFile(char *buf, const char *path)
{
FILE *inp;
int loop = 0;

if( (inp = fopen(path, "r")) == NULL)
   return -1;

while(!feof(inp))
   {
   buf[loop] = (char)fgetc(inp);
   loop++;
   }
buf[loop - 1] = 0; //Take off the garbage characater at the end.  Whatever it was buggering it up, maybe it was the eof char.  I never looked, just guessed.  And this made it work.  SO there.  Exhaustive testing I don't do.... :->
return 1;
}

int IGstrcmp(const char *str1, const char *str2)
{
unsigned char s1n, s2n;
s1n = str1 == NULL;    //I'd rather compare unsigned chars than 4 byte pointers
s2n = str2 == NULL;

if(s1n && s2n)
   return 0;
if(s1n)
   return -1;
if(s2n)
   return 1;
return strcmp(str1, str2); //Now it won't bail on null inputs
}

int NULLstrcmp(const char *str1, const char *str2)
{
unsigned char s1n, s2n;
char ts1[256], ts2[256];
s1n = str1 == NULL;    //I'd rather compare unsigned chars than 4 byte pointers
s2n = str2 == NULL;

NULLstrcmCmp++;

if(s1n && s2n)
   return 0;
if(s1n)
   return -1;
if(s2n)
   return 1;
strcpy(ts1, str1);
strcpy(ts2, str2);
rtrim(ts1, ' ');
ltrim(ts1, ' ');
rtrim(ts2, ' ');
ltrim(ts2, ' ');
return strcmp(ts1, ts2); //Now it won't bail on null inputs
}


unsigned int UniqueInt()
{
static unsigned int ret = 0;
ret++;
return ret;
}

void SetFlag(int index, unsigned char *arr, unsigned char inp)
{
if(inp == 1)
   arr[index >> 3] += (1 << index % 8);
else
   arr[index >> 3] -= (1 << index % 8);
}

unsigned char GetFlag(int index, unsigned char *arr)
{
unsigned char tp;
tp = index % 8;
return (arr[index >> 3] & (1 << tp)) >> tp;
}

void ltrim(char *inp, char delim)
{
int loop = 0, len;

if(inp == NULL)
   return;

len = strlen(inp);

while(inp[loop] == delim && loop < len)
   loop++;

strcpy(inp, &inp[loop]);
}

void rtrim(char *inp, char delim)
{
int loop;

if(inp == NULL)
   return;

loop = strlen(inp);

while(inp[loop - 1] == delim && loop > 0)
   loop--;

inp[loop] = 0;
}

