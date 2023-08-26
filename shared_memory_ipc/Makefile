CC = gcc
CPPC = g++

all: market_data strategy read_latency

market_data: producer.c shm_ipc.h
	$(CC) producer.c -o market_data

strategy: consumer.c shm_ipc.h
	$(CC) consumer.c -o strategy

read_latency: read_latency.cpp shm_ipc.h
	$(CPPC) read_latency.cpp -o read_latency

clean:
	rm -f market_data strategy read_latency
