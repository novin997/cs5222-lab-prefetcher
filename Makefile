
all: dpc2sim-stream

run: dpc2sim-stream
	zcat traces/gcc_trace2.dpc.gz | ./dpc2sim-stream

dpc2sim-stream:
	$(CXX) -g -std=c++0x -Wall -o dpc2sim-stream example_prefetchers/ghbac_direct.cc lib/dpc2sim.a

clean:
	rm -rf dpc2sim-stream

.PHONY: all clean
