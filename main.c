#include <stdio.h>
#include <string.h>
#include "preprocessador.h"
#include "lexico.h"

#define TAM_CAMINHO 1024

static int trocarExtensao(const char *origem, const char *extensao,
                          char *destino, size_t tamanho) {
    const char *barra1 = strrchr(origem, '/');
    const char *barra2 = strrchr(origem, '\\');
    const char *barra = barra1 > barra2 ? barra1 : barra2;
    const char *ponto = strrchr(origem, '.');
    size_t base = strlen(origem);

    if (ponto != NULL && (barra == NULL || ponto > barra)) base = (size_t)(ponto - origem);
    if (base + strlen(extensao) + 1 > tamanho) return 0;
    memcpy(destino, origem, base);
    destino[base] = '\0';
    strcat(destino, extensao);
    return 1;
}

int main(int argc, char *argv[]) {
    FILE *entrada, *pre, *inLex, *outLex;
    char caminhoPre[TAM_CAMINHO], caminhoTS[TAM_CAMINHO], caminhoErr[TAM_CAMINHO];

    if (argc != 3) {
        fprintf(stderr, "Erro: quantidade de argumentos invalida.\n");
        fprintf(stderr, "Uso: %s <arquivo_entrada.asm> <arquivo_saida.lex>\n", argv[0]);
        return 1;
    }
    if (!trocarExtensao(argv[2], ".pre", caminhoPre, sizeof(caminhoPre)) ||
        !trocarExtensao(argv[2], ".ts", caminhoTS, sizeof(caminhoTS)) ||
        !trocarExtensao(argv[2], ".err", caminhoErr, sizeof(caminhoErr))) {
        fprintf(stderr, "Erro: caminho de saida muito longo.\n");
        return 1;
    }

    entrada = fopen(argv[1], "r");
    if (entrada == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s'.\n", argv[1]);
        return 1;
    }
    pre = fopen(caminhoPre, "w");
    if (pre == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar '%s'.\n", caminhoPre);
        fclose(entrada);
        return 1;
    }
    if (!PreProcessamento(entrada, pre)) {
        fprintf(stderr, "Erro durante o pre-processamento.\n");
        fclose(entrada); fclose(pre);
        return 1;
    }
    fclose(entrada);
    if (fclose(pre) != 0) {
        fprintf(stderr, "Erro ao gravar '%s'.\n", caminhoPre);
        return 1;
    }

    inLex = fopen(caminhoPre, "r");
    outLex = fopen(argv[2], "w");
    if (inLex == NULL || outLex == NULL) {
        fprintf(stderr, "Erro ao abrir arquivos da analise lexica.\n");
        if (inLex != NULL) fclose(inLex);
        if (outLex != NULL) fclose(outLex);
        return 1;
    }
    configurarSaidasLexicas(caminhoTS, caminhoErr);
    AnaliseLexica(inLex, outLex);
    if (fclose(inLex) != 0 || fclose(outLex) != 0 || houveErroDeSaidaLexica()) {
        fprintf(stderr, "Erro durante a leitura ou gravacao da analise lexica.\n");
        return 1;
    }

    printf("Processamento concluido.\n");
    printf("Pre-processado: %s\nTokens: %s\nTabela: %s\nErros: %s\n",
           caminhoPre, argv[2], caminhoTS, caminhoErr);
    return 0;
}
