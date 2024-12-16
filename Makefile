SRC_DIR = src
all:
	g++ $(SRC_DIR)/main.cpp $(SRC_DIR)/z80e.cpp $(SRC_DIR)/loadHex.cpp -o main

assemble:
	vasmz80_oldstyle -Fhunk -dotdir -Fihex -o hello.hex hello.asm -L hello.lst