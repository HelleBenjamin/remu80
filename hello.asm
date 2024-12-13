main:
    ld a, 'H'
    out (0), a ; output to stdout
    ld a, 'e'
    out (0), a
    ld a, 'l'
    out (0), a
    ld a, 'l'
    out (0), a
    ld a, 'o'
    out (0), a
    ld a, ' '
    out (0), a
    ld a, 'W'
    out (0), a
    ld a, 'o'
    out (0), a
    ld a, 'r'
    out (0), a
    ld a, 'l'
    out (0), a
    ld a, 'd'
    out (0), a
    ld a, '!'
    out (0), a
    ld a, '\n'
    out (0), a
    
    ld a, '0'
    ld c, 48
    push af
    
loop:
    pop af
    out (0), a
    inc a
    dec c
    push af
    ld a, c
    jp nz, loop
    jr z, end
    
    
end:
    ld a, '\n'
    out (0), a
    halt