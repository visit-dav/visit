PROG=sim

SRC=sim.c
OBJ=$(SRC:.c=.o)

CC=gcc
LD=gcc
CFLAGS=-g
LDFLAGS=
LIBS=



all:  $(PROG)

.c.o:
	$(CC) $(CFLAGS) -c $<

$(PROG): $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) $(LIBS) -o $@
