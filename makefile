CC = g++
CFLAGS = -c -Wall -g
LDFLAGS = 

AR = ar
ARFLAGS = -cvq

# list of given client/server files files
GIVEN_SOURCES = client.c server.c server_function_skels.c server_functions.c
GIVEN_OBJECTS = $(GIVEN_SOURCES:.c=.o)
EXECUTABLES = server client

# list of rpc source and objects
RPC_SOURCES = rpc.c helper.c
RPC_OBJECTS = $(RPC_SOURCES:.c=.o)
RPC_ARCHIVE = librpc.a

# make all
all: $(EXECUTABLES) $(RPC_ARCHIVE)

# make the archive
$(RPC_ARCHIVE): $(RPC_OBJECTS)
	@echo "making $@ ..."
	$(AR) $(ARFLAGS) $@ $^

# make the given client
client: $(GIVEN_OBJECTS)
	@echo "making $@ ... running the given command"
	g++ -L. client.o -lrpc -o client

# make the given server
server: $(GIVEN_OBJECTS)
	@echo "making $@ ... running the given command"
	g++ -L. server_functions.o server_function_skels.o server.o -lrpc -o server

# make clean
clean:
	rm -f *.o *.d $(EXECUTABLES) $(RPC_ARCHIVE)

# rule to transform .c to .o
%.o: %.c debug.h defines.h
	@echo "making $@ ..."
	$(CC) $(CFLAGS) $< -o $@



