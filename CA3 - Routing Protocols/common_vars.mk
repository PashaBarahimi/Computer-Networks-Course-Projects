CXX       = g++
CXXFLAGS += -Wall -pedantic
CXX      += $(CXXFLAGS)
CPPFLAGS += -std=c++17

MKDIR  = mkdir -p
RM     = rm -f
RMDIR  = rm -rf
COPY   = cp
MOVE   = mv -f
RENAME = mv -f
NULL_DEVICE = /dev/null
