/*
 * Experimento 5: a cara de um vazamento no LeakSanitizer.
 *
 * Sobrescrevemos um ponteiro de strdup sem dar free no anterior - o
 * mesmo erro que a redefinicao de tarefa cometeria se cmd_task nao
 * liberasse as strings antigas. O LSan aponta o bloco orfao e a pilha
 * de onde ele foi alocado.
 *
 * Compilar e rodar:
 *   gcc -Wall -Wextra -fsanitize=address -o exp5 experimentos/exp5_vazamento.c
 *   ./exp5   (o relatorio de leak sai no stderr ao final)
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
    char *nome = strdup("primeiro");
    /* redefinicao ERRADA: sobrescreve sem free(nome) */
    nome = strdup("segundo");
    printf("nome atual: %s (o bloco de \"primeiro\" ficou orfao)\n", nome);
    free(nome);
    return 0;
}
