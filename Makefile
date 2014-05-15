CFLAGS += -g -O0 -std=gnu99 -Wall -Wextra -Werror -Wno-unused-parameter

all: testbignum

BIGNUM = bignum.o bigmath.o bignum-str.o bignum-add.o bignum-sub.o bignum-mul.o bignum-eq.o sstr.o

testbignum: $(BIGNUM) testbignum.o

clean:
	rm -f *.o testbignum
