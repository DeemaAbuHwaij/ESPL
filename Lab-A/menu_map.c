#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct fun_desc {
  char *name;
  char (*fun)(char);
};


/* Ignores c, reads and returns a character from stdin using fgetc. */ 
char my_get(char c){
  char ch = fgetc(stdin);
  return ch;
}

/* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII
 value c followed by a new line. Otherwise, cprt prints the dot ('.') character. 
 After printing, cprt returns the value of c unchanged. */
char cprt(char c){
  if(c > 0x20 && c < 0x7E){
    printf("%c\n", c);
  }
  else {
    printf(".\n");
  }
  return c;
}

/* Gets a char c and returns its encrypted form by adding 1 to its value. If c is
 not between 0x20 and 0x7E it is returned unchanged */
char encrypt(char c){
  if(c > 0x20 && c < 0x7E){
    c = c+1;
  }
  return c;
}

/* Gets a char c and returns its decrypted form by reducing 1 from its value.
 If c is not between 0x20 and 0x7E it is returned unchanged */
char decrypt(char c){
  if(c > 0x20 && c < 0x7E){
    c = c-1;
  }
  return c;
}


/* xprt prints the value of c in a hexadecimal representation followed by a new line,
and returns c unchanged. */ 
char xprt(char c){
  if(c > 0x20 && c < 0x7E){
    printf("%x\n", c);
  }
  else {
    printf(".\n");
  }
  return c;
}

/*do the functon f at array and put them in mapped_array*/
char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  /* TODO: Complete during task 2.a */
  for(int i = 0; i < array_length; i++)
    mapped_array[i] = f(array[i]);
  return mapped_array;
}
 
int main(int argc, char **argv){
  /* TODO: Test your code */
  char input[1000];
  char* carray = calloc(5, sizeof(char));
  struct fun_desc menu[] = {
    { "Get String", my_get },
    { "Print String", cprt },
    { "Print Hexa", xprt },
    { "Encrypt", encrypt },
    { "Decrypt", decrypt },  
    { NULL, NULL } }; 
  int size = sizeof(menu) / sizeof(struct fun_desc) - 1;
  while(1){
    printf("Select operation from the following menu:\n");
    for(int i = 0; i < size; i++){
      printf("%d) %s\n", i, menu[i].name);
    }
    printf("Option : ");
    //input
    if(fgets(input, 1000, stdin) == NULL){
      break;
    }
    int option = atoi(input);
    if(option < 0 || option > size-1){
      fprintf(stderr, "Not within bounds\n");
      break;
    }
    fprintf(stderr, "Within bounds\n");
    carray = map(carray, 5, menu[option].fun);
    printf("DONE.\n\n");
  } 
  free(carray);
}

