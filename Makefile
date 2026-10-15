CC = g++
CFLAGS = -Iincl -lX11 -lpthread
src = WindowManager.cpp Object.cpp Renderer.cpp Rasterizer.cpp WireframeRasterizer.cpp FilledWireframeRasterizer.cpp

obj = $(addprefix obj/,$(src:.cpp=.o))
dobj = $(addprefix obj/,$(src:.cpp=Debug.o))

all: Main

clean:
	rm -f obj/*.o 

$(obj): obj/%.o : src/%.cpp
	$(CC) -c $< -o $@ $(CFLAGS)

$(dobj): obj/%Debug.o : src/%.cpp
	$(CC) -g -c $< -o $@ $(CFLAGS)

Main: Main.cpp $(obj)
	$(CC) Main.cpp $(obj) -o bin/Main $(CFLAGS)

Debug: Main.cpp $(dobj)
	$(CC) -g Main.cpp $(dobj) -o bin/Debug $(CFLAGS)