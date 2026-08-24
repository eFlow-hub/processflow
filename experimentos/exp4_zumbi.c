/*
 * Experimento 4: zumbi de verdade, visivel no ps.
 *
 * O filho morre na hora; o pai espera 1s SEM chamar wait e mostra o ps:
 * o filho aparece como Z (defunct) - uma entrada morta guardando so o
 * codigo de saida. Depois do waitpid a entrada some.
 *
 * Compilar e rodar:
 *   gcc -Wall -Wextra -o exp4 experimentos/exp4_zumbi.c
 *   ./exp4
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static void mostra_filhos(const char *momento) {
    char cmd[128];
    printf("%s:\n", momento);
    fflush(stdout);
    snprintf(cmd, sizeof(cmd),
             "ps -o pid,stat,comm --ppid %d | sed 's/^/    /'", getpid());
    system(cmd);
}

int main(void) {
    pid_t pid = fork();
    if (pid == 0)
        _exit(42); /* morre imediatamente */

    sleep(1); /* garante que o filho ja morreu; pai ainda nao colheu */
    mostra_filhos("antes do wait (esperado: STAT Z, defunct)");

    int status;
    waitpid(pid, &status, 0);
    printf("colhido: codigo de saida %d\n", WEXITSTATUS(status));

    mostra_filhos("depois do wait (esperado: nenhum filho)");
    return 0;
}
