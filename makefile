IDIR	=include
SDIR	=src
ODIR	=obj
BDIR	=bin

CC	=gcc
CFLAGS	=-std=gnu89 -pedantic -Wall -I$(IDIR)
CLIBS	=-lncurses

minesweeper : $(ODIR)/main.o $(ODIR)/ui.o $(ODIR)/minesweeper.o
	$(CC) $(CFLAGS) $^ -o $(BDIR)/$@ $(CLIBS)

$(ODIR)/minesweeper.o : $(SDIR)/minesweeper.c $(IDIR)/minesweeper.h
	$(CC) -c $(CFLAGS) $< -o $@

$(ODIR)/ui.o : $(SDIR)/ui.c $(IDIR)/ui.h $(IDIR)/minesweeper.h
	$(CC) -c $(CFLAGS) $< -o $@

$(ODIR)/main.o : $(SDIR)/main.c $(IDIR)/main.h $(IDIR)/ui.h $(IDIR)/minesweeper.h
	$(CC) -c $(CFLAGS) $< -o $@
