APPNAME = fintrack
SOURCES = $(shell find . -type f -name '*.cc'; find . -type f -name '*.cu' |xargs)
HEADERS = $(shell find . -type f -name '*.h' |xargs)
ifeq (, $(shell which nvcc))
CXX = g++ -x c++
OPT = -O3 $(INCLUDES) -Wall -ffast-math -Wsign-compare -Wpointer-arith -Wcast-qual -Wcast-align
else
CXX = nvcc
OPT = -O3 $(INCLUDES) --compiler-options "-Wall -ffast-math -Wsign-compare -Wpointer-arith -Wcast-qual -Wcast-align"
endif

LIB = -lm -lpthread

INCLUDES = -I/usr/include -I.

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
	@indent -lc0 -sc -ss -ts8 -d0 -br -brs -lp -i8 -bli0 -npsl -l1000 *.cc *cu *.h
	@echo Classe.

