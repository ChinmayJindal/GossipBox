TARGET=Server
CC=g++
CFLAGS=-Wall

.PHONY : clean

$(TARGET): Main.o Server.o
	@$(CC) $(CFLAGS) -o $@ $^
	@echo "Use 'make run' to execute"

Main.o: Main.cpp
	@$(CC) $(CFLAGS) -c Main.cpp 

Server.o: Server.h Server.cpp
	@$(CC) $(CFLAGS) -c Server.cpp

run: $(TARGET)
	@./$(TARGET) 

clean:
	@rm *.o $(TARGET)
