.data
valores: .word 15, -20, 0xFF, -0x20
.text
main:
    lw $t0, 4($sp)
    sw $t0, 8($sp)
    jr $ra
