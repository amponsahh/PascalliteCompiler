CLASS = 4301
BASE_DIR = /usr/local/$(CLASS)
LINK_DIRS = -L$(BASE_DIR)/lib
INCLUDE_DIRS = -I$(BASE_DIR)/include -I.
LFLAGS = -lm
CFLAGS = -g -Wall -std=c++11

.SECONDEXPANSION:
CC = g++
.SUFFIXES:.o .C .CPP .cpp .c++ .cc .cxx .cp

.C.o:
	$(CC) $(CFLAGS) -c $< $(INCLUDE_DIRS)

.CPP.o:
	$(CC) $(CFLAGS) -c $< $(INCLUDE_DIRS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< $(INCLUDE_DIRS)

.c++.o:
	$(CC) $(CFLAGS) -c $< $(INCLUDE_DIRS)

.cc.o:
	$(CC) $(CFLAGS) -c $< $(INCLUDE_DIRS)

.cxx.o:
	$(CC) $(CFLAGS) -c $< $(INCLUDE_DIRS)

.cp.o:
	$(CC) $(CFLAGS) -c $< $(INCLUDE_DIRS)

targets1srcfile = stage2

$(targets1srcfile): $$@.o
	$(CC) -o $@ $@.o $(LINK_DIRS) $(LFLAGS)

targets2srcfiles = 

$(targets2srcfiles): $$@main.o $$@.o
	$(CC) -o $@ $@main.o $@.o $(LINK_DIRS) $(LFLAGS)

targets2srcfileswithlibrary = 

$(targets2srcfileswithlibrary): $$@main.o $$@.o
	$(CC) -o $@ $@main.o $@.o $(LINK_DIRS) \
	-Wl,-whole-archive -l$@ -Wl,-no-whole-archive $(LFLAGS)

clean:
	rm -f *.o core *~ $(targets1srcfile) $(targets2srcfiles)
	rm -f $(targets2srcfileswithlibrary)
