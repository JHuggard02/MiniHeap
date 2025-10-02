CC = clang

C_SRC = alloc.c
ASM_SRC = heap.s

C_OBJ = alloc.o
ASM_OBJ = heap.o

TARGET = miniHeap

all: $(TARGET)

$(C_OBJ): $(C_SRC)
	$(CC) -c $(C_SRC) -o $(C_OBJ)

$(ASM_OBJ): $(ASM_SRC)
	$(CC) -c $(ASM_SRC) -o $(ASM_OBJ)

$(TARGET): $(C_OBJ) $(ASM_OBJ)
	$(CC) $(C_OBJ) $(ASM_OBJ) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(C_OBJ) $(ASM_OBJ) $(TARGET)
