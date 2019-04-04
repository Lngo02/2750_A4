# Author: Linda Ngo \
  StudentID: 1004683

CC = gcc
CPPFLAGS= -Iinclude
CFLAGS = -Wall -pedantic -std=c11 -g
LDFLAGS=  -L./bin

#UNAME Shell Variable
UNAME_S := $(shell uname -s)

# OS X Dependant Variables
ifeq ($(UNAME_S), Darwin)
    LIBCAL := ../libcal.dylib
# GNU/Linux Depedant Variables
else ifeq ($(UNAME_S), Linux)
    LIBCAL := ../libcal.so
endif
ifeq ($(UNAME_S), Darwin)
    LIBLIST := ../liblist.dylib
# GNU/Linux Depedant Variables
else ifeq ($(UNAME_S), Linux)
    LIBLIST := ../liblist.so
endif
		
		
libcal: CalendarParser.o LinkedListAPI.o
	$(CC) -shared -o $(LIBCAL) ./bin/CalendarParser.o ./bin/LinkedListAPI.o

all: list parser

parserLib: libcal liblist
	$(CC) -shared -o ./bin/libcal ./bin/CalendarParser.o ./bin/LinkedListAPI.o


liblist: LinkedListAPI.o
	$(CC) -shared -o $(LIBLIST) ./bin/LinkedListAPI.o

run1: main
	./main ics/mLineProp1.ics

run2: main
	./main ics/testCalSimpleNoUTC.ics

run3: main
	./main ics/testCalEvtProp.ics

run4: main
	./main ics/testCalEvtPropAlm.ics

run5: main
	./main ics/testCalSimpleUTCComments.ics

runAll: main run1 run2 run3 run4 run5

sample: main
	./main ics/sample.ics

main: parser list
	$(CC) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) -o main ./src/main.c ./bin/libcal.so ./bin/liblist.so -lm

parser: CalendarParser.o
	$(CC) -shared -o ./bin/libcal.so ./bin/CalendarParser.o ./bin/LinkedListAPI.o

CalendarParser.o: ./src/CalendarParser.c ./include/CalendarParser.h list
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -fpic ./src/CalendarParser.c -o ./bin/CalendarParser.o

list: LinkedListAPI.o
	$(CC) -shared -o ./bin/liblist.so ./bin/LinkedListAPI.o

LinkedListAPI.o: ./src/LinkedListAPI.c ./include/LinkedListAPI.h
	mkdir -p ./bin
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -fpic ./src/LinkedListAPI.c -o ./bin/LinkedListAPI.o

valgrind: main
	valgrind --leak-check=full ./main ics/mLineProp1.ics

valgrind2: main
	valgrind --leak-check=full ./main ics/testCalSimpleNoUTC.ics

valgrind3: main
	valgrind --leak-check=full ./main ics/testCalEvtProp.ics

valgrind4: main
	valgrind --leak-check=full ./main ics/testCalEvtPropAlm.ics

valgrind5: main
	valgrind --leak-check=full ./main ics/testCalSimpleUTCComments.ics

valgrindFile: main
	valgrind --leak-check=full --log-file=valgrind-out.txt \
	./main ics/mLineProp1.ics
	cat valgrind-out.txt

valgrindFile2: main
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
	--verbose --log-file=valgrind-out.txt ./main ics/mLineProp1.ics

valgrindSample: main
	valgrind --leak-check=full --log-file=valgrind-out.txt \
	./main ./ics/sample.ics

valgrindSample2: main
	valgrind --leak-check=full ./main ./ics/sample.ics

malformed: all main
	./main ./ics/malformed.ics

vmalformed: all main
	valgrind --leak-check=full --show-leak-kinds=all ./main ./ics/malformed.ics

clean:
	-rm -rf bin/*.o bin/*.so bin/*.dylib

myclean: clean
	#-rm -i bin/*.o bin/*.so
	-rm -i main
	-rm -i valgrind-out.txt
