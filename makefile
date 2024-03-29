BIN = parser
CC = g++
CFLAGS = -g

SRCS = $(BIN).y  $(BIN).l AST.cpp main.cpp semantic.cpp symbolTable.cpp yyerror.cpp codeGen.cpp emitcode.cpp
HDRS = scanType.h AST.h symbolTable.h semantic.h yyerror.h emitcode.h
OBJS = lex.yy.o $(BIN).tab.o AST.o main.o semantic.o symbolTable.o yyerror.o emitcode.o codeGen.o

$(BIN) : $(OBJS)
	$(CC) $(OBJS) -o c-

main.o: main.cpp semantic.h
	$(CC) $(CFLAGS) -c main.cpp

codeGen.o: codeGen.cpp semantic.h
	$(CC) $(CFLAGS) -c codeGen.cpp

semantic.o: semantic.cpp semantic.h
	$(CC) $(CFLAGS) -c semantic.cpp

emitcode.o.o: emitcode.cpp emitcode.h
	$(CC) $(CFLAGS) -c emitcode.cpp

AST.o: AST.cpp AST.h 
	$(CC) $(CFLAGS) -c AST.cpp

symbolTable.o: symbolTable.cpp symbolTable.h
	$(CC) $(CFLAGS) -c symbolTable.cpp

yyerror.o: yyerror.cpp yyerror.h 
	$(CC) $(CFLAGS) -c yyerror.cpp

lex.yy.c : $(BIN).l $(BIN).tab.h $(HDR)
	flex $(BIN).l

$(BIN).tab.h $(BIN).tab.c : $(BIN).y
	bison -v -t -d $(BIN).y

clean :
	rm -f *~ $(OBJS) $(BIN) lex.yy.c $(BIN).tab.h $(BIN).tab.c $(BIN).output c-

tar : $(HDR) $(SRCS) makefile
	tar -cvf $(BIN).tar $(HDRS) $(SRCS) makefile
