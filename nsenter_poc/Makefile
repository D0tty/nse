CC = gcc
CFLAGS = -Wall -Werror -Wextra -O0 -g3
CPPFLAGS = -D_GNU_SOURCE

all: mini_nsenter mini_nsenter_fork

mini_nsenter:

debug_mini_nsenter: CFLAGS += -DDEBUG
debug_mini_nsenter: mini_nsenter

mini_nsenter_fork:

debug_mini_nsenter_fork: CFLAGS += -DDEBUG
debug_mini_nsenter_fork: mini_nsenter_fork

clean:
	${RM} mini_nsenter
	${RM} mini_nsenter_fork
