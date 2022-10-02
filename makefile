BIN = parser
CC = g++

SRCS = $(BIN).y  $(BIN).l AST.cpp main.cpp error.cpp semantic.cpp
HDRS = scanType.h AST.h symbolTable.h semantic.h
OBJS = lex.yy.o $(BIN).tab.o AST.o main.o semantic.o symbolTable.o error.o

$(BIN) : $(OBJS)
	$(CC) $(OBJS) -o c-

main.o: main.cpp semantic.h
	$(CC) $(CFLAGS) -c main.cpp

semantic.o: semantic.cpp semantic.h
	$(CC) $(CFLAGS) -c semantic.cpp

error.o: error.cpp semantic.h 
	$(CC) $(CFLAGS) -c error.cpp

AST.o: AST.cpp AST.h 
	$(CC) $(CFLAGS) -c AST.cpp

symbolTable.o: symbolTable.cpp symbolTable.h
	$(CC) $(CFLAGS) -c symbolTable.cpp

lex.yy.c : $(BIN).l $(BIN).tab.h $(HDR)
	flex $(BIN).l

$(BIN).tab.h $(BIN).tab.c : $(BIN).y
	bison -v -t -d $(BIN).y

clean :
	rm -f *~ $(OBJS) $(BIN) lex.yy.c $(BIN).tab.h $(BIN).tab.c $(BIN).output c-

tar : $(HDR) $(SRCS) makefile
	tar -cvf $(BIN).tar $(HDRS) $(SRCS) makefile
