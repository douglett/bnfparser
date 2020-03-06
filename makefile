OUT = main.out
SRC = main.cpp
HEAD = *.hpp

$(OUT): $(SRC) $(HEAD)
	g++ -std=c++14 -Wall -o $(OUT) $(SRC)

run: $(OUT)
	./$(OUT)