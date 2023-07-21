all: myshell 

myshell: myshell.o LineParser.o mypipe
	gcc -g -Wall -o myshell myshell.o LineParser.o mypipe
    
myshell.o: myshell.c LineParser.h
	gcc -g -Wall -c -o myshell.o myshell.c
    
LineParser.o: LineParser.c LineParser.h
	gcc -g -Wall -c -o LineParser.o LineParser.c

mypipe:
	gcc -g -o mypipe mypipe.c   
    
.PHONY: clean
    
clean:
	rm -f *.o myshell mypipe