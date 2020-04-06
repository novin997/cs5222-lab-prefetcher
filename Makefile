
all: dpc2sim-stream

run: dpc2sim-stream
	zcat traces/omnetpp_trace2.dpc.gz | ./dpc2sim-stream

dpc2sim-stream:
	# $(CXX) -g -std=c++0x -Wall -o dpc2sim-stream example_prefetchers/ghbpcdc_direct.cc lib/dpc2sim.a
	# $(CXX) -g -std=c++0x -Wall -o dpc2sim-stream example_prefetchers/skeleton.cc lib/dpc2sim.a
	# $(CXX) -g -std=c++0x -Wall -o dpc2sim-stream example_prefetchers/stream_prefetcher.cc lib/dpc2sim.a
	$(CXX) -g -std=c++0x -Wall -o dpc2sim-stream example_prefetchers/ghbac_direct.cc lib/dpc2sim.a

clean:
	rm -rf dpc2sim-stream

.PHONY: all clean
