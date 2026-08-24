/*
 * ProcessFlow - gerenciador simples de tarefas/processos
 * Infraestrutura de Software - Implementacao 1
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_TASKS 64
#define MAX_ARGS 32
#define MAX_LINE 1024

typedef struct {
    char name[64];
    char *argv[MAX_ARGS + 1]; /* argv[0] = programa, termina em NULL */
} Task;

static Task tasks[MAX_TASKS];
static int ntasks = 0;

static Task *find_task(const char *name) {
    for (int i = 0; i < ntasks; i++)
        if (strcmp(tasks[i].name, name) == 0)
            return &tasks[i];
    return NULL;
}

static void free_task_argv(Task *t) {
    for (int i = 0; t->argv[i] != NULL; i++)
        free(t->argv[i]);
    t->argv[0] = NULL;
}

/* task <nome> <programa> [args...] */
static void cmd_task(char **tok, int n) {
    if (n < 3) {
        fprintf(stderr, "uso: task <nome> <programa> [args...]\n");
        return;
    }
    if (n - 2 > MAX_ARGS) {
        fprintf(stderr, "task: numero de argumentos excede o limite (%d)\n", MAX_ARGS);
        return;
    }
    Task *t = find_task(tok[1]);
    if (t != NULL) {
        free_task_argv(t); /* redefinicao substitui a anterior */
    } else {
        if (ntasks == MAX_TASKS) {
            fprintf(stderr, "task: limite de %d tarefas atingido\n", MAX_TASKS);
            return;
        }
        t = &tasks[ntasks++];
        snprintf(t->name, sizeof(t->name), "%s", tok[1]);
    }
    int i;
    for (i = 2; i < n; i++)
        t->argv[i - 2] = strdup(tok[i]);
    t->argv[i - 2] = NULL;
}

/* corpo do filho: nunca retorna */
static void child_exec(Task *t) {
    execvp(t->argv[0], t->argv);
    fprintf(stderr, "processflow: nao foi possivel executar '%s': %s\n",
            t->argv[0], strerror(errno));
    _exit(127);
}

/* informa termino anormal sem travar o loop */
static void report_status(const char *name, int status) {
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        fprintf(stderr, "tarefa '%s' terminou com codigo %d\n", name, WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        fprintf(stderr, "tarefa '%s' morta pelo sinal %d\n", name, WTERMSIG(status));
}

static void run_one(const char *name) {
    Task *t = find_task(name);
    if (t == NULL) {
        fprintf(stderr, "run: tarefa '%s' nao existe\n", name);
        return;
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0)
        child_exec(t);
    int status;
    waitpid(pid, &status, 0);
    report_status(name, status);
}

/* run <nome> */
static void cmd_run(char **tok, int n) {
    if (n < 2) {
        fprintf(stderr, "uso: run <nome>\n");
        return;
    }
    run_one(tok[1]);
}

/* retorna 1 quando a linha pede o encerramento (exit) */
static int process_line(char *line) {
    char *tok[MAX_ARGS + 4];
    int n = 0;
    char *p = strtok(line, " \t\n");
    while (p != NULL && n < MAX_ARGS + 3) {
        tok[n++] = p;
        p = strtok(NULL, " \t\n");
    }
    if (n == 0)
        return 0; /* linha vazia ou so espacos */
    if (strcmp(tok[0], "exit") == 0)
        return 1;
    if (strcmp(tok[0], "task") == 0)
        cmd_task(tok, n);
    else if (strcmp(tok[0], "run") == 0)
        cmd_run(tok, n);
    else
        fprintf(stderr, "comando desconhecido: %s\n", tok[0]);
    return 0;
}

int main(void) {
    char line[MAX_LINE];
    for (;;) {
        printf("processflow> ");
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL)
            break; /* CTRL-D: exit implicito */
        if (process_line(line))
            break;
    }
    return 0;
}
