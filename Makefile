CC = gcc
CFLAGS = -Wall -Wextra -g -I./include
SRC = src/main.c src/shell.c src/parser.c src/builtin.c src/execute.c src/redirection.c src/pipeline.c src/jobs.c
OBJ = $(SRC:.c=.o)
EXEC = myshell

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f src/*.o $(EXEC)