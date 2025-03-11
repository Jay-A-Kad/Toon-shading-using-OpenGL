CXX = g++
CXXFLAGS = -Wall -std=c++17
LIBS = -lGL -lGLEW -lglut

all: cel_shaded_sphere

cel_shaded_sphere: cel_shaded_sphere.cpp
	$(CXX) $(CXXFLAGS) cel_shaded_sphere.cpp -o cel_shaded_sphere $(LIBS)

clean:
	rm -f cel_shaded_sphere

