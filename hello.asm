    .ORG 0x0000

MAIN:
    LD SP, 0x2000
    LD HL, TESTMSG  ; Load address of TESTMSG
    CALL PRINT
    CALL GETSTR_SIZE
    LD A, C ; Load size to A
    ADD A, '0' ; Convert to ASCII
    OUT (0x00), A
    CALL NEWLINE
    HALT

NEWLINE:
    PUSH AF ; Reserve current state
    LD A, 0x0D
    OUT (0x00), A
    LD A, 0x0A
    OUT (0x00), A
    POP AF
    RET

PRINT:
    PUSH HL

    .printloop:
    LD A, (HL)  ; Load char pointed by HL
    CP 0    ; Check if end
    JR Z, .printend  ; Return if end
    OUT (0x00), A   ; Print to stdout
    INC HL  ; Next char
    JR .printloop

    .printend:
    POP HL
    RET

GETINPUT:
    PUSH AF
    IN A, (0x00)
    CP 0
    JR Z, GETINPUT
    POP AF
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

GETSTR_SIZE:
    PUSH HL ; Save string pointer
    LD BC, 0
    .loop2:
        LD A, (HL)
        CP 0 ; Check if end
        JR Z, .end2
        INC HL
        INC BC ; Size in BC
        JR .loop2
    .end2:
    POP HL ; Restore string pointer
    INC BC ; String size + terminator
    RET

STRBUF .EQU 0x1000

MSG:
    .BYTE "Hello World!", 0DH, 0AH, 0
MSG2:
    .BYTE "Copied data", 0DH, 0AH, 0

TESTMSG:
    .BYTE "Hello", 0