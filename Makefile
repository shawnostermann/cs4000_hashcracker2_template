# Generic OpenMPI Makefile
# Ostermann - Mar 15, 2022

# util library needed for openpty
LDLIBS=-lutil
CC=mpicc -Wall -Werror -O2 -Wno-unused-result

HOSTFILE=cslab-hosts

CSOURCES=${wildcard *.c}
TARGET=hash_cracker

all: ${TARGET}	

${TARGET}: ${CSOURCES} 


MPI_ARGS=--hostfile cslab-hosts --display-allocation
test:
	echo "salt	 hash  alphabet  4"  		| mpiexec ${MPI_ARGS}  ./hash_cracker
	
clean:
	rm -f  *.o ${TARGET}
