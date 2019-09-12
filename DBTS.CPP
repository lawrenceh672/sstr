#include <stdio.h>
#include <iostream.h>
#include <malloc.h>
#include <conio.h>
#include <ctype.h>

#include "sstr.h"  //The application side db stuff

Table *CreateTable();
void ListColumns(database *db);
void Dump(database *db);
void Add(database *db);
int Options();
void Spec(database *db);

main()
{
database db;
Table *tab;
int sel = 0;
char *c1 = "Col1    ", *c2 = "Col2    ", *c3 = "Col3      ", *name = "HooYah!";


tab = new Table;

tab->AddColumn(c1, 8);
tab->AddColumn(c2, 7);
tab->AddColumn(c3, 10);

db.AddTable(tab);

cout << "Database mirror program" << endl;
while((sel = Options()) != 666)
   {
   cout << "Enter Selection:" << endl;
   switch (sel)
      {
      case 1:
         db.AddTable(CreateTable());
         break;
      case 3:
         ListColumns(&db);
         break;
      case 5:
         Add(&db);
         break;
      case 7:
         Dump(&db);
         break;
      case 9:
         Spec(&db);
         break;
      default:
         cout << "Huh?" << endl;
         break;
      }
   sel = 0;
   }
}

int Options()
{
int tp;

cout << "1. Create Table" << endl;
cout << "3. List columns in table" << endl;
cout << "5. Enter data in table" << endl;
cout << "7. Output all data in table" << endl;
cout << "9. Get Specific data from a table" << endl;
cout << "666. Quit" << endl;
cin >> tp;

return tp;
}

Table *CreateTable()
{
int tp, loop;
char *name;
Table *tab;

name = (char *)calloc(2, 16);

cout << "So, Like Y'Know...How many columns?" << endl;
cin >> tp;
tab = new Table();

for(loop = 0;loop < tp;loop++)
   {
   cout << "Enter column " << loop << " name: ";
   cin >> name;
   tab->AddColumn(name, StringSize(name));
   }

return tab;
}

void ListColumns(database *db)
{
int loop = 0, tp;

cout << "Ok...Which table?" << endl;
for(loop = 0; loop < db->Tables;loop++)
   cout << loop << ". Table " << loop << endl;

cin >> tp;
if(tp < 0 || tp > db->Tables)
   {
   cout << "Don't be an idiot." << endl;
   ListColumns(db);
   return;
   }
else
   {
   for(loop = 0;loop < db->tab(tp)->columns;loop++)
      cout << "Column " << loop << " name: " << db->tab(tp)->cols[loop] << endl;
   }
}

void Dump(database *db)
{
int tp, loop, iloop;
Table *tab;


cout << "You want the data?  You CAN'T handle the data..." << endl;
for(loop = 0; loop < db->Tables;loop++)
   cout << loop << ". Table " << loop << endl;

cout << "...alright, which table?" << endl;
cin >> tp;

if(tp < 0 || tp > db->Tables)
   {
   cout << "You think your funny eh?" << endl;
   Dump(db);
   return;
   }
else
   {
   tab = db->tab(tp);
   for(loop = 0;loop < tab->columns;loop++)
      {
      printf( "%-*.*s",tab->colsize[loop],tab->colsize[loop],tab->cols[loop]);
      }
   printf("\n");
   for(iloop = 0;iloop < tab->rows;iloop++)
      {
      for(loop = 0;loop < tab->columns;loop++)
         {
         printf( "%-*.*s",tab->colsize[loop],tab->colsize[loop],tab->rowdata[iloop]->data[loop]);
         }
      printf("\n");
      }
   }
}

void Add(database *db)
{
int loop, tp, rws, iloop, Done = 0, ch, counter = 0;
Table *tab;
char input[700];

memset(input, 0, 700);

for(loop = 0; loop < db->Tables;loop++)
   cout << loop << ". Table " << loop << endl;

cout << "Okay, buddy boy, take your best shot" << endl;
cin >> tp;

if(tp < 0 || tp > db->Tables)
   {
   cout << "Oh, ho, oh boy...your GONNA DIE FOR THAT...." << endl;
   Add(db);
   return;
   }
else
   {
   cout << "K, so far so good...how many rows?" << endl;
   cin >> rws;
   tab = db->tab(tp);
   for(iloop = 0;iloop < rws;iloop++)
      {
      tab->NewRow();
      for(loop = 0;loop < tab->columns;loop++)
         {
         cout << "Row " << iloop << ", Column " << loop << ": " << tab->colsize[loop] << endl;
         do
            {
            ch = _getch();
            _putch(ch);
            switch (ch)
               {
               case 13:
                  Done = 1;
                  break;
               case 8:
                  if(counter != 0)
                     {
                     counter--;
                     input[counter] = 0;
                     _putch(32); //Space
                     _putch(8);
                     }
                  else
                     _putch(input[counter]);
                  break;
               default:
                  input[counter] = ((char)tolower(ch));
                  counter++;
                  if(counter == tab->colsize[loop]) //Can't be longer than the db will accept
                     Done = 1;
                  break;
               }
            }
         while(!Done);
         tab->AddData(input, tab->colsize[loop], loop);
         Done = 0;
         counter = 0;
         cout << endl;
         }
      }
   }
}

void Spec(database *db)
{
int tp, loop, row, col;
char *ans;
Table *tab;

for(loop = 0; loop < db->Tables;loop++)
   cout << loop << ". Table " << loop << endl;

cout << "Waddya have?" << endl;
cin >> tp;

if(tp < 0 || tp > db->Tables)
   {
   cout << "Your ARE so dumb!  I mean *really*. Option 0 is table 0 etc... I mean, how hard is it????" << endl;
   Spec(db);
   return;
   }
else
   {
   tab = db->tab(tp);
   cout << "Lets have the row, then the column" << endl;
   cin >> row >> col;
   ans = tab->GetData(row, col);
   cout << ans << endl;
   }
}
