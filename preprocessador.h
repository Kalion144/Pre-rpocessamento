#ifndef PREPROCESSADOR_H
#define PREPROCESSADOR_H

#include <stdio.h>

/*
 * Realiza o pre-processamento do arquivo Assembly.
 *
 * Remove:
 * - comentarios iniciados por # fora de strings;
 * - linhas vazias;
 * - espacos desnecessarios no inicio e no final;
 * - espacos consecutivos fora de strings;
 * - tabulacoes fora de strings.
 *
 * Preserva:
 * - conteudo das strings;
 * - rotulos;
 * - diretivas;
 * - instrucoes.
 *
 * Retorna 1 em caso de sucesso e 0 em caso de erro.
 */
int preprocessar(FILE *entrada, FILE *saida);

#endif
