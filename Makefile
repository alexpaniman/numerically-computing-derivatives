.PHONY: run
run: numerical-derivative-plotter.out
	./numerical-derivative-plotter.out

numerical-derivative-plotter.out: main.cpp
	g++ main.cpp -Icppgfplots --std=c++20 -o numerical-derivative-plotter.out
