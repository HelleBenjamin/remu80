    .ORG 0x0000

MAIN:
    LD SP, 0x2000
    LD HL, MSG  ; Load address of MSG
    CALL PRINT
    LD A, '0'
    OUT (0x00), A
    LD (0x1000), A
    LD IX, 0xFFF
    INC (IX+1)
    LD A, (0x1000)
    OUT (0x00), A
    HALT

PRINT:
    LD A, (HL)  ; Load char pointed by HL
    CP 0    ; Check if end
    RET Z   ; Return if end
    OUT (0x00), A   ; Print to stdout
    INC HL  ; Next char
    JR PRINT

MSG:
    .DB "Hello World!", 0DH, 0AH, 0