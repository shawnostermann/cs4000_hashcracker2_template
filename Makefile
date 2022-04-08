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

commit:
	git add Makefile ${CSOURCES} ${HOSTFILE}
	git commit -m "latest commit" Makefile ${CSOURCES} ${HOSTFILE}
	git push

test: ${TARGET}	
	echo "foobar 	 9pHdGraWcEy3y.NvdzCOSfu0XalZhBWUgJ/iKxpdipC  01  8"  	 	| mpiexec -n 4 ./hash_cracker
	@echo "Correct answer: 204: '11001100'"
	echo "notfound 	 9pHdGraWcEy3y.NvdzCOSfu0XalZhBWUgJ/iKxpdipC  23  8"  	 	| mpiexec -n 4 ./hash_cracker
	@echo "Correct answer: not found"
	echo "foobar 	 P0jyUFlvXHFKF8.IDZKc.9dXfKDvfmFv7sD6VFxFnE1  01 10"  		| mpiexec -n 4 ./hash_cracker
	@echo "Correct answer: 819: '1100110011'"
	echo "vowelsRfun lFtqQCPzGxs/jOa1WSAVq1A40onc4iV1WcBdcVgxKV0  aeiou  5"		| mpiexec -n 4 ./hash_cracker
	@echo "Correct answer: 1730: 'iouea'"
	echo "hexydigits BKD9vk1zOVvrNd8xRZR7B5cEsqrd61b9IUmPGSu9.c2  0123456789ABCDEF  3"		| mpiexec -n 4 ./hash_cracker
	@echo "Correct answer: 3150: 'C4E'"
	echo "andpepper	 Xl9dddHVkCrdG3vTJkuEBK9ecqr/aZT5wJFKqz/PRJC  clangCLANK  4"  		| mpiexec -n 4 ./hash_cracker
	@echo "Correct answer: 8251: 'NaCl'"
	echo "rnderr     eNQG/ksOHKtFACs9sjiPMHvwiaSVFMiKOgtGUNVX9r3  01  8"        	| mpiexec -n 3 ./hash_cracker
	@echo "Correct answer: 255: '11111111'"

MPI_ARGS=--hostfile cslab-hosts --display-allocation
labtest:
	echo "andpepper	 Xl9dddHVkCrdG3vTJkuEBK9ecqr/aZT5wJFKqz/PRJC  clangCLANK  4"  		| mpiexec ${MPI_ARGS}  ./hash_cracker
	echo "foobar 	 9pHdGraWcEy3y.NvdzCOSfu0XalZhBWUgJ/iKxpdipC  01234  8"  	 	| mpiexec ${MPI_ARGS} ./hash_cracker
	echo "andpepper	 Xl9dddHVkCrdG3vTJkuEBK9ecqr/aZT5wJFKqz/PRJC  abcdefghijklmnopqrstuvwxyz  4"  		| mpiexec ${MPI_ARGS} ./hash_cracker
	echo "digits	 /qO6EaGmCN0xb.A7TaQqaiqe2abxKRT6lWGXQ5clK50  0123456789  6"  		| mpiexec ${MPI_ARGS} ./hash_cracker
	echo "digits	 V3yp06hCkJ1duTQBwYIx1jkWFpgne7blfDWtWopHTG/  0123456789  7"  		| mpiexec ${MPI_ARGS} ./hash_cracker
	echo "digits	 ZpcAGpt1bosB01d/m/YDcyQ4paLq5Bh5kgPMxgEwp59  0123456789  8"  		| mpiexec ${MPI_ARGS} ./hash_cracker
	echo "digits	 Na4xVSbM4JlVhwZTxzT3.bas3bwHBAFCZD8VAUZaYmB  0123456789  9"  		| mpiexec ${MPI_ARGS} ./hash_cracker
	echo "digits	 cPZlVRbNBK2GiIHLmJQfqLwaj5KWDvENPvr7ay/4ZW.  0123456789  10"  		| mpiexec ${MPI_ARGS} ./hash_cracker

clean:
	rm -f  *.o ${TARGET}
