#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "preprocessador.h"

#define TAM_LINHA 4096

/*
 * Remove comentarios e normaliza espacos da linha.
 *
 * O processamento considera o estado "dentro de string".
 * Assim, um # dentro de aspas nao inicia comentario.
 */
static void processar_linha(const char *linha, char *resultado) {
    int dentro_string = 0;
    int escapado = 0;
    int espaco_pendente = 0;
    int iniciou_conteudo = 0;
    size_t i;
    size_t pos = 0;

    for (i = 0; linha[i] != '\0'; i++) {
        char c = linha[i];

        /* Trata quebras de linha de diferentes sistemas operacionais. */
        if (c == '\n' || c == '\r') {
            break;
        }

        /*
         * Dentro de uma string, todos os caracteres sao preservados.
         * Aspas escapadas nao encerram a string.
         */
        if (dentro_string) {
            if (espaco_pendente) {
                resultado[pos++] = ' ';
                espaco_pendente = 0;
            }

            resultado[pos++] = c;

            if (c == '"' && !escapado) {
                dentro_string = 0;
            }

            if (c == '\\' && !escapado) {
                escapado = 1;
            } else {
                escapado = 0;
            }

            iniciou_conteudo = 1;
            continue;
        }

        /* Inicio de uma string. */
        if (c == '"') {
            if (espaco_pendente && iniciou_conteudo) {
                resultado[pos++] = ' ';
                espaco_pendente = 0;
            }

            resultado[pos++] = c;
            dentro_string = 1;
            escapado = 0;
            iniciou_conteudo = 1;
            continue;
        }

        /* # inicia comentario somente fora de uma string. */
        if (c == '#') {
            break;
        }

        /* Tabulacoes e espacos sao tratados como separadores. */
        if (c == ' ' || c == '\t') {
            if (iniciou_conteudo) {
                espaco_pendente = 1;
            }
            continue;
        }

        /*
         * Se havia espaco antes de um novo caractere, mantemos apenas
         * um espaco. Isso normaliza sequencias de espacos.
         */
        if (espaco_pendente) {
            resultado[pos++] = ' ';
            espaco_pendente = 0;
        }

        resultado[pos++] = c;
        iniciou_conteudo = 1;
    }

    /*
     * O espaco pendente nao e escrito aqui. Dessa forma, os espacos
     * no final da linha sao eliminados.
     */
    resultado[pos] = '\0';
}

/*
 * Verifica se uma linha possui algum conteudo.
 */
static int linha_vazia(const char *linha) {
    size_t i;

    for (i = 0; linha[i] != '\0'; i++) {
        if (!isspace((unsigned char)linha[i])) {
            return 0;
        }
    }

    return 1;
}

int preprocessar(FILE *entrada, FILE *saida) {
    char linha[TAM_LINHA];
    char processada[TAM_LINHA];

    if (entrada == NULL || saida == NULL) {
        return 0;
    }

    while (fgets(linha, sizeof(linha), entrada) != NULL) {
        processar_linha(linha, processada);

        /*
         * Linhas vazias, inclusive linhas que continham somente
         * comentarios, nao sao copiadas para a saida.
         */
        if (linha_vazia(processada)) {
            continue;
        }

        fprintf(saida, "%s\n", processada);
    }

    if (ferror(entrada) || ferror(saida)) {
        return 0;
    }

    return 1;
}
