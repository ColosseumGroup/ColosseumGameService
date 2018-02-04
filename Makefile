CC = gcc
CFLAGS = -O3 -Wall

PROGRAMS = all_in_expectation bm_run_matches dealer_poker dealer_renju

all: $(PROGRAMS)

clean:
	rm -f $(PROGRAMS)


all_in_expectation: all_in_expectation.c game.c game.h rng.c rng.h net.c net.h
	$(CC) $(CFLAGS) -o $@ all_in_expectation.c game.c rng.c net.c

bm_server: bm_server.c game.c game.h rng.c rng.h net.c net.h
	$(CC) $(CFLAGS) -o $@ bm_server.c game.c rng.c net.c

bm_widget: bm_widget.c net.c net.h
	$(CC) $(CFLAGS) -o $@ bm_widget.c net.c

bm_run_matches: bm_run_matches.c net.c net.h
	$(CC) $(CFLAGS) -o $@ bm_run_matches.c net.c

dealer_poker: game.c game.h evalHandTables rng.c rng.h dealerPoker.c net.c net.h
	$(CC) $(CFLAGS) -o $@ game.c rng.c dealerPoker.c net.c

dealer_renju: renju.c renju.h dealerRenju.c net.c net.h
	$(CC) $(CFLAGS) -o $@ renju.c dealerRenju.c net.c
