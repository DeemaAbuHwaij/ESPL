all: mypipeline myshell looper 

mypipeline: mypipeline.c
	gcc -g -Wall -o mypipeline mypipeline.c

myshell: myshell.o LineParser.o 
	gcc -g -Wall -o myshell myshell.o LineParser.o 
 
myshell.o: myshell.c LineParser.h
	gcc -g -Wall -c -o myshell.o myshell.c

looper: looper.c
	gcc -g -Wall -o looper looper.c

LineParser.o: LineParser.c LineParser.h
	gcc -g -Wall -c -o LineParser.o LineParser.c
    
.PHONY: clean
    
clean:
	rm -f *.o mypipeline myshell looper myshell