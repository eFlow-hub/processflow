# ProcessFlow

Gerenciador simples de tarefas/processos — Infraestrutura de Software, Implementação 1.

Implementado e testado em **Ubuntu (WSL2, kernel Linux) com gcc 15.2**.

## Arquivos

| Arquivo | Responsabilidade |
|---|---|
| `processflow.c` | Todo o programa: parser, tabela de tarefas, execução (simples, sequential, parallel, pipe), redirecionamento, workdir, jobs em background |
| `Makefile` | Alvos `processflow` (padrão), `clean` e `test` |
| `testes/*.pf` | Workflows de teste (básico, grupos, redirecionamento, pipe, erros, sem exit) |
| `testes/nomes.txt` | Entrada usada pelo teste de redirecionamento |
| `evidencias.log` | Log das sessões de teste gravado com `script` |

## Compilar

```bash
make clean
make
```

## Executar

Modo interativo:

```bash
./processflow
```

Modo workflow:

```bash
./processflow testes/t1_basico.pf
```

## Testes

```bash
make test
```

Roda todos os arquivos `testes/*.pf` em sequência.

```bash
make check
```

Compara a saída dos workflows determinísticos (t2, t3, t5, t6) com os arquivos
em `testes/esperado/` via `diff` — pass/fail automático. t1 e t4 ficam de fora
porque `ls -l` e `wc -l` variam com o conteúdo do diretório.

## Comandos suportados

```
task <nome> <programa> [args...]   cadastra uma tarefa
run <nome>                         executa e espera
run sequential t1 t2 ...           executa em sequência
run parallel t1 t2 ...             executa em paralelo
run pipe t1 t2 ...                 conecta stdout->stdin
input|output|append <tarefa> <arquivo>
workdir <diretório>                muda o diretório corrente
start <tarefa>                     executa em background ([id] PID)
jobs                               lista jobs ativos
wait [jobId]                       espera um job (sem argumento: todos)
exit                               encerra; avisa se houver jobs em background
                                   (CTRL-D também encerra)
```
