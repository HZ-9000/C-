BIN = parser
CC = g++

SRCS = $(BIN).y  $(BIN).l AST.cpp main.cpp 
HDRS = scanType.h AST.h
OBJS = lex.yy.o $(BIN).tab.o AST.o main.o

$(BIN) : $(OBJS)
	$(CC) $(OBJS) -o c-

main.o: main.cpp AST.h
	$(CC) $(CFLAGS) -c main.cpp

AST.o: AST.cpp AST.h 
	$(CC) $(CFLAGS) -c AST.cpp

lex.yy.c : $(BIN).l $(BIN).tab.h $(HDR)
	flex $(BIN).l

$(BIN).tab.h $(BIN).tab.c : $(BIN).y
	bison -v -t -d $(BIN).y

clean :
	rm -f *~ $(OBJS) $(BIN) lex.yy.c $(BIN).tab.h $(BIN).tab.c $(BIN).output 

tar : $(HDR) $(SRCS) makefile
	tar -cvf $(BIN).tar $(HDRS) $(SRCS) makefile
