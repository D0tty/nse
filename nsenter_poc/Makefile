CC = gcc
CFLAGS = -Wall -Werror -Wextra -O0 -g3
CPPFLAGS = -D_GNU_SOURCE
SRC = mini_nsenter.c
BIN = mini_nsenter

all: ${BIN}

mini_nsenter:

debug_mini_nsenter: CFLAGS += -DDEBUG
debug_mini_nsenter: mini_nsenter

clean:
	${RM} ${BIN}
