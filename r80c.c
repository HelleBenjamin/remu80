#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/*  
    ZC80 compiler for Z80

    ZC80 is a C-like language for the Z80 CPU

    Declare all variables before any functions

    HL as parameter for functions
    BC as return value pointer

    example:

    uint16_t a = 2;

    uint16_t foo(uint16_t a, uint16_t* ret) { // ret is the return value
        a++;
        return a;
    }

    uint16_t main(0, 0) {
        foo(a, &a);
        return 0;
    }


    WORK IN PROGRESS

*/

/*
    TODO:
    
*/

#define MAX_VARS 16
#define MAX_LINE_LENGTH 128

typedef struct {
    char name[16];
    int address;
} Variable;

Variable variables[MAX_VARS];
int var_count = 0;
int label_count = 0;

void add_variable(const char *name, int type) {
    if (var_count >= MAX_VARS) {
        fprintf(stderr, "Error: Too many variables.\n");
        exit(1);
    }
    strcpy(variables[var_count].name, name);
    variables[var_count].address = 0x8000 + (var_count * 1);
    var_count++;
}

bool is_variable(const char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return true;
        }
    }
    return false;
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
    while (isspace((unsigned char)*str) || *str == '\t') str++;
    if (*str == '\0') return str;
    end = str + strlen(str) - 1;
    while (end > str && (isspace((unsigned char)*end) || *end == '\t')) end--;
    *(end + 1) = '\0';
    return str;
}

// Compile a single line
void compile_line(const char *line, FILE *output, int ind) {
    char keyword[16], name[16], arg1[16], arg2[16];
    char buffer[MAX_LINE_LENGTH];
    strcpy(buffer, line);
    trim_whitespace(buffer);

    if (strcmp(buffer, "\n") == 0) return;

    if (sscanf(buffer, "int %s = %s;", arg1, arg2) == 2) { // initialized variable
        add_variable(arg1);
        fprintf(output, "\tLD A, %s\n", arg2); // Load immediate value
        fprintf(output, "\tLD ($%04X), A\n", get_variable_address(arg1));
    } else if (sscanf(buffer, "int %s;", arg1) == 1) { // uninitialized variable
        add_variable(arg1);
        fprintf(output, "\tLD A, 0\n"); // Load immediate value
        fprintf(output, "\tLD ($%04X), A\n", get_variable_address(arg1));
    } else if (sscanf(buffer, "void %s() {", name) == 1) {
        fprintf(output, "l_%d:\n", label_count);
        label_count++;
    }
     else if (sscanf(buffer, "if (%s == %s)", arg1, arg2) == 2) {
        fprintf(output, "\tLD A, ($%04X)\n", get_variable_address(arg1));
        fprintf(output, "\tCP %s\n", arg2);
        fprintf(output, "\tJP Z, l_%d\n", label_count);
        label_count++;
    } else if (sscanf(buffer, "goto %s;", arg1) == 1) {
        fprintf(output, "\tJP %s\n", arg1);
    }

    // Arithmetic and logical operations
    else if (sscanf(buffer, "%[^+]+;", name) == 1 && strstr(buffer, "++;")) {
        trim_whitespace(name);
        fprintf(output, "\tLD A, ($%04X)\n", get_variable_address(name));
        fprintf(output, "\tINC A\n");
        fprintf(output, "\tLD ($%04X), A\n", get_variable_address(name));
    } else if (sscanf(buffer, "%[^-]-;", name) == 1 && strstr(buffer, "--;")) {
        trim_whitespace(name);
        fprintf(output, "\tLD A, ($%04X)\n", get_variable_address(name));
        fprintf(output, "\tDEC A\n");
        fprintf(output, "\tLD ($%04X), A\n", get_variable_address(name));
    }

    // Functions
    else if (sscanf(buffer, "int %s(int %[^)])", name, arg1) == 2) {
        fprintf(output, "%s:\n", name); // Function label
        add_variable(arg1);            // Parameter as a variable
    } else if (strstr(buffer, "return ") == buffer) {
        sscanf(buffer, "return %s;", arg1);
        if (is_variable(arg1)) {
            fprintf(output, "\tLD A, (%04X)\n", get_variable_address(arg1)); // Load return value
        } else {
            fprintf(output, "\tLD A, %s\n", arg1); // Immediate return value
        }
        fprintf(output, "\tRET\n"); // End function
    }
    else if (sscanf(buffer, "%s = %s(%[^)])", arg1, name, arg2) == 3) {
    if (is_variable(arg2)) {
        fprintf(output, "\tLD HL, (%04X)\n", get_variable_address(arg2)); // Load parameter
    } else {
        fprintf(output, "\tLD HL, %s\n", arg2); // Immediate parameter
    }
    fprintf(output, "\tCALL %s\n", name);      // Call function
    fprintf(output, "\tLD (%04X), A\n", get_variable_address(arg1)); // Store return value
    }




    else if (strcmp(buffer, "}") == 0) {
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
