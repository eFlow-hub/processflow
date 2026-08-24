CC = gcc
CFLAGS = -Wall -Wextra -g

processflow: processflow.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f processflow

test: processflow
	@for f in testes/*.pf; do \
		echo "===== $$f ====="; \
		./processflow $$f; \
		echo "===== fim $$f (status $$?) ====="; \
	done

.PHONY: clean test
