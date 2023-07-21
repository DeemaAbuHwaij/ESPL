#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct
{
    char debug_mode;
    char display_mode;
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
} state;

struct fun_desc
{
    char *name;
    void (*fun)(state *);
};

void toggle_debug_mode(state *s)
{
    s->debug_mode = !s->debug_mode;
    if (s->debug_mode)
    {
        printf("Debug flag now on\n");
    }
    else
    {
        printf("Debug flag now off\n");
    }
}

void set_file_name(state *s)
{
    printf("Enter file name: ");
    scanf("%s", s->file_name);
    if (s->debug_mode)
    {
        fprintf(stderr, "Debug: file name set to %s\n", s->file_name);
    }
}

void set_unit_size(state *s)
{
    printf("Enter unit size (1, 2, or 4): ");
    int size;
    scanf("%d", &size);
    if (size == 1 || size == 2 || size == 4)
    {
        s->unit_size = size;
        if (s->debug_mode)
        {
            if (s->display_mode)
                printf("Debug: set size to %x\n", s->unit_size);
            else
                printf("Debug: set size to %d\n", s->unit_size);
        }
    }
    else
    {
        printf("Invalid unit size\n");
    }
}

void quit(state *s)
{
    if (s->debug_mode == 1)
    {
        fprintf(stderr, "Debug: Quitting\n");
    }
    exit(0);
}

void load_into_memory(state *s)
{
    if (strlen(s->file_name) == 0)
    {
        printf("Error: file name not set\n");
        return;
    }
    FILE *fp = fopen(s->file_name, "r");
    if (fp == NULL)
    {
        printf("Error: failed to open file\n");
        return;
    }
    printf("Enter location (in hexadecimal): ");
    unsigned int location;
    ;
    scanf("%x", &location);
    printf("Enter length (in decimal): ");
    unsigned int length;
    scanf("%d", &length);
    if (s->debug_mode)
    {
        printf("Debug: file_name='%s' location=%x length=%d\n", s->file_name, location, length);
    }
    fseek(fp, location, SEEK_SET);
    s->mem_count = fread(s->mem_buf, s->unit_size, length, fp);
    printf("Loaded %d units into memory\n", length);
    fclose(fp);
}

void toggle_display_mode(state *s)
{
    s->display_mode = !s->display_mode;
    if (s->display_mode)
    {
        printf("Display flag now on, hexadecimal representation\n");
    }
    else
    {
        printf("Display flag now off, decimal representation\n");
    }
}

void print_units(int unit_size, unsigned char *data)
{
    for (int i = unit_size - 1; i >= 0; i--)
    {
        printf("%x", data[i]);
    }
}

void memory_display(state *s)
{
    printf("Enter address and length\n");
    printf("> ");
    unsigned int addr, length;
    scanf("%x %u", &addr, &length);
    if (addr == 0)
    {
        addr = (unsigned int)s->mem_buf;
    }
    if (s->display_mode)
    {
        printf("Hexadecimal\n===========\n");
        for (unsigned int i = 0; i < length; i++)
        {
            print_units(s->unit_size, (unsigned char *)(addr + i * s->unit_size));
            printf("\n");
        }
    }
    else
    {
        printf("Decimal\n=======\n");
        for (unsigned int i = 0; i < length; i++)
        {
            switch (s->unit_size)
            {
            case 1:
                printf("%d\n", *(unsigned char *)(addr + i * s->unit_size));
                break;
            case 2:
                printf("%d\n", *(unsigned short *)(addr + i * s->unit_size));
                break;
            case 4:
                printf("%d\n", *(unsigned int *)(addr + i * s->unit_size));
                break;
            }
        }
    }
}

void save_into_file(state *s)
{
    printf("Please enter <source-address> <target-location> <length>\n");
    int source_addr, target_loc, length;
    scanf("%x %x %d", &source_addr, &target_loc, &length);
    if (strlen(s->file_name) == 0)
    {
        printf("Error: file name is not set\n");
        return;
    }
    FILE *fp = fopen(s->file_name, "r+");
    if (fp == NULL)
    {
        printf("Error: unable to open file\n");
        return;
    }
    if (s->debug_mode)
    {
        printf("Debug: file name = %s\n", s->file_name);
        printf("Debug: source address = %x\n", source_addr);
        printf("Debug: target location = %x\n", target_loc);
        printf("Debug: length = %d\n", length);
    }
    if (target_loc > ftell(fp))
    {
        printf("Error: target location is greater than the size of the file\n");
        return;
    }
    unsigned char* buf;
    fseek(fp, target_loc, SEEK_SET);
  if (source_addr == 0) {
    buf = s->mem_buf;
  }
  else {
    buf = (unsigned char*) source_addr;
  }
  fwrite(buf,s->unit_size,length, fp); 
  fclose(fp);
    // for (int i = 0; i < length; i++)
    // {
    //     fseek(fp, target_loc + i * s->unit_size, SEEK_SET);
    //     fwrite(&s->mem_buf[source_addr + i * s->unit_size], s->unit_size, 1, fp);
    // }
    // fclose(fp);
}

void memory_modify(state *s)
{
    printf("Please enter <location> <val>\n");
    int location, val;
    scanf("%x %x", &location, &val);
    if (s->debug_mode)
    {
        printf("Debug: modifying memory at location 0x%x with value 0x%x\n", location, val);
    }
    // if (location + s->unit_size > s->mem_count)
    // {
    //     printf("Error: invalid memory location\n");
    //     return;
    // }
    memmove(s->mem_buf + location, &val, s->unit_size);
}

void not_implemented_yet(state *s)
{
    printf("Not implemented yet\n");
}

struct fun_desc menu[] = {
    {"Toggle Debug Mode", toggle_debug_mode},
    {"Set File Name", set_file_name},
    {"Set Unit Size", set_unit_size},
    {"Load Into Memory", load_into_memory},
    {"Toggle Display Mode", toggle_display_mode},
    {"Memory Display", memory_display},
    {"Save Into File", save_into_file},
    {"Memory Modify", memory_modify},
    {"Quit", quit}};

int main(int argc, char *argv[])
{
    state *s = (state *)calloc(sizeof(state), 1);
    while (1)
    {
        printf("Choose action:\n");
        for (int i = 0; i < sizeof(menu) / sizeof(struct fun_desc); i++)
        {
            printf("%d-%s\n", i, menu[i].name);
        }
        int choice;
        scanf("%d", &choice);
        if (choice < sizeof(menu) / sizeof(struct fun_desc) && choice >= 0)
        {
            menu[choice].fun(s);
        }
        else
            printf("OUT OF BOUNDS\n");
        printf("\n");
    }

    return 0;
}