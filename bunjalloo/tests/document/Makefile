
CFILES:=$(wildcard *.cpp)
OFILES:=$(CFILES:%.cpp=%.o)
CPPFLAGS += -I../../arm9/source/ -g

all : tester run

LINKFILES:=Document.o \
	   UTF8.o \
	   Entity.o \
	   HeaderParser.o \
	   HtmlParser.o \
	   HtmlDocument.o \
	   ElementFactory.o \
	   HtmlElement.o \
	   HtmlMetaElement.o \
	   HtmlAnchorElement.o

clean:
	rm -f *.o tester

tester: $(OFILES) $(LINKFILES:%.o=../../arm9/build.sdl/%.o) ../Main.o
	$(CXX) -o $@ $^ -L.. -lcppunit -ldl

run: tester
	@./$<
