CC=g++


surface : surface.cpp ifs.h
	${CC} -std=c++11 surface.cpp  -o surface -lGL -lGLEW -lGLU -lglut  -lm  -lopenblas -llapack

clean :
	rm -f surface *~ \#*\#
