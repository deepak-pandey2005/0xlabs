CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O0

all: app hook_strcmp.so

app: app.c
	$(CC) $(CFLAGS) app.c -o app

hook_strcmp.so: hook_strcmp.c
	$(CC) $(CFLAGS) -fPIC -shared hook_strcmp.c -o hook_strcmp.so -ldl

clean:
	rm -f app hook_strcmp.so
