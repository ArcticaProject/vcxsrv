ORIGIN = PWB
ORIGIN_VER = 2.0
PROJ = BISON_PP
PROJFILE = BISON_PP.MAK
DEBUG = 1

CC  = cl
CFLAGS_G  = /AL /W4 /Za /BATCH /Gt8 /DSTDC_HEADERS /DHAVE_STRERROR
CFLAGS_D  = /f /Od /Zi /Zr
CFLAGS_R  = /f- /Ot /Ol /Og /Oe /Oi /Gs
CXX  = cl
CXXFLAGS_G  = /W2 /BATCH
CXXFLAGS_D  = /f /Zi /Od
CXXFLAGS_R  = /f- /Ot /Oi /Ol /Oe /Og /Gs
MAPFILE_D  = NUL
MAPFILE_R  = NUL
LFLAGS_G  = /NOI /STACK:32000 /BATCH /ONERROR:NOEXE
LFLAGS_D  = /CO /FAR /PACKC
LFLAGS_R  = /EXE /FAR /PACKC
LINKER	= link
ILINK  = ilink
LRF  = echo > NUL
ILFLAGS  = /a /e
RUNFLAGS  = -dtv -o d:\tmp\test.cpp -h d:\tmp\test.h d:\tmp\test.y

FILES  = ALLOCATE.C CLOSURE.C DERIVES.C FILES.C GETARGS.C GETOPT.C GETOPT1.C\
	GRAM.C LALR.C LEX.C MAIN.C NULLABLE.C OUTPUT.C PRINT.C READER.C\
	REDUCE.C SYMTAB.C VERSION.C WARSHALL.C LR0.C CONFLICT.C
OBJS  = ALLOCATE.obj CLOSURE.obj DERIVES.obj FILES.obj GETARGS.obj GETOPT.obj\
	GETOPT1.obj GRAM.obj LALR.obj LEX.obj MAIN.obj NULLABLE.obj OUTPUT.obj\
	PRINT.obj READER.obj REDUCE.obj SYMTAB.obj VERSION.obj WARSHALL.obj\
	LR0.obj CONFLICT.obj

all: $(PROJ).exe

.SUFFIXES:
.SUFFIXES: .obj .c
.SUFFIXES: .obj .c

ALLOCATE.obj : ALLOCATE.C
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoALLOCATE.obj ALLOCATE.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoALLOCATE.obj ALLOCATE.C
<<
!ENDIF

CLOSURE.obj : CLOSURE.C system.h machine.h new.h gram.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoCLOSURE.obj CLOSURE.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoCLOSURE.obj CLOSURE.C
<<
!ENDIF

DERIVES.obj : DERIVES.C system.h new.h types.h gram.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoDERIVES.obj DERIVES.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoDERIVES.obj DERIVES.C
<<
!ENDIF

FILES.obj : FILES.C system.h files.h new.h gram.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoFILES.obj FILES.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoFILES.obj FILES.C
<<
!ENDIF

GETARGS.obj : GETARGS.C getopt.h system.h files.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoGETARGS.obj GETARGS.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoGETARGS.obj GETARGS.C
<<
!ENDIF

GETOPT.obj : GETOPT.C getopt.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoGETOPT.obj GETOPT.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoGETOPT.obj GETOPT.C
<<
!ENDIF

GETOPT1.obj : GETOPT1.C getopt.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoGETOPT1.obj GETOPT1.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoGETOPT1.obj GETOPT1.C
<<
!ENDIF

GRAM.obj : GRAM.C
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoGRAM.obj GRAM.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoGRAM.obj GRAM.C
<<
!ENDIF

LALR.obj : LALR.C system.h machine.h types.h state.h new.h gram.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoLALR.obj LALR.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoLALR.obj LALR.C
<<
!ENDIF

LEX.obj : LEX.C system.h files.h symtab.h lex.h new.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoLEX.obj LEX.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoLEX.obj LEX.C
<<
!ENDIF

MAIN.obj : MAIN.C system.h machine.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoMAIN.obj MAIN.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoMAIN.obj MAIN.C
<<
!ENDIF

NULLABLE.obj : NULLABLE.C system.h types.h gram.h new.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoNULLABLE.obj NULLABLE.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoNULLABLE.obj NULLABLE.C
<<
!ENDIF

OUTPUT.obj : OUTPUT.C system.h machine.h new.h files.h gram.h state.h symtab.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoOUTPUT.obj OUTPUT.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoOUTPUT.obj OUTPUT.C
<<
!ENDIF

PRINT.obj : PRINT.C system.h machine.h new.h files.h gram.h state.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoPRINT.obj PRINT.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoPRINT.obj PRINT.C
<<
!ENDIF

READER.obj : READER.C system.h files.h new.h symtab.h lex.h gram.h machine.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoREADER.obj READER.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoREADER.obj READER.C
<<
!ENDIF

REDUCE.obj : REDUCE.C system.h files.h gram.h machine.h new.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoREDUCE.obj REDUCE.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoREDUCE.obj REDUCE.C
<<
!ENDIF

SYMTAB.obj : SYMTAB.C system.h new.h symtab.h gram.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoSYMTAB.obj SYMTAB.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoSYMTAB.obj SYMTAB.C
<<
!ENDIF

VERSION.obj : VERSION.C
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoVERSION.obj VERSION.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoVERSION.obj VERSION.C
<<
!ENDIF

WARSHALL.obj : WARSHALL.C system.h machine.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoWARSHALL.obj WARSHALL.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoWARSHALL.obj WARSHALL.C
<<
!ENDIF

LR0.obj : LR0.C system.h machine.h new.h gram.h state.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoLR0.obj LR0.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoLR0.obj LR0.C
<<
!ENDIF

CONFLICT.obj : CONFLICT.C system.h machine.h new.h files.h gram.h state.h
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /FoCONFLICT.obj CONFLICT.C
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /FoCONFLICT.obj CONFLICT.C
<<
!ENDIF


$(PROJ).exe : $(OBJS)
!IF $(DEBUG)
	$(LRF) @<<$(PROJ).lrf
$(RT_OBJS: = +^
) $(OBJS: = +^
)
$@
$(MAPFILE_D)
$(LIBS: = +^
) +
$(LLIBS_G: = +^
) +
$(LLIBS_D: = +^
)
$(DEF_FILE) $(LFLAGS_G) $(LFLAGS_D);
<<
!ELSE
	$(LRF) @<<$(PROJ).lrf
$(RT_OBJS: = +^
) $(OBJS: = +^
)
$@
$(MAPFILE_R)
$(LIBS: = +^
) +
$(LLIBS_G: = +^
) +
$(LLIBS_R: = +^
)
$(DEF_FILE) $(LFLAGS_G) $(LFLAGS_R);
<<
!ENDIF
	$(LINKER) @$(PROJ).lrf


.c.obj :
!IF $(DEBUG)
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_D) /Fo$@ $<
<<
!ELSE
	@$(CC) @<<$(PROJ).rsp
/c $(CFLAGS_G)
$(CFLAGS_R) /Fo$@ $<
<<
!ENDIF


run: $(PROJ).exe
	$(PROJ).exe $(RUNFLAGS)

debug: $(PROJ).exe
	CV $(CVFLAGS) $(PROJ).exe $(RUNFLAGS)

# << User_supplied_information >>
