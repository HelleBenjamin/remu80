uint8_t a = 2;
uint8_t b = 0;
uint8_t ret = 0;

goto main;

uint8_t foo(uint8_t num, uint8_t& ret) {
    num++;
    return num;
}

void test() {
    foo(a, b);
}

void main() {
    foo(1, b);     
}