#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/*  
    ZC80 compiler for Z80

    ZC80 is a C-like language for the Z80 CPU. The ZC80 is almost fully compatible with C.

    Declare all variables before any functions

    HL as parameter for functions

    All variables treated as 8-bit

    In C:
        uint8_t a = 2;
        uint8_t b;

        uint8_t foo(uint8_t a) {
            a++;
            return a;
        }

        void main() {
            b = foo(a);
        }


*/


#define MAX_VARS 32
#define MAX_LINE_LENGTH 128

const char* init[] = {
    "\tLD SP, 0xFFFF\n",

};

const char* reservedKeywords[] = {
    "uint8_t", "void", "read", "write"
};

bool isReservedKeyword(const char *keyword) {
    for (int i = 0; i < sizeof(reservedKeywords) / sizeof(reservedKeywords[0]); i++) {
        if (strcmp(keyword, reservedKeywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

typedef struct {
    char name[16];
    int address;
} Variable;

typedef struct {
    char name[16];
    int return_addr;
} Function;

Function functions[MAX_VARS];
Variable variables[MAX_VARS];
int var_count = 0;
int func_count = 0;
int label_count = 0;


void add_variable(const char *name, int type) { // 1 = uint8_t, 2 = uint16_t*
    if (var_count >= MAX_VARS) {
        fprintf(stderr, "Error: Too many variables.\n");
        exit(1);
    }
    strcpy(variables[var_count].name, name);
    variables[var_count].address = 0x8000 + (var_count * type); // TODO
    var_count++;
}

void add_function(const char *name, int return_addr) {
    if (func_count >= MAX_VARS) {
        fprintf(stderr, "Error: Too many functions.\n");
        exit(1);
    }
    strcpy(functions[func_count].name, name);
    functions[func_count].return_addr = return_addr;
    func_count++;
}

void modify_ret_address(const char *name, int address) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(functions[i].name, name) == 0) {
            functions[i].return_addr = address;
        }
    }
}

bool is_variable(const char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

bool is_function(const char *name) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(functions[i].name, name) == 0) {
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

int get_function_ret_address(char *name) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(functions[i].name, name) == 0) {
            return functions[i].return_addr;
        }
    }
    fprintf(stderr, "Error: Undefined function '%s'.\n", name);
    exit(1);
}

bool startsWith(const char *str1, const char *str2) {
    while (*str2 != '\0') {
        if (*str1 != *str2) {
            return 1;
        }
        str1++;
        str2++;
    }
    return 0;
}

void removeChar(char *str, char garbage) {
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}

int checkIfPointer(char *name) {
    bool isPointer = false;
    if (startsWith(name, "&")) {
        isPointer = true;
    }
    removeChar(name, '&');
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            if (isPointer) return variables[i].address;
            else return 0;
        }
    }
    return 0;
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

void remove_leading_tabs_and_spaces(char *str) {
    char *start = str;
    while (*start == '\t' || *start == ' ') {
        start++;
    }
    memmove(str, start, strlen(start) + 1);
}


// Compile a single line
void compile_line(const char *line, FILE *output, int ind) {
    char keyword[16], name[16], arg1[16], arg2[16], fnName[16];
    char buffer[MAX_LINE_LENGTH];
    strcpy(buffer, line);
    remove_leading_tabs_and_spaces(buffer);
    trim_whitespace(buffer);

    if (strcmp(buffer, "\n") == 0) return;
    else if (strcmp(buffer, "}") == 0) return;
    else if (strlen(buffer) == 0) return;
    else if (strcmp(buffer, "//") == 0) return;
    else if (strcmp(buffer, "__halt;") == 0) fprintf(output, "\tHALT\n");

    else if (sscanf(buffer, "uint8_t %[^(](uint8_t %[^)]) {", fnName, arg1) == 2) { // function declaration
        add_function(fnName, 0);
        add_variable(arg1, 1);
        fprintf(output, "l_%s:\n", fnName); // func name
        fprintf(output, "\tLD ($%04X), HL\n", get_variable_address(arg1));
        add_variable(arg1, 1);
    }

    // Variables
    else if (sscanf(buffer, "uint8_t %s = %[^;];", arg1, arg2) == 2) { // initialized variable
        add_variable(arg1, 1);
        fprintf(output, "\tLD A, %s\n", arg2); // Load immediate value
        fprintf(output, "\tLD ($%04X), A\n", get_variable_address(arg1));
    } else if (sscanf(buffer, "uint8_t %[^;];", arg1) == 1) { // uninitialized variable
        add_variable(arg1, 1);
        fprintf(output, "\tLD A, 0\n"); // Load immediate value
        fprintf(output, "\tLD ($%04X), A\n", get_variable_address(arg1));
    } else if (sscanf(buffer, "uint16_t* %s = &%[^;];", arg1, arg2) == 2) { // pointer variable
        add_variable(arg1, 2);
        fprintf(output, "\tLD HL, $%04X\n", get_variable_address(arg2)); // Load address of variable
        fprintf(output, "\tLD ($%04X), HL\n", get_variable_address(arg1));
    } else if (sscanf(buffer, "uint16_t* %s = %[^;];", arg1, arg2) == 2) { // immediate pointer variable
        add_variable(arg1, 2);
        fprintf(output, "\tLD HL, %s\n", arg2); // Load address of variable
        fprintf(output, "\tLD ($%04X), HL\n", get_variable_address(arg1));
    }

    else if (sscanf(buffer, "void %[^(]() {", name) == 1) {
        fprintf(output, "l_%s:\n", name);
        label_count++;
    } else if (strcmp(buffer, "return;") == 0) { // Void return
        fprintf(output, "\tRET\n");
    } else if (sscanf(buffer, "if (%s == %s)", arg1, arg2) == 2) {
        fprintf(output, "\tLD A, ($%04X)\n", get_variable_address(arg1));
        fprintf(output, "\tCP %s\n", arg2);
        fprintf(output, "\tJP Z, l_%d\n", label_count);
        label_count++;
    } else if (sscanf(buffer, "goto %[^;];", arg1) == 1) {
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
    } else if (sscanf(buffer, "%s += %[^;];", name, arg1) == 2) {
        if (!isdigit(arg1[0])) { // Variable
            fprintf(output, "\tLD A, ($%04X)\n", get_variable_address(arg1));
        } else {
            fprintf(output, "\tLD A, %s\n", arg1); // Immediate value
        }
        fprintf(output, "\tLD HL, $%04X\n", get_variable_address(name));
        fprintf(output, "\tADD A, (HL)\n");
        fprintf(output, "\tLD ($%04X), A\n", get_variable_address(name));
    } else if (sscanf(buffer, "%s -= %[^;];", name, arg1) == 2) {
        if (!isdigit(arg1[0])) { // Variable
            fprintf(output, "\tLD A, ($%04X)\n", get_variable_address(arg1));
        } else {
            fprintf(output, "\tLD A, %s\n", arg1); // Immediate value
        }
        fprintf(output, "\tLD HL, $%04X\n", get_variable_address(name));
        fprintf(output, "\tSUB A, (HL)\n");
        fprintf(output, "\tLD ($%04X), A\n", get_variable_address(name));
    }

    // Input/Output
    else if (sscanf(buffer, "read(%[^,], %[^)]);", arg1, arg2) == 2) { // variable, port
        fprintf(output, "\tIN A, (%s)\n", arg2);
        fprintf(output, "\tLD ($%04X), A\n", get_variable_address(arg1));
    } else if (sscanf(buffer, "write(%[^,], %[^)]);", arg1, arg2) == 2) { // variable, port
        fprintf(output, "\tLD A, ($%04X)\n", get_variable_address(arg1));
        fprintf(output, "\tOUT (%s), A\n", arg2);
    }

    // Functions
    else if (strstr(buffer, "return ") == buffer) {
        sscanf(buffer, "return %[^;];", arg1);
        if (!isdigit(arg1[0])) { // Variable
            fprintf(output, "\tLD A, ($%04X)\n", get_variable_address(arg1));
        } else {
            fprintf(output, "\tLD A, %s\n", arg1); // Immediate return value
        }
        fprintf(output, "\tRET\n"); // End function
    }
    else if (sscanf(buffer, "%s = %[^(](%[^)]);", name, fnName, arg1) == 3) { // variable = function(arg1);
        if (isReservedKeyword(fnName)) return;
        //add_function(fnName, get_variable_address(arg2));
        // Save arg1
        if (is_variable(arg1)) {
            fprintf(output, "\tLD HL, ($%04X)\n", get_variable_address(arg1)); // Load parameter
        } else {
            fprintf(output, "\tLD HL, %s\n", arg1); // Immediate parameter
        }

        fprintf(output, "\tCALL l_%s\n", fnName);      // Call function

        // Load return value
        fprintf(output, "\tLD ($%04X), A\n", get_variable_address(name));

    } else if (sscanf(buffer, "%[^(]();", name) == 1) { // Call void function, fix
        if (isReservedKeyword(name)) return;
        fprintf(output, "\tCALL l_%s\n", name);
    }


    // Inline assembly
    else if (sscanf(buffer, "asm('%['])", arg1) == 1) {
        fprintf(output, "%s\n", arg1);
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
