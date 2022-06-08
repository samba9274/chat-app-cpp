all: main

main: main.cpp
	g++ -o main main.cpp -lncurses -lsioclient -lboost_system -lboost_thread -lpthread