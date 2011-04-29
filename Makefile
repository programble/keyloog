CC = clang

PROGRAM_VERSION := $(shell git describe --tags HEAD | cut -c2-)

override WARNINGS += -Wall -Wextra -Wno-unused-parameter
override CFLAGS += -std=c99 -D 'PROGRAM_VERSION="$(PROGRAM_VERSION)"'
override LIBS += -lm -lX11

OUTPUT = keyloog

$(OUTPUT): keyloog.c
	$(CC) $(CFLAGS) $(WARNINGS) $(LIBS) $< -o $@

clean:
	rm -f $(OUTPUT)

.PHONY: clean
