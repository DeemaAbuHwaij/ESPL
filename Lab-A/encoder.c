#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv) {
    int encryption = 0;
    int key_length = 0;
    char *key = NULL;
    int c = 0;  
    int index = 0;
    FILE *infile = stdin; /*Part 3*/
    FILE *outfile = stdout;


    int mode = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "+D") == 0) {
            mode = 1;
        }
        else if (strcmp(argv[i], "-D") == 0) {
            mode = 0;
        }
    }

    if (mode) {
        for (int i = 1; i < argc; i++) {
            fprintf(stderr, "%s\n", argv[i]);
        }
    }

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "+e", 2) == 0) {
            encryption = 1;
            key_length = strlen(argv[i] + 2);
            key = malloc(key_length + 1);
            strcpy(key, argv[i] + 2);
        }
        else if (strncmp(argv[i], "-e", 2) == 0) {
            encryption = -1;
            key_length = strlen(argv[i] + 2);
            key = malloc(key_length + 1);
            strcpy(key, argv[i] + 2); 
        }
        else if (strncmp(argv[i], "-i", 2) == 0) {
            infile = fopen(argv[i] + 2, "r");
            if (infile == NULL) {
                fprintf(stderr, "ERROR!%s\n", argv[i] + 2);
                return 1;
            }
        } else if (strncmp(argv[i], "-o", 2) == 0) {
            outfile = fopen(argv[i] + 2, "w");
            if (outfile == NULL) {
                fprintf(stderr, "ERROR!,could not open output file %s\n", argv[i] + 2);
                return 1;
            }
        }
    }

    while ((c = fgetc(infile)) != EOF) {
        if((c >= 'A' && c <= 'Z')||(c >= 'a' && c <= 'z')||(c >= '0' && c <= '9')|| c == '\n'){
            int key_index = index % key_length;
            int encoding_digit = key[key_index] - '0';
            if (encryption == -1) {
                encoding_digit = (-1) * encoding_digit;
            }
            if (c >= '0' && c <= '9')
                c = ((c - '0' + encoding_digit + 10) % 10) + '0';
            else if (c >= 'a' && c <= 'z') 
                c = ((c - 'a' + encoding_digit + 26) % 26) + 'a';
            else if (c >= 'A' && c <= 'Z') 
                c = ((c - 'A' + encoding_digit + 26) % 26) + 'A';  
            key_index = (key_index + 1) % key_length;
            index++;
        }
        fputc(c, outfile);
    }

    if (infile != stdin) 
        fclose(infile);
    if (outfile != stdout) 
        fclose(outfile);

    free(key);
    return 0;
}
