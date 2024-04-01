flags := -std=c++20 $(shell sdl2-config --cflags --libs) -lSDL2_ttf
all_sources := $(wildcard src/*.cc)
all_objects := $(patsubst src/%.cc,obj/%.o,$(all_sources))

out/smocc: $(all_objects)
	g++ -o out/smocc $(all_objects) $(flags)

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

obj/enemies.o: obj/
	g++ -o obj/enemies.o -c src/enemies.cc $(flags)

obj/:
	mkdir -p obj

clean:
	rm -rf out/smocc
	rm -rf obj
