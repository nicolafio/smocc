std_flag := -std=c++20
sdl2_cflags := $(shell sdl2-config --cflags)
sdl2_libs_flags := $(shell sdl2-config --libs)
sdl2_ttf_flag := -lSDL2_ttf
all_flags := $(std_flag) $(sdl2_cflags) $(sdl2_libs_flags) $(sdl2_ttf_flag)
all_sources := $(wildcard src/*.cc)
all_objects := $(patsubst src/%.cc,obj/%.o,$(all_sources))

out/smocc: $(all_objects)
	g++ -o out/smocc $(all_objects) $(all_flags)

obj/smocc.o: obj/
	g++ -o obj/smocc.o -c src/smocc.cc $(std_flag) $(sdl2_cflags)

obj/gfx.o: obj/
	g++ -o obj/gfx.o -c src/gfx.cc $(std_flag) $(sdl2_cflags)

obj/ui.o: obj/
	g++ -o obj/ui.o -c src/ui.cc $(std_flag) $(sdl2_cflags)

obj/game.o: obj/
	g++ -o obj/game.o -c src/game.cc $(std_flag) $(sdl2_cflags)

obj/player.o: obj/
	g++ -o obj/player.o -c src/player.cc $(std_flag) $(sdl2_cflags)

obj/enemies.o: obj/
	g++ -o obj/enemies.o -c src/enemies.cc $(std_flag) $(sdl2_cflags)

obj/bullets.o: obj/
	g++ -o obj/bullets.o -c src/bullets.cc $(std_flag) $(sdl2_cflags)

obj/explosions.o: obj/
	g++ -o obj/explosions.o -c src/explosions.cc $(std_flag) $(sdl2_cflags)

obj/:
	mkdir -p obj

clean:
	rm -rf out/smocc
	rm -rf obj
