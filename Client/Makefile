TARGET=Client
CC=g++
CFLAGS=-Wall

.PHONY : clean

$(TARGET): Main.o Client.o
	@$(CC) $(CFLAGS) -o $@ $^
	@echo "Use 'make run' to execute"

Main.o: Main.cpp Client.h
	@$(CC) $(CFLAGS) -c Main.cpp

Client.o: Client.cpp Client.h
	@$(CC) $(CFLAGS) -c Client.cpp

run: $(TARGET)
	@./$(TARGET)

clean:
	@rm *.o $(TARGET)
