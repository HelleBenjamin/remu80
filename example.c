uint8_t a = 2;
uint8_t b;

void main() {
    read(a, 0);
    foo(1, b);
    bar(2, b);
    test();
    read(a, 1);
}

uint8_t foo(uint8_t num, uint8_t& ret) {
    num++;
    return num;
}

uint8_t bar(uint8_t num, uint8_t& ret) {
    num--;
    return num;
}

void test() {
    foo(a, b);
    bar(a, b);
    return;
}