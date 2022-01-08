# Makefile for the smash program
CC = g++
CFLAGS = -std=c++11 -Wall -Werror -pedantic-errors -DNDEBUG
CCLINK = $(CC)
OBJS = smash.o Commands.o signals.o
RM = rm -f
# Creating the  executable
smash: $(OBJS)
	$(CCLINK) -o smash $(OBJS)
# Creating the object files
commands.o: Commands.cpp Commands.h
smash.o: smash.cpp Commands.h signals.h
signals.o: signals.cpp signals.h Commands.h
# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.* smash

