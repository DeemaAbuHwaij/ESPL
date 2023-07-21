#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int digit_cnt(char* arg){
	int count = 0;
        for(int i = 0; arg[i] != '\0'; i++){
            if(arg[i] >= '0' && arg[i] <= '9')
            count++;
        }
	return count;
}

int main(int argc, char **argv) {
    if(argc < 2)
        printf("task4 <string>\n");
    else
        printf("The number of digits in the string is: %d\n", digit_cnt(argv[1]));
    
}