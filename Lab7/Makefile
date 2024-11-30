CC = gcc
CFLAGS = -Wall -O2


GAME = game
GUESS = guess

all: $(GAME) $(GUESS)

$(GAME): game.c
	$(CC) $(CFLAGS) -o $(GAME) game.c


$(GUESS): guess.c
	$(CC) $(CFLAGS) -o $(GUESS) guess.c

clean:
	rm -f $(GAME) $(GUESS)