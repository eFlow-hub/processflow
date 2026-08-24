/*
 * Experimento 3: o travamento classico do pipe.
 *
 * O filho executa `wc -l` lendo do pipe; o pai escreve tres linhas.
 * - Modo errado (padrao): o pai NAO fecha a ponta de escrita depois de
 *   escrever. O wc nunca recebe EOF e o programa fica pendurado, sem
 *   nenhuma mensagem de erro. Rodar com `timeout 3` para comprovar.
 * - Modo certo (`./exp3 certo`): o pai fecha p[1]; o wc imprime 3 e o
 *   programa termina na hora.
 *
 * Compilar e rodar:
 *   gcc -Wall -Wextra -o exp3 experimentos/exp3_pipe_travado.c
 *   timeout 3 ./exp3        # trava; timeout mata com status 124
 *   ./exp3 certo            # termina normalmente
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int certo = (argc > 1 && strcmp(argv[1], "certo") == 0);
    int p[2];
    if (pipe(p) < 0) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        dup2(p[0], STDIN_FILENO);
        /* o filho fecha AS DUAS pontas originais; se deixasse p[1]
         * aberto, travaria a si mesmo esperando um EOF que ele
         * proprio impede */
        close(p[0]);
        close(p[1]);
        execlp("wc", "wc", "-l", (char *)NULL);
        perror("exec wc");
        _exit(127);
    }

    write(p[1], "um\ndois\ntres\n", 13);
    close(p[0]);
    if (certo)
        close(p[1]); /* sem esta linha o wc nunca ve EOF */
    else
        fprintf(stderr, "modo errado: pai mantem p[1] aberto... vou travar\n");

    waitpid(pid, NULL, 0);
    puts("terminou (so aparece no modo certo)");
    return 0;
}
