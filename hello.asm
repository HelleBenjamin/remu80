    .ORG 0x0000

MAIN:
    LD SP, 0x2000
    LD HL, MSG  ; Load address of MSG
    CALL PRINT
    LD HL, STRBUF
    CALL GETSTR
    CALL PRINT
    LD BC, 0x1234
    PUSH BC
    POP AF
    HALT

PRINT:
    LD A, (HL)  ; Load char pointed by HL
    CP 0    ; Check if end
    RET Z   ; Return if end
    OUT (0x00), A   ; Print to stdout
    INC HL  ; Next char
    JR PRINT

GETINPUT:
    IN A, (0x00)
    CP 0
    JR Z, GETINPUT
    RET

GETSTR:
    PUSH HL ; Save string pointer
    .loop1:
        IN A, (0x00)
        CP 0
        JR Z, .loop1
        CP 0AH ; Check if enter
        JR Z, .end1
        LD (HL), A
        INC HL
        JR .loop1
    .end1:
    LD (HL), 0
    POP HL ; Restore string pointer
    RET

STRBUF .EQU 0x1000

MSG:
    .BYTE "Hello World!", 0DH, 0AH, 0