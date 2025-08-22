EXECUTABLE := tvdemo
LIBSDIR := . 
LIBS := pthread z protobuf
CC=g++

CFLAGS := -std=c++11 -g -O3 $(addprefix -D,$(DEFMACRO)) $(addprefix -I,$(LIBSDIR))  \
	-I/usr/include/mysql -I../include/dalian -fPIC -DNET_BYTE_ORDER
CXXFLAGS := $(CFLAGS)

SOURCE := $(wildcard *.c) $(wildcard *.cc) $(wildcard *.cpp) $(wildcard ./base/*.cpp) $(wildcard ./net/*.cpp) $(wildcard ./profile/*.cpp)
OBJS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
DEPS := $(patsubst %.o,%.d,$(OBJS))
MISSING_DEPS := $(filter-out $(wildcard $(DEPS)),$(DEPS))
MISSING_DEPS_SOURCES := $(wildcard $(patsubst %.d,%.c,$(MISSING_DEPS)) \
$(patsubst %.d,%.cpp,$(MISSING_DEPS)))
CPPFLAGS += -MD

everything : $(EXECUTABLE)

deps : $(DEPS)

objs : $(OBJS)

clean :
	rm -f *.o
	rm -f base/*.o
	rm -f base/*.d
	rm -f net/*.o
	rm -f net/*.d
	rm -f *.d

veryclean: clean
	rm -f $(EXECUTABLE)
	
	
rebuild: veryclean everything

ifneq ($(MISSING_DEPS),)
$(MISSING_DEPS) :
	@$(RM-F) $(patsubst %.d,%.o,$@)
endif

#-include $(DEPS)

$(EXECUTABLE) : $(OBJS)
	g++ -std=c++11 -g -o $(EXECUTABLE) $(OBJS) $(addprefix -l,$(LIBS)) -L../release -L/usr/lib64/mysql/ -Wl,-rpath=./ -lrt -ldl 

	
