#!/bin/bash
# Testes oficiais da disciplina (testes.tar):
# teste1 e teste2 sao interativos (stdin via pipe, sem prompt),
# teste3 e batch (modo workflow, linhas ecoadas antes de processar).
cd "$(dirname "$0")/.." || exit 1
fail=0

# os arquivos -saida.txt do professor nao terminam com newline; nossa
# saida termina toda linha com \n (padrao Unix). sed '$a\' acrescenta o
# newline final que falta no esperado antes de comparar.
for n in 1 2; do
    if diff -u <(sed -e '$a\' "testes/oficiais/teste$n-saida.txt") \
              <(./processflow < "testes/oficiais/teste$n-entrada.txt"); then
        echo "OK: teste$n (interativo)"
    else
        echo "FALHOU: teste$n (interativo)"
        fail=1
    fi
done

rm -f saida.txt
if diff -u <(sed -e '$a\' "testes/oficiais/teste3-saida.txt") \
          <(./processflow "testes/oficiais/teste3-entrada.txt"); then
    echo "OK: teste3 (batch)"
else
    echo "FALHOU: teste3 (batch)"
    fail=1
fi
rm -f saida.txt
exit $fail
