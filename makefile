INCLUDES_DIR:=./include

FLIP_SRC_DIR:=./flip
FLIP_SRCS:=$(shell find $(FLIP_SRC_DIR) -name '*.*')

CXX:=g++
CXXFLAGS:= -std=c++11 -I$(INCLUDES_DIR)

all: flip bowyerwatson

.PHONY: flip
flip: $(FLIP_SRCS)
	$(CXX)  $(CXXFLAGS) flip/delaunay_flip.cpp -o bin/delaunay_flip 

.PHONY: bowyerwatson
bowyerwatson: $(FLIP_SRCS)
	$(CXX)  $(CXXFLAGS) bowyer-watson/delaunay_bowyerwatson.cpp -o bin/delaunay_bowyerwatson

.PHONY: runflip
runflip:
	time ./bin/delaunay_flip

.PHONY: runbowyerwatson
runbowyerwatson:
	time ./bin/delaunay_bowyerwatson

.PHONY: runonline
runonline:
	time ./bin/delaunay_online

.PHONY: runall
runall: runflip runbowyerwatson runonline
	python delaunayplot.py

.PHONY: genandrun
genandrun:
	python generate_test.py
	time ./bin/delaunay_flip
	time ./bin/delaunay_bowyerwatson
	time ./bin/delaunay_online
	python delaunayplot.py

.PHONY: benchmark
benchmark:
	python generate_test.py
	time ./bin/delaunay_flip
	time ./bin/delaunay_bowyerwatson 2> /dev/null
	time ./bin/delaunay_online

