# Pre-processador e analisador lexico MIPS

Trabalho das etapas de pre-processamento e analise lexica para o subconjunto Assembly MIPS definido na disciplina.

## Compilacao

### Linux ou Git Bash

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic main.c preprocessador.c lexico.c -o analisador
./analisador teste.asm teste.lex
```

### Windows PowerShell (MinGW)

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic main.c preprocessador.c lexico.c -o analisador.exe
.\analisador.exe .\teste.asm .\teste.lex
```

O primeiro argumento e o arquivo de entrada Assembly e o segundo e o arquivo de
saida lexica. Por exemplo, `programa.asm` e `programa.lex` representam nomes
escolhidos pelo usuario, nao arquivos obrigatorios com esses nomes.

O programa cria automaticamente `programa.pre`, `programa.lex`, `programa.ts` e `programa.err`.

## Testes

```bash
make test
```

Os casos ficam em `testes/corretos` e `testes/erros`. A descricao da implementacao e dos testes esta em `RELATORIO_TECNICO.md`. O AFD foi entregue nos formatos DOT e PNG.
