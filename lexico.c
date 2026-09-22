#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexico.h"

#define MAX_SIMBOLOS 256
#define TAM_CAMINHO 1024

typedef struct { const char *lexema; const char *token; } Reservada;

static const Reservada DIRETIVAS[] = {
    {".data","DIR_DATA"},{".text","DIR_TEXT"},{".word","DIR_WORD"},
    {".half","DIR_HALF"},{".byte","DIR_BYTE"},{".space","DIR_SPACE"},
    {".ascii","DIR_ASCII"},{".asciiz","DIR_ASCIIZ"},{".globl","DIR_GLOBL"},
    {".align","DIR_ALIGN"}
};
static const Reservada INSTRUCOES[] = {
    {"li","INS_LI"},{"la","INS_LA"},{"move","INS_MOVE"},{"lw","INS_LW"},
    {"sw","INS_SW"},{"lb","INS_LB"},{"sb","INS_SB"},{"lh","INS_LH"},
    {"sh","INS_SH"},{"add","INS_ADD"},{"addi","INS_ADDI"},{"addu","INS_ADDU"},
    {"sub","INS_SUB"},{"subu","INS_SUBU"},{"mul","INS_MUL"},{"div","INS_DIV"},
    {"mflo","INS_MFLO"},{"mfhi","INS_MFHI"},{"and","INS_AND"},{"or","INS_OR"},
    {"xor","INS_XOR"},{"nor","INS_NOR"},{"sll","INS_SLL"},{"srl","INS_SRL"},
    {"slt","INS_SLT"},{"beq","INS_BEQ"},{"bne","INS_BNE"},{"blt","INS_BLT"},
    {"ble","INS_BLE"},{"bgt","INS_BGT"},{"bge","INS_BGE"},{"j","INS_J"},
    {"jal","INS_JAL"},{"jr","INS_JR"},{"syscall","INS_SYSCALL"},{"nop","INS_NOP"}
};
static const char *REGISTRADORES[] = {
    "$zero","$v0","$v1","$a0","$a1","$a2","$a3",
    "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7","$t8","$t9",
    "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
    "$k0","$k1","$gp","$sp","$fp","$ra"
};

static Simbolo tabela[MAX_SIMBOLOS];
static int totalSimbolos;
static int erros;
static int erroSaida;
static char saidaTS[TAM_CAMINHO] = "saida.ts";
static char saidaErr[TAM_CAMINHO] = "saida.err";

static void copiar(char *destino, size_t tamanho, const char *origem) {
    if (tamanho == 0) return;
    strncpy(destino, origem, tamanho - 1);
    destino[tamanho - 1] = '\0';
}

static void minusculas(const char *origem, char *destino, size_t tamanho) {
    size_t i;
    for (i = 0; origem[i] != '\0' && i + 1 < tamanho; i++)
        destino[i] = (char)tolower((unsigned char)origem[i]);
    destino[i] = '\0';
}

static int buscarReservada(const Reservada *lista, size_t n, const char *lexema) {
    size_t i;
    for (i = 0; i < n; i++) if (strcmp(lista[i].lexema, lexema) == 0) return (int)i;
    return -1;
}

static int ehRegistrador(const char *lexema) {
    size_t i;
    if (lexema[0] == '$' && isdigit((unsigned char)lexema[1])) {
        char *fim;
        long valor = strtol(lexema + 1, &fim, 10);
        return *fim == '\0' && valor >= 0 && valor <= 31;
    }
    for (i = 0; i < sizeof(REGISTRADORES)/sizeof(REGISTRADORES[0]); i++)
        if (strcmp(REGISTRADORES[i], lexema) == 0) return 1;
    return 0;
}

static void adicionarSimbolo(const char *lexema, const char *categoria, int linha, int coluna) {
    int i;
    for (i = 0; i < totalSimbolos; i++) {
        if (strcmp(tabela[i].lexema, lexema) == 0 && strcmp(tabela[i].categoria, categoria) == 0) {
            if (tabela[i].primeiraLinha == 0 && linha > 0) {
                tabela[i].primeiraLinha = linha;
                tabela[i].primeiraColuna = coluna;
            }
            return;
        }
    }
    if (totalSimbolos >= MAX_SIMBOLOS) return;
    copiar(tabela[totalSimbolos].lexema, sizeof(tabela[totalSimbolos].lexema), lexema);
    copiar(tabela[totalSimbolos].categoria, sizeof(tabela[totalSimbolos].categoria), categoria);
    tabela[totalSimbolos].primeiraLinha = linha;
    tabela[totalSimbolos].primeiraColuna = coluna;
    totalSimbolos++;
}

static void inicializarTabela(void) {
    size_t i;
    char numero[16];
    totalSimbolos = 0;
    for (i = 0; i < sizeof(DIRETIVAS)/sizeof(DIRETIVAS[0]); i++)
        adicionarSimbolo(DIRETIVAS[i].lexema, "diretiva", 0, 0);
    for (i = 0; i < sizeof(INSTRUCOES)/sizeof(INSTRUCOES[0]); i++)
        adicionarSimbolo(INSTRUCOES[i].lexema, "instrucao", 0, 0);
    for (i = 0; i < sizeof(REGISTRADORES)/sizeof(REGISTRADORES[0]); i++)
        adicionarSimbolo(REGISTRADORES[i], "registrador", 0, 0);
    for (i = 0; i <= 31; i++) {
        snprintf(numero, sizeof(numero), "$%zu", i);
        adicionarSimbolo(numero, "registrador", 0, 0);
    }
}

static void registrarErro(FILE *arquivo, const char *tipo, const char *lexema, int linha, int coluna) {
    erros++;
    if (arquivo == NULL || fprintf(arquivo, "<%s, %s> %d %d\n", tipo, lexema, linha, coluna) < 0)
        erroSaida = 1;
}

static void anexar(char *lexema, int *pos, int ch) {
    if (*pos < 99) lexema[(*pos)++] = (char)ch;
    lexema[*pos] = '\0';
}

static void devolver(FILE *in, int ch) { if (ch != EOF) ungetc(ch, in); }

static int proximoEhDoisPontos(FILE *in) {
    int ch = fgetc(in);
    devolver(in, ch);
    return ch == ':';
}

static Token proximoToken(FILE *in, int *linha, int *coluna, FILE *arqErr) {
    Token t = {{0},{0},0,0};
    int ch, pos = 0;

inicio:
    ch = fgetc(in);
    while (ch != EOF && isspace((unsigned char)ch)) {
        if (ch == '\n') { (*linha)++; *coluna = 1; }
        else (*coluna)++;
        ch = fgetc(in);
    }
    if (ch == '#') {
        while ((ch = fgetc(in)) != EOF && ch != '\n') {}
        if (ch == '\n') { (*linha)++; *coluna = 1; }
        goto inicio;
    }
    t.linha = *linha; t.coluna = *coluna;
    if (ch == EOF) { copiar(t.nome,sizeof(t.nome),"TK_EOF"); copiar(t.lexema,sizeof(t.lexema),"EOF"); return t; }

    if (ch == ',' || ch == ':' || ch == '(' || ch == ')') {
        anexar(t.lexema,&pos,ch); (*coluna)++;
        copiar(t.nome,sizeof(t.nome), ch==','?"SMB_COM":ch==':'?"SMB_COL":ch=='('?"SMB_OPA":"SMB_CPA");
        return t;
    }

    if (ch == '"') {
        int fechada = 0, escapeInvalido = 0;
        anexar(t.lexema,&pos,ch); (*coluna)++;
        while ((ch = fgetc(in)) != EOF) {
            if (ch == '\n') break;
            anexar(t.lexema,&pos,ch); (*coluna)++;
            if (ch == '"') { fechada = 1; break; }
            if (ch == '\\') {
                int esc = fgetc(in);
                if (esc == EOF || esc == '\n') { ch = esc; break; }
                anexar(t.lexema,&pos,esc); (*coluna)++;
                if (esc!='n' && esc!='t' && esc!='"' && esc!='\\' && esc!='0') escapeInvalido = 1;
            }
        }
        if (!fechada) {
            registrarErro(arqErr,"ERRO_STRING_NAO_FECHADA",t.lexema,t.linha,t.coluna);
            copiar(t.nome,sizeof(t.nome),"ERRO");
            if (ch == '\n') { (*linha)++; *coluna = 1; }
        } else {
            copiar(t.nome,sizeof(t.nome),"STRING");
            if (escapeInvalido) registrarErro(arqErr,"ERRO_ESCAPE_INVALIDO",t.lexema,t.linha,t.coluna);
        }
        return t;
    }

    if (ch == '$') {
        char normal[100];
        anexar(t.lexema,&pos,ch); (*coluna)++;
        while ((ch=fgetc(in))!=EOF && (isalnum((unsigned char)ch)||ch=='_')) { anexar(t.lexema,&pos,ch); (*coluna)++; }
        devolver(in,ch); minusculas(t.lexema,normal,sizeof(normal));
        if (ehRegistrador(normal)) {
            copiar(t.nome,sizeof(t.nome),"REG"); adicionarSimbolo(normal,"registrador",t.linha,t.coluna);
        } else { registrarErro(arqErr,"ERRO_REGISTRADOR_INVALIDO",t.lexema,t.linha,t.coluna); copiar(t.nome,sizeof(t.nome),"ERRO"); }
        return t;
    }

    if (ch == '.') {
        char normal[100]; int indice;
        anexar(t.lexema,&pos,ch); (*coluna)++;
        while ((ch=fgetc(in))!=EOF && (isalnum((unsigned char)ch)||ch=='_')) { anexar(t.lexema,&pos,ch); (*coluna)++; }
        devolver(in,ch); minusculas(t.lexema,normal,sizeof(normal));
        indice=buscarReservada(DIRETIVAS,sizeof(DIRETIVAS)/sizeof(DIRETIVAS[0]),normal);
        if (indice>=0) { copiar(t.nome,sizeof(t.nome),DIRETIVAS[indice].token); adicionarSimbolo(normal,"diretiva",t.linha,t.coluna); }
        else { registrarErro(arqErr,"ERRO_DIRETIVA_INVALIDA",t.lexema,t.linha,t.coluna); copiar(t.nome,sizeof(t.nome),"ERRO"); }
        return t;
    }

    if (isdigit((unsigned char)ch) || ch == '-') {
        int inicioNegativo = ch == '-';
        anexar(t.lexema,&pos,ch); (*coluna)++;
        if (inicioNegativo) {
            ch=fgetc(in);
            if (ch==EOF || !isdigit((unsigned char)ch)) { devolver(in,ch); registrarErro(arqErr,"ERRO_CARACTERE_INVALIDO","-",t.linha,t.coluna); copiar(t.nome,sizeof(t.nome),"ERRO"); return t; }
            anexar(t.lexema,&pos,ch); (*coluna)++;
        }
        while ((ch=fgetc(in))!=EOF && (isalnum((unsigned char)ch)||ch=='_')) { anexar(t.lexema,&pos,ch); (*coluna)++; }
        devolver(in,ch);
        {
            const char *p=t.lexema+(inicioNegativo?1:0); int valido=1;
            if (p[0]=='0' && (p[1]=='x'||p[1]=='X')) {
                int i=2; if (p[i]=='\0') valido=0;
                for (;p[i];i++) if (!isxdigit((unsigned char)p[i])) valido=0;
            } else {
                int i=0; for (;p[i];i++) if (!isdigit((unsigned char)p[i])) valido=0;
            }
            if (valido) copiar(t.nome,sizeof(t.nome),"NUM_INT");
            else {
                const char *tipo;
                if (p[0]=='0' && (p[1]=='x'||p[1]=='X')) tipo="ERRO_NUMERO_MALFORMADO";
                else if (strpbrk(p,"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_")) tipo="ERRO_IDENTIFICADOR_MALFORMADO";
                else tipo="ERRO_NUMERO_MALFORMADO";
                registrarErro(arqErr,tipo,t.lexema,t.linha,t.coluna); copiar(t.nome,sizeof(t.nome),"ERRO");
            }
        }
        return t;
    }

    if (isalpha((unsigned char)ch) || ch == '_') {
        char normal[100]; int indice;
        anexar(t.lexema,&pos,ch); (*coluna)++;
        while ((ch=fgetc(in))!=EOF && (isalnum((unsigned char)ch)||ch=='_')) { anexar(t.lexema,&pos,ch); (*coluna)++; }
        devolver(in,ch); minusculas(t.lexema,normal,sizeof(normal));
        indice=buscarReservada(INSTRUCOES,sizeof(INSTRUCOES)/sizeof(INSTRUCOES[0]),normal);
        if (indice>=0) {
            copiar(t.nome,sizeof(t.nome),INSTRUCOES[indice].token);
            adicionarSimbolo(normal,"instrucao",t.linha,t.coluna);
            if (proximoEhDoisPontos(in)) registrarErro(arqErr,"ERRO_IDENTIFICADOR_MALFORMADO",t.lexema,t.linha,t.coluna);
        } else {
            copiar(t.nome,sizeof(t.nome),"ID");
            if (proximoEhDoisPontos(in)) adicionarSimbolo(t.lexema,"identificador/rotulo",t.linha,t.coluna);
        }
        return t;
    }

    anexar(t.lexema,&pos,ch); (*coluna)++;
    registrarErro(arqErr,"ERRO_CARACTERE_INVALIDO",t.lexema,t.linha,t.coluna);
    copiar(t.nome,sizeof(t.nome),"ERRO");
    return t;
}

static void gerarTS(void) {
    FILE *f=fopen(saidaTS,"w"); int i;
    if (f==NULL) { erroSaida=1; return; }
    fprintf(f,"LEXEMA,CATEGORIA,LINHA,COLUNA\n");
    for (i=0;i<totalSimbolos;i++)
        fprintf(f,"%s,%s,%d,%d\n",tabela[i].lexema,tabela[i].categoria,tabela[i].primeiraLinha,tabela[i].primeiraColuna);
    if (ferror(f) || fclose(f)!=0) erroSaida=1;
}

void configurarSaidasLexicas(const char *ts, const char *err) {
    copiar(saidaTS,sizeof(saidaTS),ts); copiar(saidaErr,sizeof(saidaErr),err);
}

void AnaliseLexica(FILE *in, FILE *out) {
    FILE *arqErr; int linha=1,coluna=1; Token t;
    erroSaida=0; erros=0; inicializarTabela();
    arqErr=fopen(saidaErr,"w");
    if (arqErr==NULL) erroSaida=1;
    do {
        t=proximoToken(in,&linha,&coluna,arqErr);
        if (strcmp(t.nome,"ERRO")!=0 && fprintf(out,"<%s, %s> %d %d\n",t.nome,t.lexema,t.linha,t.coluna)<0) erroSaida=1;
    } while (strcmp(t.nome,"TK_EOF")!=0);
    if (arqErr!=NULL) {
        if (erros==0) fprintf(arqErr,"Nenhum erro lexico encontrado.\n");
        if (ferror(arqErr) || fclose(arqErr)!=0) erroSaida=1;
    }
    if (ferror(in) || ferror(out)) erroSaida=1;
    gerarTS();
}

int houveErroDeSaidaLexica(void) { return erroSaida; }
