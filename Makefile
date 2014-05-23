CFLAGS += -g -O0 -std=gnu99 -Wall -Wextra -Werror -Wno-unused-parameter

all: testbignum

BIGNUM = bignum.o bignum-math.o bignum-str.o \
	 bignum-add.o bignum-sub.o bignum-mul.o \
	 bignum-eq.o bignum-sqr.o bignum-div.o \
	 bignum-shift.o bignum-modmul.o \
	 sstr.o

testbignum: $(BIGNUM) testbignum.o

clean:
	rm -f *.o testbignum
test: testbignum
	./testbignum

soaktest: testbignum gentests.py
	python gentests.py --continuous | ./testbignum --no-exec stdin
