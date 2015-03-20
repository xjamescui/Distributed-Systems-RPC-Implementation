CC = g++
CFLAGS = -c -Wall -g
LDFLAGS = 

AR = ar
ARFLAGS = -cvq

# list of given client/server files files
GIVEN_SOURCES = client1.c server.c server_function_skels.c server_functions.c
SERVER_SOURCES = server.c server_function_skels.c server_functions.c
CLIENT_SOURCES = client1.c client2.c

GIVEN_OBJECTS = $(GIVEN_SOURCES:.c=.o)
SERVER_OBJECTS = $(SERVER_SOURCES:.c=.o)
CLIENT_OBJECTS = $(CLIENT_SOURCES:.c=.o)

EXECUTABLES = server client

# list of rpc sources and objects
RPC_SOURCES = helper.c host_database.c
RPC_CPP_SOURCES = rpc_client.cc ClientCacheDatabase.cc rpc_server.cc SkeletonDatabase.cc
RPC_OBJECTS = $(RPC_SOURCES:.c=.o)
RPC_CPP_OBJECTS = $(RPC_CPP_SOURCES:.cc=.o)
RPC_ARCHIVE = librpc.a

# list of binder sources and objects
BINDER_SOURCES = binder.c host_database.c helper.c
BINDER_OBJECTS = $(BINDER_SOURCES:.c=.o)
BINDER_EXECUTABLE = binder

# make all
all: $(RPC_ARCHIVE) binder $(EXECUTABLES)

# make the archive
$(RPC_ARCHIVE): $(RPC_OBJECTS) $(RPC_CPP_OBJECTS)
	@echo "making $@ ..."
	$(AR) $(ARFLAGS) $@ $^

# make binder
$(BINDER_EXECUTABLE): $(BINDER_OBJECTS)
	@echo "making $@ ..."
	$(CC) $(LDFLAGS) $^ -o $@

# make the given client
# client: $(CLIENT_OBJECTS) $(RPC_ARCHIVE)
# 	@echo "making $@ ... running the given command"
# 	g++ -L. client1.o -lrpc -lpthread -o client
# 	g++ -L. client2.o -lrpc -lpthread -o client

# # make the given server
# server: $(SERVER_OBJECTS) $(RPC_ARCHIVE)
# 	@echo "making $@ ... running the given command"
# 	g++ -L. server_functions.o server_function_skels.o server.o -lrpc -lpthread -o server

# make clean
clean:
	rm -f *.o *.d $(BINDER_EXECUTABLE) $(RPC_ARCHIVE)

# rule to transform .c to .o
%.o: %.c debug.h defines.h
	@echo "making $@ ..."
	$(CC) $(CFLAGS) $< -o $@

# rule to transform .cc to .o
%.o: %.cc debug.h defines.h
	@echo "making $@ ..."
	$(CC) $(CFLAGS) $< -o $@
