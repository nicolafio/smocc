build:
	mkdir -p build
	g++ -o build/smocc src/main.cc $(shell sdl2-config --cflags --libs)

clean:
	rm -r build
