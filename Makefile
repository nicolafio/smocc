std_flag := -std=c++20
sdl2_cflags := $(shell sdl2-config --cflags)
sdl2_libs_flags := $(shell sdl2-config --libs)
sdl2_image_flag := -lSDL2_image
sdl2_ttf_flag := -lSDL2_ttf
all_flags := $(std_flag) $(sdl2_cflags) $(sdl2_libs_flags) $(sdl2_image_flag) $(sdl2_ttf_flag)
all_sources := $(wildcard src/*.cc src/*/*.cc)
all_objects := $(patsubst src/%.cc,obj/%.o,$(all_sources))

out/smocc: $(all_objects)
	g++ -o out/smocc $(all_objects) $(all_flags)

obj/%.o: obj/
	g++ -o $@ -c $(patsubst obj/%.o,src/%.cc,$@) $(std_flag) $(sdl2_cflags)

obj/ui/%.o: obj/
	g++ -o $@ -c $(patsubst obj/ui/%.o,src/ui/%.cc,$@) $(std_flag) $(sdl2_cflags)

obj/:
	mkdir -p obj
	mkdir -p obj/ui

clean:
	rm -rf out/smocc
	rm -rf obj
