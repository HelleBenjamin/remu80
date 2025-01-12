#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*  
    C compiler for Z80

    WORK IN PROGRESS

*/

/*
    TODO:
    
    - Add stack based variables

*/

#define MAX_VARS 16
#define MAX_LINE_LENGTH 128

typedef struct {
    char name[16];
    int address; // Memory address for variable
} Variable;

Variable variables[MAX_VARS];
int var_count = 0;
int label_count = 0;

// Add variable to the table
void add_variable(const char *name) {
    if (var_count >= MAX_VARS) {
        fprintf(stderr, "Error: Too many variables.\n");
        exit(1);
    }
    strcpy(variables[var_count].name, name);
    variables[var_count].address = 0x8000 + (var_count * 2);
    var_count++;
}

// Get variable memory address
int get_variable_address(const char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].address;
        }
    }
    fprintf(stderr, "Error: Undefined variable '%s'.\n", name);
    exit(1);
}

// Trim leading/trailing whitespace
char *trim_whitespace(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == '\0') return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}

// Compile a single line
void compile_line(const char *line, FILE *output, int ind) {
    char keyword[16], arg1[16], arg2[16];
    char buffer[MAX_LINE_LENGTH];
    strcpy(buffer, line);
    trim_whitespace(buffer);

    if (strcmp(buffer, "\n") == 0) return;

    if (sscanf(buffer, "int %s = %s;", arg1, arg2) == 2) {
        add_variable(arg1);
        fprintf(output, "LD A, %s\n", arg2); // Load immediate value
        fprintf(output, "LD (%04X), A\n", get_variable_address(arg1));
    } else if (sscanf(buffer, "if (%s == %s)", arg1, arg2) == 2) {
        fprintf(output, "LD A, (%04X)\n", get_variable_address(arg1));
        fprintf(output, "CP %s\n", arg2);
        fprintf(output, "JP Z, LABEL%d\n", label_count);
        label_count++;
    } else if (sscanf(buffer, "goto %s;", arg1) == 1) {
        fprintf(output, "JP %s\n", arg1);
    } else if (strcmp(buffer, "}") == 0) {
        // Skip closing braces
    } else if (strlen(buffer) == 0) {
        // Skip empty lines
    } else {
        fprintf(stderr, "Error: Unrecognized line: %s at %d \n", line, ind);
        exit(1);
    }
}

void compile(const char *input_file, const char *output_file) {
    FILE *input = fopen(input_file, "r");
    FILE *output = fopen(output_file, "w");

    if (!input || !output) {
        fprintf(stderr, "Error: Unable to open input or output file.\n");
        exit(1);
    }
    int ind = 0;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), input)) {
        compile_line(line, output, ind);
        ind++;
    }

    fclose(input);
    fclose(output);
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    compile(argv[1], argv[2]);
    printf("Compilation successful. Output written to %s\n", argv[2]);
    return 0;
}
