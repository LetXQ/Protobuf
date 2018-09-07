all:  server client mymv
LIB_PATH = /usr/local/protobuf/lib/
LIBS = protobuf -lpthread
INCLUDES = /usr/local/protobuf/include/ -I./ -I./include/
FLAGS = std=c++11 -g
SERV_SRCS = ./src/server.cpp ./src/socket.cpp ./proto/user.pb.cc
CLNT_SRCS = ./src/client.cpp ./src/socket.cpp ./proto/user.pb.cc

proto_msg:
	protoc --cpp_out=. ./proto/user.proto
	
server: $(SERV_SRCS)
	g++ $^ -o $@ -$(FLAGS) -I$(INCLUDES) -L$(LIB_PATH) -l$(LIBS)
	
client: $(CLNT_SRCS)
	g++ $^ -o $@ -$(FLAGS) -I$(INCLUDES) -L$(LIB_PATH) -l$(LIBS)
	
.PHONY : clean
clean:
	rm -rf ./bin/* *.o
	
mymv:
	mv server client ./bin/