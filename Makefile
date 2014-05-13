CFLAGS += -g -O0 -std=gnu99 -Wall -Wextra -Werror -Wno-unused-parameter

all: testbignum

testbignum: sstr.o bignum.o bignum-str.o testbignum.o

clean:
	rm -f *.o testbignum
