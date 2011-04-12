CC = clang

override WARNINGS += -Wall -Wextra -Wno-unused-parameter
override CFLAGS += -std=c99
override LIBS += -lm -lX11

OUTPUT = keycounter

$(OUTPUT): keycounter.c
	$(CC) $(CFLAGS) $(WARNINGS) $(LIBS) $< -o $@

clean:
	rm -f $(OUTPUT)

.PHONY: clean
