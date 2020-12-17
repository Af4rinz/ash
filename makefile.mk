PROGRAM = ash
FLAGS = -w -lreadline -pthread -o $(PROGRAM)
CC = g++
build:
	$(CC) $(PROGRAM).cpp $(FLAGS)