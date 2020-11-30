APPNAME = fintrack
SOURCES = $(shell find . -type f -name '*.cc'; find . -type f -name '*.c' |xargs)
HEADERS = $(shell find . -type f -name '*.h' |xargs)
CXX = g++ -std=gnu++1y

LIB = -lm -lpthread `pkg-config --libs --cflags sdl`

INCLUDES = -I/usr/include -I.
WARNINGS = -Wall -Wsign-compare -Wpointer-arith -Wcast-qual -Wcast-align

OPT = -O3 -g3 -ffast-math $(INCLUDES) $(WARNINGS)

#OPT = -O0 -g3 $(INCLUDES) $(WARNINGS)

all:	$(APPNAME)
	@wc $(SOURCES) $(HEADERS)
	@echo OK.

$(APPNAME):
	$(CXX) $(OPT) -o $(APPNAME) $(SOURCES) $(LIB)

clean:
	@echo Cleaning up...
	@rm -f nohup.out core gmon.out *.o *~ $(APPNAME)
	@echo Done.

classe:
	@indent -lc0 -sc -ss -ts8 -bl -bls -d0 -lp -i8 -bli0 -npsl -l1000 *.cpp *.h
	@echo Classe.

