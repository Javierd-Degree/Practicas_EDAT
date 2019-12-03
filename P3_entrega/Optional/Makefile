CC = gcc
CFLAGS = -Wall -Wextra -pedantic -ansi -g
LDLIBS = -lodbc

EXE = score_index suggest_index scorematch

all : $(EXE)

clean :
	rm -f *.o core $(EXE)

$(EXE) : % : %.o odbc.o table.o type.o index_optional.o
