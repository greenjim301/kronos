CXX = g++
#2、定义您自己的可执行文件名称
PROGRAM_NAME=kronos
##################################################################### #
#3、指定您必须生成的工程文件

SOURCE = $(wildcard *.cpp) \
         $(wildcard k_util/*.cpp)

OBJECTS = $(SOURCE:.cpp=.o) 
CFLAGS = -g

.PHONY: all
all: $(PROGRAM_NAME)

clean:
	@echo "[Cleanning...]"
	@rm -f $(OBJECTS) $(PROGRAM_NAME)

%.o: %.cpp
	$(CXX) $(CFLAGS) -o $@ -c $<


$(PROGRAM_NAME): $(OBJECTS)
	$(CXX) $(CFLAGS) -o $@ $^ -lpthread
