#include <stdio.h>
int x = 10;
int y = 20;

void main() {
    if (x == y) {
        goto equal;
    }
}
goto end;

equal:
    int z = 1;

end: