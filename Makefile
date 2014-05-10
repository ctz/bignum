CFLAGS += -g -O0 -std=c99 -Wall -Wextra -Werror -Wno-unused-parameter

all: testbignum

testbignum: bignum.o bignum-str.o testbignum.o

clean:
	rm *.o testbignum
