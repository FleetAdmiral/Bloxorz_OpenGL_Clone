game: Sample_GL3_2D.cpp glad.c
	g++ -g -std=c++11 -Wall -o game Sample_GL3_2D.cpp glad.c -lGL -lglfw -ldl -I"./include" -L"/usr/lib" ./libIrrKlang.so -pthread

clean:
	@rm -f game
