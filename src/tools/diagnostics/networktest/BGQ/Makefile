#-------------------------------------------------------------------------------
# Default target
default : all
#-------------------------------------------------------------------------------
# Ensure no naming conflicts for these targets with the file system
.PHONY : default all clean build cnk
#-------------------------------------------------------------------------------
# Variables
BGQ_CXX=g++
BGQ_CROSS_CXX=bgxlC_r
CXXFLAGS=-g -DSERVER_MPI
LDFLAGS=
RM=rm -f

BLUEGENEQ=/bgsys/drivers/ppcfloor
MPI_CXXFLAGS=-I${BLUEGENEQ}/comm/include
MPI_LDFLAGS=-L${BLUEGENEQ}/spi/lib -L${BLUEGENEQ}/comm/lib -R/opt/ibmcmp/lib64/bg
MPI_LIBS=-lmpich-xl -lopa-xl -lmpl-xl -lpami-gcc -lSPI -lSPI_cnk -lrt -lpthread -lstdc++ -lpthread 

#-------------------------------------------------------------------------------
# Build rules
build : cnk

cnk : server.elf client.elf

server.elf : bgqserver.o
	$(BGQ_CROSS_CXX) $? ${LDFLAGS} -o $@ ${MPI_LDFLAGS} ${MPI_LIBS}

client.elf : bgqclient.o
	$(BGQ_CXX) $? ${LDFLAGS} -o $@

bgqclient.o:
	$(BGQ_CXX) ${CXXFLAGS} -c bgqclient.cpp -o $@

bgqserver.o:
	$(BGQ_CROSS_CXX) ${CXXFLAGS} ${MPI_CXXFLAGS} -c bgqserver.cpp -o $@
	
%.o : %.cpp
	$(BGQ_CROSS_CXX) ${CXXFLAGS} -c $< -o $@

#-------------------------------------------------------------------------------
# All
all : build
#-------------------------------------------------------------------------------
# clean
clean:
	$(RM) *.elf *.o
