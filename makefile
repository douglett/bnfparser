OUT = main.out
SRC = main.cpp

$(OUT): $(SRC)
	g++ -std=c++14 -Wall -o $(OUT) $(SRC)

run: $(OUT)
	./$(OUT)