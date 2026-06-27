CC = gcc
CFLAGS = -Wall -g
SOURCESDIR = sources
SOURCESFILES = compteur.c
SOURCES = $(addprefix $(SOURCESDIR)/, $(SOURCESFILES))
HEADERSDIR = sources/headers
HEADERSFILES = fnsdefs.h struct.h
HEADERS =  $(addprefix $(HEADERSDIR)/, $(HEADERSFILES))

OBJECTS = $(SOURCES:.c=.o)
TARGET = Compte-minute

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ -lncursesw -lpulse-simple -lpulse -lm

$(SOURCESDIR)/%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJECTS)
