# outsvr2.mak
# Created by IBM WorkFrame/2 MakeMake at 09:55:59 on 03/11/99
#
# The actions included in this make file are:
#  Compile
#  Make Def file
#  Make exp and lib files
#  Link

.SUFFIXES:

.all: \
    .\storp.dll

.\utils.obj: \
    d:\IBMCPPW\Working\sstrdll\utils.cpp \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}rowcol.h
    @echo " Compile "
    icc.exe /Gm /Ti /Ge- /Gf- /Fo".\%|fF.obj" /C d:\IBMCPPW\Working\sstrdll\utils.cpp

.\tables.obj: \
    d:\IBMCPPW\Working\sstrdll\tables.cpp \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}table.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}setclass.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}rowcol.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}key.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}wherec.h
    @echo " Compile "
    icc.exe /Gm /Ti /Ge- /Gf- /Fo".\%|fF.obj" /C d:\IBMCPPW\Working\sstrdll\tables.cpp

.\storp.obj: \
    d:\IBMCPPW\Working\sstrdll\storp.cpp \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}table.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}lhdb.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}database.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}rowcol.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}setclass.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}key.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}wherec.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}dbutils.h
    @echo " Compile "
    icc.exe /Gm /Ti /Ge- /Gf- /Fo".\%|fF.obj" /C d:\IBMCPPW\Working\sstrdll\storp.cpp

.\setclass.obj: \
    d:\IBMCPPW\Working\sstrdll\setclass.cpp \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}setclass.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}table.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}rowcol.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}key.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}wherec.h
    @echo " Compile "
    icc.exe /Gm /Ti /Ge- /Gf- /Fo".\%|fF.obj" /C d:\IBMCPPW\Working\sstrdll\setclass.cpp

.\rowcol.obj: \
    d:\IBMCPPW\Working\sstrdll\rowcol.cpp \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}rowcol.h
    @echo " Compile "
    icc.exe /Gm /Ti /Ge- /Gf- /Fo".\%|fF.obj" /C d:\IBMCPPW\Working\sstrdll\rowcol.cpp

.\lhdb.obj: \
    d:\IBMCPPW\Working\sstrdll\lhdb.cpp \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}lhdb.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}table.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}dbutils.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}setclass.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}rowcol.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}key.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}wherec.h
    @echo " Compile "
    icc.exe /Gm /Ti /Ge- /Gf- /Fo".\%|fF.obj" /C d:\IBMCPPW\Working\sstrdll\lhdb.cpp

.\key.obj: \
    d:\IBMCPPW\Working\sstrdll\key.cpp \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}key.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}rowcol.h
    @echo " Compile "
    icc.exe /Gm /Ti /Ge- /Gf- /Fo".\%|fF.obj" /C d:\IBMCPPW\Working\sstrdll\key.cpp

.\dbutils.obj: \
    d:\IBMCPPW\Working\sstrdll\dbutils.cpp \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}dbutils.h
    @echo " Compile "
    icc.exe /Gm /Ti /Ge- /Gf- /Fo".\%|fF.obj" /C d:\IBMCPPW\Working\sstrdll\dbutils.cpp

.\database.obj: \
    d:\IBMCPPW\Working\sstrdll\database.cpp \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}database.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}table.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}wherec.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}key.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}setclass.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}rowcol.h
    @echo " Compile "
    icc.exe /Gm /Ti /Ge- /Gf- /Fo".\%|fF.obj" /C d:\IBMCPPW\Working\sstrdll\database.cpp

.\wherec.obj: \
    d:\IBMCPPW\Working\sstrdll\wherec.cpp \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}sstr.h \
    {d:\IBMCPPW\Working\sstrdll;$(INCLUDE);}wherec.h
    @echo " Compile "
    icc.exe /Gm /Ti /Ge- /Gf- /Fo".\%|fF.obj" /C d:\IBMCPPW\Working\sstrdll\wherec.cpp

.\storp.def: \
    .\utils.obj \
    .\wherec.obj \
    .\database.obj \
    .\dbutils.obj \
    .\key.obj \
    .\lhdb.obj \
    .\rowcol.obj \
    .\setclass.obj \
    .\storp.obj \
    .\tables.obj
    @echo " Make Def file "
    imakedef.bat storp storp.def .\utils.obj .\tables.obj .\storp.obj .\setclass.obj .\rowcol.obj .\lhdb.obj .\key.obj .\dbutils.obj .\database.obj .\wherec.obj

.\storp.exp: \
    .\storp.def
    @echo " Make exp and lib files "
    ilib.exe /Gi:storp .\storp.def

.\storp.dll: \
    .\utils.obj \
    .\storp.exp \
    c:\sqllib\lib\db2api.lib \
    c:\sqllib\lib\db2apie.lib \
    c:\sqllib\lib\db2cli.lib \
    .\wherec.obj \
    .\database.obj \
    .\dbutils.obj \
    .\key.obj \
    .\lhdb.obj \
    .\rowcol.obj \
    .\setclass.obj \
    .\storp.obj \
    .\tables.obj
    @echo " Link "
    icc.exe @<<
     /B" /de /pmtype:vio /noe /code:RX /data:RW /dll"
     /B" /def"
     /B" /nod:storp.lib"
     /Festorp.dll 
     .\utils.obj
     .\storp.exp
     c:\sqllib\lib\db2api.lib
     c:\sqllib\lib\db2apie.lib
     c:\sqllib\lib\db2cli.lib
     .\wherec.obj
     .\database.obj
     .\dbutils.obj
     .\key.obj
     .\lhdb.obj
     .\rowcol.obj
     .\setclass.obj
     .\storp.obj
     .\tables.obj
<<
