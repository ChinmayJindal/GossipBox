TARGET=client
CC=gcc

$(TARGET): main.o client.o
	@$(CC) main.o client.o -o client

main.o: main.cpp client.h
	@$(CC) -c main.cpp

client.o: client.cpp client.h
	@$(CC) -c client.cpp

run: $(TARGET)
	@./$(TARGET)

clean:
	@rm *.o $(TARGET)
