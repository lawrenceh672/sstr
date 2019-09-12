# makefile
# Created by IBM WorkFrame/2 MakeMake at 11:15:51 on 05/11/99
#
# The actions included in this make file are:
#  Compile
#  Link

.SUFFIXES:

.all: \
    .\mr.exe

.\utils.obj: \
    d:\ibmcppw\Working\sstrdll\utils.cpp \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}rowcol.h
    @echo " Compiling generic utilities "
    icc.exe ".\%|fF.obj" /Ti /C d:\ibmcppw\Working\sstrdll\utils.cpp

.\tables.obj: \
    d:\ibmcppw\Working\sstrdll\tables.cpp \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}table.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}setclass.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}rowcol.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}key.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}wherec.h
    @echo " Compiling table class "
    icc.exe ".\%|fF.obj" /Ti /C d:\ibmcppw\Working\sstrdll\tables.cpp

.\setclass.obj: \
    d:\ibmcppw\Working\sstrdll\setclass.cpp \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}setclass.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}table.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}rowcol.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}key.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}wherec.h
    @echo " Compiling set manipulation class "
    icc.exe ".\%|fF.obj" /Ti /C d:\ibmcppw\Working\sstrdll\setclass.cpp

.\rowcol.obj: \
    d:\ibmcppw\Working\sstrdll\rowcol.cpp \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}rowcol.h
    @echo " Compiling row and column classes "
    icc.exe ".\%|fF.obj" /Ti /C d:\ibmcppw\Working\sstrdll\rowcol.cpp

.\manyrowi.obj: \
    d:\ibmcppw\Working\sstrdll\manyrowi.cpp \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}table.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}lhdb.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}database.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}rowcol.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}setclass.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}key.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}wherec.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}dbutils.h
    @echo " Compiling main "
    icc.exe ".\%|fF.obj" /Ti /C d:\ibmcppw\Working\sstrdll\manyrowi.cpp

.\lhdb.obj: \
    d:\ibmcppw\Working\sstrdll\lhdb.cpp \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}lhdb.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}table.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}dbutils.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}setclass.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}rowcol.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}key.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}wherec.h
    @echo " Compile LHDB utilities"
    icc.exe ".\%|fF.obj" /Ti /C d:\ibmcppw\Working\sstrdll\lhdb.cpp

.\key.obj: \
    d:\ibmcppw\Working\sstrdll\key.cpp \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}key.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}rowcol.h
    @echo " Compiling key class "
    icc.exe ".\%|fF.obj" /Ti /C d:\ibmcppw\Working\sstrdll\key.cpp

.\dbutils.obj: \
    d:\ibmcppw\Working\sstrdll\dbutils.cpp \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}dbutils.h
    @echo " Compiling CLI utilities "
    icc.exe ".\%|fF.obj" /Ti /C d:\ibmcppw\Working\sstrdll\dbutils.cpp

.\database.obj: \
    d:\ibmcppw\Working\sstrdll\database.cpp \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}database.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}table.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}wherec.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}key.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}setclass.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}rowcol.h
    @echo " Compiling database class "
    icc.exe ".\%|fF.obj" /Ti /C d:\ibmcppw\Working\sstrdll\database.cpp

.\wherec.obj: \
    d:\ibmcppw\Working\sstrdll\wherec.cpp \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\ibmcppw\Working\sstrdll;$(INCLUDE);}wherec.h
    @echo " Compiling whereclause class "
    icc.exe ".\%|fF.obj" /Ti /C d:\ibmcppw\Working\sstrdll\wherec.cpp

.\mr.exe: \
    .\utils.obj \
    c:\sqllib\lib\db2api.lib \
    c:\sqllib\lib\db2apie.lib \
    c:\sqllib\lib\db2cli.lib \
    .\wherec.obj \
    .\database.obj \
    .\dbutils.obj \
    .\key.obj \
    .\lhdb.obj \
    .\manyrowi.obj \
    .\rowcol.obj \
    .\setclass.obj \
    .\tables.obj
    @echo " Link "
    icc.exe @<<
     /B" /de"
     /Femr.exe 
     .\utils.obj
     c:\sqllib\lib\db2api.lib
     c:\sqllib\lib\db2apie.lib
     c:\sqllib\lib\db2cli.lib
     .\wherec.obj
     .\database.obj
     .\dbutils.obj
     .\key.obj
     .\lhdb.obj
     .\manyrowi.obj
     .\rowcol.obj
     .\setclass.obj
     .\tables.obj
<<
