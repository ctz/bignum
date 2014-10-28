FEATURES =
# -DWITH_MONTY
CFLAGS += -g -O0 -std=gnu99 -Wall -Wextra -Werror -Wno-unused-parameter $(FEATURES)

all: out testbignum teststr

BIGNUM = bignum.o bignum-math.o bignum-str.o \
	 bignum-add.o bignum-sub.o bignum-mul.o \
	 bignum-eq.o bignum-sqr.o bignum-div.o \
	 bignum-shift.o bignum-modmul.o bignum-modexp.o \
	 bignum-gcd.o bignum-modinv.o bignum-monty.o \
	 bignum-dbg.o \
	 sstr.o

testbignum: $(BIGNUM) testbignum.o

libbignum.a: $(BIGNUM) dstr.o
	ar rcD $@ $^

teststr: sstr.o dstr.o teststr.o

clean:
	rm -f *.o *.pyc testbignum teststr

test: testbignum
	./teststr
	./testbignum

gentests:
	python gentests.py

soaktest: testbignum gentests.py
	python gentests.py --continuous | ./testbignum --no-exec stdin

.PHONY: out
out: libbignum.a bignum.h bignum-str.h sstr.h dstr.h handy.h ext/cutest.h
	mkdir -p $@
	cp -v $^ $@
