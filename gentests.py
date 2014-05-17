import random
import operator
import math

TESTS = 32

SIGNS = ((1, 1), (-1, 1), (1, -1), (-1, -1))
SIZES = (16, 32, 64, 128, 192, 512, 1024, 2048, )

def wordsz(n):
    if n == 0:
        return 1
    return math.ceil(math.log(n, 2) / 32)

def random_carry(sz):
    """
    Make a number which is sz bits long, and has some
    32-bit words set or cleared to execise carry/borrow
    code paths.
    """

    a = random.getrandbits(sz)
    for _ in range(random.getrandbits(3)):
        word = random.randrange(0, wordsz(a))
        shift = word * 32

        if random.getrandbits(1):
            a |= 0xffffffff << shift
        else:
            a ^= ((a >> shift) & 0xffffffff) << shift
    return a

def gen_tests(name, op):
    def gen(f, sz, candidate, signa = 1, signb = 1):
        for _ in range(TESTS):
            a = candidate(sz) * signa
            b = candidate(sz) * signb
            print >>f, 'check("%s(%d, %d) == %d");' % (name, a, b, op(a, b))

    filename = 'test-%s.inc' % name
    with open(filename, 'w') as f:
        for signa, signb in SIGNS:
            for can in (random.getrandbits, random_carry):
                for sz in SIZES:
                    gen(f, sz, can, signa, signb)
    print filename, 'written.'

gen_tests('mul', operator.mul)
gen_tests('add', operator.add)
gen_tests('sub', operator.sub)

