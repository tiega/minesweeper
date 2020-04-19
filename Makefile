TARGET=minesweeper

$(TARGET): main.cc
	g++ main.cc -o $(TARGET)

.PHONY: clean
clean:
	rm $(TARGET)
