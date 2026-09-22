#!/bin/sh
set -eu

total=0
for arquivo in testes/corretos/*.asm; do
    base="${arquivo%.asm}"
    ./analisador "$arquivo" "$base.lex" >/dev/null
    grep -q '^<TK_EOF, EOF>' "$base.lex"
    grep -q '^Nenhum erro lexico encontrado\.$' "$base.err"
    total=$((total + 1))
done

for arquivo in testes/erros/*.asm; do
    base="${arquivo%.asm}"
    ./analisador "$arquivo" "$base.lex" >/dev/null
    grep -q '^<TK_EOF, EOF>' "$base.lex"
    if grep -q '^Nenhum erro lexico encontrado\.$' "$base.err"; then
        echo "Falha: era esperado erro em $arquivo" >&2
        exit 1
    fi
    total=$((total + 1))
done

echo "$total casos de teste executados com sucesso."
