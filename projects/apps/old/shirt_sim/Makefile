OUT=tut
.default = all
all: final

final:
	gcc -o tut main.cpp $(shell pkg-config --cflags --libs gtk+-3.0 gmodule-2.0)

