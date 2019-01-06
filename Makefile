EXE = txtwrite
CC = gcc
OBJS = txtwrite.o

all: $(exe) ROOMS1 SPECIAL1 LINE

ROOMS1: $(EXE) rooms.txt
	$(EXE) -e ROOMS1 ROOMS2 < rooms.txt
SPECIAL1: $(EXE) special.txt
	$(EXE) -e SPECIAL1 SPECIAL2 < special.txt
LINE: $(EXE) line.txt
	$(EXE) -e LINE < line.txt

$(EXE): $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(LDFLAGS)

.c.o:
	$(CC) -c $(CFLAGS) -O3 $<
clean:
	rm -f ROOMS1 ROOMS2 SPECIAL1 SPECIAL2 LINE
	rm -f *.o
	rm -f $(EXE)
	rm -f $(EXE).exe
