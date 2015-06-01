CXX = clang++
CXXFLAGS = -std=c++11 -g -Wall
LDFLAGS = -pthread

.PHONY: main # compile regardless of file changes
main: 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) main.cpp -o main

.PHONY: clean
clean:
	-rm main
