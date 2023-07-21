#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

typedef struct link {
    struct link *nextVirus;
    virus *vir;
} link;

char *fileName[1024];
FILE* infile;
FILE *suspectedFile;
char buffer[10000]; 

// read a virus from a file pointer
virus* readVirus(FILE* f) {
    virus* v = malloc(sizeof(virus));
    fread(&v->SigSize, sizeof(short), 1, f);  //length
    fread(&v->virusName, sizeof(char), 16, f);  //name
    v->sig = malloc(v->SigSize);
    fread(v->sig, sizeof(char), v->SigSize, f);
    return v;
}

// print a virus to a file pointer
void printVirus(virus* v, FILE* f) {
    int counter = 0;
    fprintf(f, "Virus name: %s\n", v->virusName);
    fprintf(f, "Virus size: %d\n", v->SigSize);
    fprintf(f, "Signature:\n");
    for (int i = 0; i < v->SigSize; i++) {
        if(counter == 20){
            printf("\n");
            counter = 0;
        }
        fprintf(f, "%02X ", v->sig[i]);
        counter++;
    }
        fprintf(f, "\n");
}

// Print the data of every link in list to the given stream. Each item followed by a newline character. 
void list_print(link* virus_list, FILE* f) {
    while (virus_list != NULL) {
        printVirus(virus_list->vir, f);
        fprintf(f, "\n");
        virus_list = virus_list->nextVirus;
    }
}

// add a new link with the given data to the list, and return a pointer to the list
link* list_append(link* virus_list, virus* data) {
    link *newLink = (link*)malloc(sizeof(link));
    newLink->vir = data;
    newLink->nextVirus = NULL;
    if (virus_list == NULL) {  /*if the list is null - create a new entry*/
        virus_list = newLink;
    }
    else {
        link *current = virus_list;
        while (current->nextVirus != NULL) {
            current = current->nextVirus;
        }
        current->nextVirus = newLink;
    }
    return virus_list;
}

// free the memory allocated by the list
void list_free(link* virus_list) {
    while (virus_list != NULL) {
        link* tmp = virus_list;
        virus_list = virus_list->nextVirus;
        free(tmp->vir->sig);
        free(tmp->vir);
        free(tmp);
    }
}

link* fixfile(link* virus_list){
    FILE* file = fopen(suspectedFile, "rb");
    if(file == NULL){
        fprintf(stderr, "ERROR!, Could not open file");
        return;
    }
    int fileSize = fread(buffer, 1, 10000, file);
    link* curr = virus_list;     
    while (curr != NULL){
        virus *curr_virus = curr->vir;
        int virus_size = curr_virus->SigSize;
        char *virus_signature = curr_virus->sig;
        for (int i = 0; i < fileSize; i ++) {
            if (memcmp(buffer + i, virus_signature, virus_size) == 0) {
                neutralize_virus(suspectedFile,i);
            }
        }
        curr = curr->nextVirus;       
    }
    fclose(file);
    return virus_list;
}

void neutralize_virus(char *fileName, int signatureOffset) {
    FILE *file = fopen(fileName, "wb+");
    if (file == NULL) {
        fprintf(stderr, "Error: Unable to open file %s\n");
        perror("fopen");
        return;
    }
    fseek(file, signatureOffset, SEEK_SET);
    unsigned char retInstruction = 0xC3;
    fwrite(&retInstruction, sizeof(unsigned char), 1, file);
    fclose(file);
}

link* printSignatures(link* virus_list){
    if(virus_list != NULL)
        list_print(virus_list, stdout);
        return virus_list;
}

link* detectViruses(link* virus_list){
    FILE* fileToDetect = fopen(suspectedFile, "rb");
    if(suspectedFile == NULL){
        fprintf(stderr, "failed opening file\n");
        return virus_list;
    }
    int fileSize = fread(buffer, 1, 10000, fileToDetect);
    detect_virus(buffer, fileSize, virus_list); 
    fclose(fileToDetect);
    return virus_list;
}

void detect_virus(char *buffer, unsigned int size, link *virus_list) {
    link* curr = virus_list;     
    while (curr != NULL){
        virus *curr_virus = curr->vir;
        int virus_size = curr_virus->SigSize;
        char *virus_signature = curr_virus->sig;
        for (int i = 0; i< size; i ++) {
            if (memcmp(buffer + i, virus_signature, virus_size) == 0) {
                fprintf(stderr,"The starting byte location in suspected file: %d\n", i);
                fprintf(stderr,"The Virus name: %s\n", curr_virus->virusName);
                fprintf(stderr,"The size of the virus signature: %d \n", virus_size);
                return;
            }
        }
        curr = curr->nextVirus;      
    }
}

link* quit(link* virus_list){
    if(virus_list != NULL)
        list_free(virus_list);
    exit(0);
    return virus_list;
}

typedef struct fun_desc {
char *name;
link* (*fun)(link*);
} fun_desc;

link* load_signatures(FILE* input) {
    if (suspectedFile == NULL) {
        fprintf(stderr, "Error: input file is NULL\n");
        return;
    }
    link* newLink = NULL;
    char* fileName = NULL;
    char magicNum[4];
    int counter = 4;
    if (input == NULL) {
        fprintf(stderr,"Please enter the file name:\n");
        fgets(buffer, 10000, stdin);
        sscanf(buffer, "%ms", &fileName);
        input = fopen(fileName, "rb");
        free(fileName);
        if (input == NULL) {
               fprintf(stderr, "Could not read the signature file\n");
                return;
        }
    }
    fseek(input, 0, SEEK_END);
    int size = ftell(input);
    rewind(input);
    fread(magicNum, 1, 4, input);
    if(strncmp(magicNum,"VISL",4) != 0){
        fprintf(stderr, "Invalid magic number\n");
        return;
    }
    while (counter < size) {
        virus* virus = readVirus(input);
        newLink = list_append(newLink, virus);
        counter = counter + 18 + virus->SigSize;
    }
    fclose(input);
    return newLink;
}

int main(int argc, char **argv){
    link* virus_List = NULL;
    int option;
    struct fun_desc menu[] = {
            {"Load signature", load_signatures},
            {"Print signature", printSignatures},
            {"Detect viruses", detectViruses},
            {"Fix file", fixfile},
            {"Quit", quit},
            {NULL, NULL}};
    if(argc > 1)
        suspectedFile = argv[1];
    while(1){
        int menuSize = sizeof(menu) / sizeof(struct fun_desc) - 1;
        fprintf(stderr, "Select operation from the following menu:\n");
        for(int i = 0; i < menuSize; i++){
            printf("%d) %s\n", i+1, menu[i].name);
        }
        printf("Option: ");
        scanf("%d", &option);
        fgetc(stdin);
        if(feof(stdin)){
            break;
        }
        if(option >= 0 && option <= menuSize){ 
            printf("Within bounds.\n");
        }
        else{
            printf("Not within bounds.\n");
            break;
        }
        virus_List = menu[option - 1].fun(virus_List);
        printf("\n");
    }
}