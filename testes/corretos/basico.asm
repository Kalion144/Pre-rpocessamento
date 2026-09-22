.data
mensagem: .asciiz "Resultado: "
valor: .word 10
.text
main:
    li $v0, 4
    la $a0, mensagem
    syscall
