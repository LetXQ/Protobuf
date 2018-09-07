all:  server client
LIB_PATH = /usr/local/protobuf/lib/
LIBS = protobuf -lpthread
INCLUDES = /usr/local/protobuf/include/ -I./proto -I.
FLAGS = std=c++11 -g
SERV_SRCS = ./server.cpp ./proto/user.pb.cc
CLNT_SRCS = ./client.cpp ./proto/user.pb.cc

proto_msg:
	protoc --cpp_out=. ./proto/user.proto
	
server: $(SERV_SRCS)
	g++ $^ -o $@ -$(FLAGS) -I$(INCLUDES) -L$(LIB_PATH) -l$(LIBS)
	
client: $(CLNT_SRCS)
	g++ $^ -o $@ -$(FLAGS) -I$(INCLUDES) -L$(LIB_PATH) -l$(LIBS)
	
clean:
	rm -rf server client *.o