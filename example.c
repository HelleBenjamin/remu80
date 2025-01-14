uint8_t a = 2;
uint8_t b;
uint16_t* ptr = &a;
uint16_t* ptr2 = 1;

void main() {
    read(a, 0);
    foo(1, b);
}

uint8_t foo(uint8_t num, uint8_t& ret) {
    num++;
    return num;
}

void test() {
    foo(a, b);
}