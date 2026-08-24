/*
 * Experimento 1: por que o filho usa _exit apos um exec que falhou.
 *
 * Aqui o filho tenta executar um programa inexistente e, DE PROPOSITO,
 * nao chama _exit depois do exec falhar. Resultado: o filho "sobra" e
 * continua executando o codigo do pai a partir dali - a mensagem final
 * aparece DUAS vezes.
 *
 * Compilar e rodar:
 *   gcc -Wall -Wextra -o exp1 experimentos/exp1_exit_vs_return.c
 *   ./exp1
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    printf("antes do fork (aparece 1x)\n");
    fflush(stdout);

    pid_t pid = fork();
    if (pid == 0) {
        char *argv[] = { "programa_que_nao_existe", NULL };
        execv("/bin/programa_que_nao_existe", argv);
        fprintf(stderr, "filho: exec falhou (%s) e eu NAO vou chamar _exit\n",
                strerror(errno));
        /* o certo seria: _exit(127); */
    } else {
        wait(NULL);
    }

    printf("fim do main (deveria aparecer 1x... aparece 2x!)\n");
    return 0;
}
