#
# Makefile
#
CC = gcc

TARGET = sw4s_ctrl

CFLAGS = -O0 -g3 -Wall

LIBS = -ludev

LDFLAGS =

OBJS = sw4s_ctrl.o sw4s_udev.o

.SUFFIXES: .o .c

all: $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $<

