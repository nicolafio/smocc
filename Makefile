flags := -std=c++17 $(shell sdl2-config --cflags --libs) -lSDL2_ttf

out/smocc: obj/main.o obj/ui.o
	g++ -o out/smocc obj/main.o obj/ui.o $(flags)

obj/main.o: obj/
	g++ -o obj/main.o -c src/main.cc $(flags)

obj/ui.o: obj/
	g++ -o obj/ui.o -c src/ui.cc $(flags)

obj/:
	mkdir -p obj

clean:
	rm -rf out/smocc
	rm -rf obj
