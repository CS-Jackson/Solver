SOURCE  := $(wildcard *.cpp)
OBJS    := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

TARGET  := SolverServer
CC      := g++
LIBS    := -lmysqlclient -lpthread -lz -lm -lrt -ldl -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs
INCLUDE := -I/usr/local/include 
LDFLAGS := -L/usr/local/lib -L/usr/lib/x86_64-linux-gnu
CFLAGS  := -std=c++11 -g -Wall -O3 $(INCLUDE)
CXXFLAGS:= $(CFLAGS)

.PHONY : objs clean veryclean rebuild all
all : $(TARGET)
objs : $(OBJS)
rebuild: veryclean all
clean :
	rm -fr *.o
veryclean : clean
	rm -rf $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)