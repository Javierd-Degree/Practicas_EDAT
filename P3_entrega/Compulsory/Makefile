CC = gcc
CFLAGS = -Wall -Wextra -pedantic -ansi -g
LDLIBS = -lodbc

EXE = test score suggest score_index suggest_index index_test

all : $(EXE)

clean :
	rm -f *.o core $(EXE)

$(EXE) : % : %.o odbc.o table.o type.o index.o
