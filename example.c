uint8_t a = 2;
uint8_t b = 3;

void main() {
    a = foo(a);
    a += 3;
    b = bar(a);
    a += b;
    __halt;
}

uint8_t foo(uint8_t num) {
    num++;
    return num;
}

uint8_t bar(uint8_t num) {
    num--;
    return num;
}
