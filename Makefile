CC = g++
CFLAGS = -O3 -Iincl -lX11 -lpthread
src = WindowManager.cpp Object.cpp Renderer.cpp Rasterizer.cpp WireframeRasterizer.cpp FilledWireframeRasterizer.cpp

obj = $(addprefix obj/,$(src:.cpp=.o))

all: Main

$(obj): obj/%.o : src/%.cpp
	$(CC) -c $< -o $@ $(CFLAGS)

Main: Main.cpp $(obj)
	$(CC) Main.cpp $(obj) -o bin/Main $(CFLAGS)

Debug: Main.cpp $(obj)
	$(CC) -g Main.cpp $(obj) -o bin/Debug $(CFLAGS)