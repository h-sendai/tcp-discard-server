PROG = discard-server
CPPFLAGS += -I.
CFLAGS = -g -O2 -Wall -std=gnu99

all: $(PROG)

OBJS += get_num.o
OBJS += host_info.o
OBJS += my_signal.o
OBJS += my_socket.o
OBJS += logUtil.o
OBJS += set_cpu.o

OBJS += $(PROG).o

$(PROG): $(OBJS)

clean:
	rm -f *.o $(PROG)
