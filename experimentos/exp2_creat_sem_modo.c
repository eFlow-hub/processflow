/*
 * Experimento 2: O_CREAT sem o terceiro argumento de open.
 *
 * O modo (permissoes) do open e um argumento variadico: so e lido quando
 * as flags tem O_CREAT. Se for omitido, o kernel recebe LIXO da pilha
 * como permissoes. Aqui criamos dois arquivos em /tmp (ext4 de verdade;
 * /mnt/c finge permissoes) e comparamos os modos resultantes.
 *
 * As flags ficam numa variavel para o compilador nao conseguir detectar
 * estaticamente a falta do modo (com literal, o glibc/gcc barram).
 *
 * Compilar e rodar:
 *   gcc -Wall -Wextra -o exp2 experimentos/exp2_creat_sem_modo.c
 *   ./exp2
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

static void mostra(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0)
        printf("%s -> modo %04o\n", path, st.st_mode & 07777);
    else
        perror(path);
}

int main(void) {
    int flags = O_WRONLY | O_CREAT | O_TRUNC;

    unlink("/tmp/exp2_errado.txt");
    unlink("/tmp/exp2_certo.txt");

    int fd1 = open("/tmp/exp2_errado.txt", flags); /* sem modo: lixo */
    if (fd1 >= 0) close(fd1);

    int fd2 = open("/tmp/exp2_certo.txt", flags, 0644); /* modo explicito */
    if (fd2 >= 0) close(fd2);

    puts("permissoes resultantes (esperado: errado = lixo, certo = 0644):");
    mostra("/tmp/exp2_errado.txt");
    mostra("/tmp/exp2_certo.txt");
    return 0;
}
