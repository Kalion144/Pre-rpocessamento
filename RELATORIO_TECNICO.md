# Relatorio tecnico - Pre-processador e analisador lexico MIPS

## 1. Organizacao do projeto

O programa foi dividido em tres modulos. `main.c` valida os argumentos, cria os nomes dos arquivos de saida e coordena as duas etapas. `preprocessador.c` remove comentarios e normaliza espacos, preservando o conteudo das strings. `lexico.c` implementa o AFD, a tabela de simbolos e os arquivos `.lex`, `.ts` e `.err`.

As estruturas `Token` e `Simbolo` ficam declaradas em `lexico.h`. Um token guarda nome, lexema, linha e coluna inicial. Um simbolo guarda lexema, categoria e posicao da primeira ocorrencia.

## 2. Funcoes principais

- `PreProcessamento`: le o arquivo Assembly, remove comentarios e normaliza os espacos fora de strings.
- `AnaliseLexica`: inicializa a tabela, solicita tokens ate `TK_EOF` e gera as tres saidas da etapa lexica.
- `proximoToken`: percorre a entrada caractere por caractere e realiza as transicoes do AFD.
- `adicionarSimbolo`: impede duplicatas e atualiza a primeira ocorrencia de simbolos pre-carregados.
- `registrarErro`: registra o erro e permite que a analise continue no proximo lexema.
- `gerarTS`: escreve a tabela completa em CSV.

## 3. AFD

O estado inicial ignora espacos e escolhe o caminho de acordo com o primeiro caractere. Letras e sublinhado levam ao estado de identificador ou instrucao. Ponto inicia diretiva, cifrao inicia registrador, digito inicia numero, sinal de menos inicia numero negativo, aspas iniciam string e `#` inicia comentario. Virgula, dois-pontos e parenteses chegam diretamente a estados finais.

Os estados de numero distinguem decimal e hexadecimal. O prefixo `0x` somente aceita digitos hexadecimais. A string possui um estado proprio para escape. Lexemas invalidos chegam aos estados de erro, sao registrados e nao impedem o reconhecimento dos tokens seguintes. O arquivo `afd.dot` contem a representacao completa e `afd.png` e a imagem para apresentacao.

## 4. Linha e coluna

A contagem comeca em linha 1 e coluna 1. Cada caractere consumido aumenta a coluna. A quebra de linha aumenta a linha e redefine a coluna para 1. A posicao e copiada para o token antes do consumo de seu primeiro caractere. O `TK_EOF` recebe a posicao atual ao final da entrada.

## 5. Erros e recuperacao

Sao reconhecidos caractere invalido, identificador malformado, numero malformado, diretiva invalida, registrador invalido, string nao fechada e escape invalido. O analisador consome o lexema problematico, grava o registro em `.err` e continua. Assim, o arquivo apresenta todos os erros encontrados, e nao somente o primeiro.

## 6. Tabela de simbolos

A tabela inicia com todas as diretivas, instrucoes e formas de registradores aceitas. Esses itens ficam com linha e coluna zero ate sua primeira utilizacao. Palavras reservadas e registradores sao normalizados para minusculas. Identificadores permanecem sensiveis a maiusculas e minusculas e somente sao inseridos como rotulos quando seguidos de dois-pontos. Numeros, strings e pontuacao nao entram na tabela.

## 7. Testes

Foram criados seis programas. Os tres corretos cobrem o programa basico, comentarios e escapes em strings, numeros decimais e hexadecimais e enderecamento `deslocamento(registrador)`. Os tres incorretos cobrem registradores, caractere invalido, diretiva, numeros, identificadores, escapes, string nao fechada e palavra reservada usada como rotulo.

O script `testes/executar_testes.sh` compila e executa todos os casos. Para entradas corretas, verifica `TK_EOF` e ausencia de erros. Para entradas incorretas, verifica `TK_EOF` e a existencia de ao menos um erro, comprovando a recuperacao.

## 8. Saidas

Para cada entrada sao produzidos:

- `.pre`: codigo normalizado pelo pre-processador;
- `.lex`: tokens no formato `<TOKEN, lexema> linha coluna`, incluindo `TK_EOF`;
- `.ts`: tabela completa no formato CSV;
- `.err`: todos os erros ou a mensagem de que nenhum erro foi encontrado.

As saidas geradas pelos seis casos de teste estao nas mesmas pastas dos arquivos `.asm`.
