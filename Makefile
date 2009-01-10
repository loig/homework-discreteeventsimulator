# C compiler
CC=	gcc
CFLAGS=	-W -Wall -pedantic -std=c99 -lm

OBJ=simulator

# uncomment the second line if you use pdftex to bypass .dvi files
#PDFTEX = dvipdfm
PDFTEX = pdftex

CWEAVE = cweave
CTANGLE = ctangle


WEBSOURCES = simulator.w
ALLWEBSOURCES = events.w   \
		main.w  \
		network.w  \
		prelude.w  \
		scheduler.w  \
		simulation.w \
		simulator.w

CSOURCES = parser.c simulator.c



.SUFFIXES: .dvi .tex .w .pdf

.w.tex:
	$(CWEAVE) $*

.tex.dvi:	
	tex $<

.w.dvi:
	make $*.tex
	make $*.dvi

.w.c:
	$(CTANGLE) $*

.w.o:
	make $*.c
	make $*.o

.w.pdf:
	make $*.tex
	case "$(PDFTEX)" in \
	 dvipdfm ) tex "\let\pdf+ \input $*"; dvipdfm $* ;; \
	 pdftex ) pdftex $* ;; \
	esac

all: web tangle

web: $(WEBSOURCES:.w=.pdf)

tangle: 
	$(CTANGLE) simulator.w
	$(CC) $(CFLAGS) $(CSOURCES) -o $(OBJ) 

clean:
	rm -rf $(OBJ) simulator.c simulator.h simulator_events.h simulator_network.h simulator.tex simulator.dvi simulator.pdf