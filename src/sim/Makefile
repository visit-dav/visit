EWRAP=libewrap.so
EWRAPSRC=enginewrapper.C
EWRAPOBJ=$(EWRAPSRC:.C=.o)

SIM=sim
SIMSRC=sim.c
SIMOBJ=$(SIMSRC:.c=.o)

CC=gcc
CXX=g++
LD=gcc
CFLAGS=-g -I../include/visit -Wimplicit-function-declaration
LDFLAGS=-L../lib
LIBS=-ldl -lsocket -lnsl
LIBLIBS=-lengine_ser -lplugin -lviswindow -lavtmath_ser -lplotter \
        -ldatabase_ser -lmir -lquery_ser -lexpressions_ser -lavtfilters_ser \
        -lpipeline_ser -lavtview -lavtwriter -lvisit_vtk \
        -llightweight_visit_vtk \
	-lenginerpc -lutility -lmisc -lcomm -lstate \
	-lengineparstate_ser -ldbatts -lavtexceptions -lparser -lvtkCommon \
        -lvtkDICOMParser -lvtkFiltering -lvtkGraphics -lvtkHybrid -lvtkIO \
        -lvtkImaging -lvtkRendering -lvtkzlib -lvtkexpat -lvtkjpeg -lvtkpng \
        -lvtktiff -lvtkftgl -lvtkfreetype -lGL -lOSMesa -lMesaGL -lX11 \
	-lm -lresolv -lsocket -lnsl -ldl  -lthread

all:  $(SIM) $(EWRAP)
	cd mainplugin && visit -xml2makefile -noengineplugin -clobber -noprint SimDB.xml
	cd simplugin && env VISITPLUGININST=.. visit -xml2makefile -onlyengineplugin -clobber -noprint SimDB.xml
	cd mainplugin && $(MAKE)
	cd simplugin && $(MAKE)

.c.o:
	$(CC) $(CFLAGS) -c $<

.C.o:
	$(CXX) $(CFLAGS) -c $<

$(SIM): $(SIMOBJ)
	$(LD) $(LDFLAGS) $(SIMOBJ) $(LIBS) -o $@

$(SIM).pure: $(SIMOBJ)
	purify $(LD) $(LDFLAGS) $(SIMOBJ) $(LIBS) -o $@

$(EWRAP): $(EWRAPOBJ)
	$(LD) -Wl,-G $(LDFLAGS) $(EWRAPOBJ) -o $@ $(LIBLIBS)


clean:
	rm -f $(SIMOBJ) $(EWRAPOBJ) $(SIM) $(SIM).pure $(EWRAP)

.SUFFIXES: .C
