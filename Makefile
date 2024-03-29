flags := -std=c++17 $(shell sdl2-config --cflags --libs) -lSDL2_ttf

out/smocc: obj/smocc.o obj/gfx.o obj/ui.o obj/game.o obj/player.o
	g++ -o out/smocc obj/gfx.o obj/smocc.o obj/ui.o obj/game.o obj/player.o $(flags)

obj/smocc.o: obj/
	g++ -o obj/smocc.o -c src/smocc.cc $(flags)

obj/gfx.o: obj/
	g++ -o obj/gfx.o -c src/gfx.cc $(flags)

obj/ui.o: obj/
	g++ -o obj/ui.o -c src/ui.cc $(flags)

obj/game.o: obj/
	g++ -o obj/game.o -c src/game.cc $(flags)

obj/player.o: obj/
	g++ -o obj/player.o -c src/player.cc $(flags)

obj/:
	mkdir -p obj

clean:
	rm -rf out/smocc
	rm -rf obj
