#!/bin/bash
# make check: compara a saida de cada workflow deterministico com testes/esperado/<t>.out
# t1 e t4 ficam de fora: ls -l e wc -l variam com o conteudo do diretorio
cd "$(dirname "$0")/.." || exit 1
fail=0
for t in t2_grupos t3_redir t5_erros t6_sem_exit; do
    rm -f resultado.txt
    if diff -u "testes/esperado/$t.out" <(./processflow "testes/$t.pf" 2>&1); then
        echo "OK: $t"
    else
        echo "FALHOU: $t"
        fail=1
    fi
done
rm -f resultado.txt
exit $fail
