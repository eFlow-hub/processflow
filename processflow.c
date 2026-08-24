/*
 * ProcessFlow - gerenciador simples de tarefas/processos
 * Infraestrutura de Software - Implementacao 1
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_TASKS 64
#define MAX_ARGS 32
#define MAX_LINE 1024

typedef struct {
    char name[64];
    char *argv[MAX_ARGS + 1]; /* argv[0] = programa, termina em NULL */
    char *infile;             /* redirecionamento de entrada (ou NULL) */
    char *outfile;            /* redirecionamento de saida (ou NULL) */
    int append;               /* 0 = output (trunca), 1 = append */
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
    free(t->infile);
    free(t->outfile);
    t->infile = t->outfile = NULL;
    t->append = 0;
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

/* aplica os redirecionamentos registrados; em caso de falha o filho
 * sai sem executar o programa */
static void setup_redirects(Task *t) {
    if (t->infile != NULL) {
        int fd = open(t->infile, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "input: %s: %s\n", t->infile, strerror(errno));
            _exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (t->outfile != NULL) {
        int flags = O_WRONLY | O_CREAT | (t->append ? O_APPEND : O_TRUNC);
        int fd = open(t->outfile, flags, 0644);
        if (fd < 0) {
            fprintf(stderr, "output: %s: %s\n", t->outfile, strerror(errno));
            _exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

/* corpo do filho: nunca retorna */
static void child_exec(Task *t) {
    setup_redirects(t);
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

/* input|output|append <tarefa> <arquivo>: so registra; quem aplica e o run */
static void cmd_redirect(char **tok, int n) {
    if (n != 3) {
        fprintf(stderr, "uso: %s <tarefa> <arquivo>\n", tok[0]);
        return;
    }
    Task *t = find_task(tok[1]);
    if (t == NULL) {
        fprintf(stderr, "%s: tarefa '%s' nao existe\n", tok[0], tok[1]);
        return;
    }
    if (strcmp(tok[0], "input") == 0) {
        free(t->infile);
        t->infile = strdup(tok[2]);
    } else {
        free(t->outfile);
        t->outfile = strdup(tok[2]);
        t->append = (strcmp(tok[0], "append") == 0);
    }
}

/* sequencial: fork+wait um a um; paralelo: fork todos, depois wait todos */
static void run_group(char **names, int n, int paralelo) {
    if (!paralelo) {
        for (int i = 0; i < n; i++)
            run_one(names[i]);
        return;
    }
    pid_t pids[MAX_ARGS];
    for (int i = 0; i < n; i++) {
        Task *t = find_task(names[i]);
        if (t == NULL) {
            fprintf(stderr, "run: tarefa '%s' nao existe\n", names[i]);
            pids[i] = -1;
            continue;
        }
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            continue;
        }
        if (pids[i] == 0)
            child_exec(t);
    }
    /* espera na ordem de criacao: quem ja terminou e colhido na hora */
    for (int i = 0; i < n; i++) {
        if (pids[i] <= 0)
            continue;
        int status;
        waitpid(pids[i], &status, 0);
        report_status(names[i], status);
    }
}

/* run <nome> | run sequential <t...> | run parallel <t...> */
static void cmd_run(char **tok, int n) {
    if (n < 2) {
        fprintf(stderr, "uso: run <nome> | run sequential|parallel <tarefas...>\n");
        return;
    }
    if (strcmp(tok[1], "sequential") == 0 || strcmp(tok[1], "parallel") == 0) {
        if (n < 3) {
            fprintf(stderr, "uso: run %s <tarefa1> [tarefa2...]\n", tok[1]);
            return;
        }
        run_group(tok + 2, n - 2, strcmp(tok[1], "parallel") == 0);
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
    else if (strcmp(tok[0], "input") == 0 || strcmp(tok[0], "output") == 0 ||
             strcmp(tok[0], "append") == 0)
        cmd_redirect(tok, n);
    else if (strcmp(tok[0], "workdir") == 0) {
        if (n != 2)
            fprintf(stderr, "uso: workdir <diretorio>\n");
        else if (chdir(tok[1]) != 0)
            fprintf(stderr, "workdir: %s: %s\n", tok[1], strerror(errno));
    }
    else
        fprintf(stderr, "comando desconhecido: %s\n", tok[0]);
    return 0;
}

int main(int argc, char *argv[]) {
    FILE *in = stdin;
    int interativo = 1;

    if (argc > 2) {
        fprintf(stderr, "uso: %s [workflow.pf]\n", argv[0]);
        return 1;
    }
    if (argc == 2) {
        in = fopen(argv[1], "r");
        if (in == NULL) {
            fprintf(stderr, "processflow: %s: %s\n", argv[1], strerror(errno));
            return 1;
        }
        interativo = 0;
    }

    char line[MAX_LINE];
    for (;;) {
        if (interativo) {
            printf("processflow> ");
            fflush(stdout);
        }
        if (fgets(line, sizeof(line), in) == NULL)
            break; /* CTRL-D ou fim do workflow: exit implicito */
        if (!interativo) {
            fputs(line, stdout); /* imprime a linha antes de processar */
            if (line[strlen(line) - 1] != '\n')
                putchar('\n');
        }
        if (process_line(line))
            break;
    }
    if (!interativo)
        fclose(in);
    return 0;
}
