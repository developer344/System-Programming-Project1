CC = gcc
CCFLAGS = -no-pie
INCLUDE = -I ./include
SRC = ./src
BIN = ./bin

all: main

main: $(SRC)/vaccineMonitor.c
	$(CC) $(CCFLAGS) $(INCLUDE) -o $(BIN)/vaccineMonitor $(SRC)/vaccineMonitor.c $(SRC)/date.c $(SRC)/bloomfilter.c $(SRC)/skipList.c $(SRC)/BST.c $(SRC)/countryTree.c $(SRC)/Virus.c $(SRC)/linkedList.c

clean:
	rm $(BIN)/vaccineMonitor