CC=gcc
XX=g++
SOURCES=$(wildcard ../PFRPSchedII/*.cpp)
OBJS=$(addprefix ./,$(notdir $(SOURCES:.cpp=.o)))
CFLAGS=-Wall -O -g -std=c++0x
TARGET=./pfrpschedii

./%.o:../PFRPSchedII/%.cpp
	$(XX) $(CFLAGS) -c $< -o $@	

$(TARGET):$(OBJS)
	$(XX) $(OBJS) -o $(TARGET)
clean:
# to fix "clock skew" problem
#	find . -type f | xargs -n 5 touch
	rm -rf *.o *. *~ pfrpschedii

