# Exemplo de programa

.data

mensagem:    .asciiz    "Resultado # obtido"    # mensagem exibida
valor:       .word      10

.text

main:

    li    $t0,    5       # primeiro numero
    li    $t1,    10      # segundo numero
    add   $t2,    $t0,    $t1

# comentario sozinho
