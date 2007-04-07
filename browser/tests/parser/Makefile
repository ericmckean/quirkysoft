
CFILES:=$(wildcard *.cpp)
OFILES:=$(CFILES:%.cpp=%.o)
CPPFLAGS += -I../../arm9/source/ -g

all : tester run

clean:
	rm -f *.o tester

tester: $(OFILES) $(wildcard ../../arm9/build.sdl/*Parser.o)  ../../arm9/build.sdl/UTF8.o ../../arm9/build.sdl/Entity.o ../Main.o
	g++ -o $@ $^ -L.. -lcppunit -ldl

run: tester
	@./$<
