TARGET=Client
CC=g++
CFLAGS=-Wall
THREAD=-lpthread

.PHONY : clean

$(TARGET): Main.o Client.o
	@$(CC) $(CFLAGS) -o $@ $^ $(THREAD)
	@echo "Use 'make run' to execute"

Main.o: Main.cpp Client.h
	@$(CC) $(CFLAGS) $(THREAD) -c Main.cpp

Client.o: Client.cpp Client.h
	@$(CC) $(CFLAGS) $(THREAD) -c Client.cpp

run: $(TARGET)
	@./$(TARGET)

clean:
	@rm *.o $(TARGET)
