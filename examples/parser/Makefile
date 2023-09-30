TARGET := parser

CPP := g++

CPP_FLAGS := -I../../src -Wall -Werror -ggdb -fsanitize=address

CPP_SRCS := ${wildcard *.cpp ../../src/*.cpp}

CPP_OBJS := ${CPP_SRCS:.cpp=.o}

%.o: %.cpp
	${CPP} ${CPP_FLAGS} -c -o $@ $<

${TARGET}: ${CPP_OBJS}
	${CPP} ${CPP_FLAGS} $^ -o $@

all: ${TARGET}

clean:
	rm -f ${TARGET} ${CPP_OBJS}
