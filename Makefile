all: build/server

clean:
	@rm -rf build/*.o
	@rm -rf build/server

build/main.o: main.c server.h
	gcc -c -o build/main.o main.c

build/server.o: server.c server.h
	gcc -c -o build/server.o server.c

build/server: build/main.o build/server.o
	gcc -o build/server $^
