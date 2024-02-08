snake:
	g++ ../lodepng/lodepng.cpp snake.cpp -o snake.exe -std=c++20 -I../sdl/include -L../sdl/lib -lSDL2main -lSDL2 -I../lodepng