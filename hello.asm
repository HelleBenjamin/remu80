    .ORG 0x0000

MAIN:
    LD SP, 0x2000
    LD HL, MSG  ; Load address of MSG
    CALL PRINT
    LD BC, 0x1234
    PUSH BC
    LD IX, 0x199D
    INC (IX+1)
    POP BC
    OUT (0x01), A    
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