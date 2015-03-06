CC = g++
CFLAGS = -c -Wall -g
LDFLAGS = 

AR = ar
ARFLAGS = -cvq

# list of given client/server files files
GIVEN_SOURCES = client1.c server.c server_function_skels.c server_functions.c
GIVEN_OBJECTS = $(GIVEN_SOURCES:.c=.o)
EXECUTABLES = server client

# list of rpc sources and objects
RPC_SOURCES = rpc.c helper.c
RPC_OBJECTS = $(RPC_SOURCES:.c=.o)
RPC_ARCHIVE = librpc.a

# list of binder sources and objects
BINDER_SOURCES = binder.c helper.c
BINDER_OBJECTS = $(BINDER_SOURCES:.c=.o)
BINDER_EXECUTABLE = binder

# make all
all: $(RPC_ARCHIVE) binder $(EXECUTABLES)

# make the archive
$(RPC_ARCHIVE): $(RPC_OBJECTS)
	@echo "making $@ ..."
	$(AR) $(ARFLAGS) $@ $^

# make binder
$(BINDER_EXECUTABLE): $(BINDER_OBJECTS)
	@echo "making $@ ..."
	$(CC) $(LDFLAGS) $^ -o $@

# make the given client
client: $(GIVEN_OBJECTS)
	@echo "making $@ ... running the given command"
	g++ -L. client1.o -lrpc -o client

# make the given server
server: $(GIVEN_OBJECTS)
	@echo "making $@ ... running the given command"
	g++ -L. server_functions.o server_function_skels.o server.o -lrpc -o server

# make clean
clean:
	rm -f *.o *.d $(EXECUTABLES) $(BINDER_EXECUTABLE) $(RPC_ARCHIVE)

# rule to transform .c to .o
%.o: %.c debug.h defines.h
	@echo "making $@ ..."
	$(CC) $(CFLAGS) $< -o $@



