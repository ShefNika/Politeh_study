CC = gcc
CFLAGS = -Wall -O2
OBJDIR = ../../build/obj/test
BINDIR = ../../build/bin

TARGET = $(BINDIR)/test-app
OBJS = $(OBJDIR)/main.o $(OBJDIR)/funcs.o

.PHONY: all clean directories

all: directories $(TARGET)

directories:
	mkdir -p $(OBJDIR) $(BINDIR) 

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@  

$(OBJDIR)/main.o: main.c funcs.h
	$(CC) $(CFLAGS) -c main.c -o $@

$(OBJDIR)/funcs.o: funcs.c funcs.h
	$(CC) $(CFLAGS) -c funcs.c -o $@

clean:
	rm -f $(OBJDIR)/*.o $(TARGET)