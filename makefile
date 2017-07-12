INCLUDES_DIR:=./include

FLIP_SRC_DIR:=./flip
FLIP_SRCS:=$(shell find $(FLIP_SRC_DIR) -name *)

CXX:=g++
CXXFLAGS:= -std=c++11 -I$(INCLUDES_DIR)

all: flip

.PHONY: flip
flip: $(FLIP_SRCS)
	@echo $(FLIP_SRCS)
	$(CXX)  $(CXXFLAGS) flip/delaunay_flip.cpp -o bin/delaunay_flip 

