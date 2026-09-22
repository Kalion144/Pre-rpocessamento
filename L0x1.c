#include <stdio.h>

int reconhece_L(char *s);

int main(int argc, char **argv) {
    int i;

    for (i = 1; i < argc; i++) {
        int teste = reconhece_L(argv[i]);
        printf("%s\t%s\n", argv[i], teste ? "ACEITA" : "REJEITA");
    }

    return 0;
}

int reconhece_L(char *s) {
    int i = 0;
    int n0 = 0;
    int n1 = 0;

    if (s == NULL) {
        return 1;
    }

    while (s[i] == '0') {
        n0++;
        i++;
    }

    while (s[i] == '1') {
        n1++;
        i++;
    }

    if (s[i] != '\0') {
        return 0;
    }

    return n0 == n1;
}