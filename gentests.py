import random
import operator
import itertools
import math

TESTS = 8

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

def gen_tests(name, nargs, op, reject = lambda *x: False, sizesa = SIZES, sizesb = SIZES):
    def gen2(f, sza, szb, candidate, signa = 1, signb = 1):
        for _ in range(TESTS):
            first = True
            a = b = 1
            while first or reject(a, b):
                a = candidate(sza) * signa
                b = candidate(szb) * signb
                first = False
            
            print >>f, 'check("%s(%d, %d) == %d");' % (name, a, b, op(a, b))
    
    def gen1(f, sza, szb, candidate, signa = 1, signb = 1):
        for _ in range(TESTS):
            first = True
            a = 1
            while first or reject(a):
                a = candidate(sza) * signa
                first = False

            print >>f, 'check("%s(%d) == %d");' % (name, a, op(a))

    gen = gen1 if nargs == 1 else gen2
    sizes = itertools.product(sizesa, (1,)) if nargs == 1 else itertools.product(sizesa, sizesb)

    filename = 'test-%s.inc' % name
    with open(filename, 'w') as f:
        for signa, signb in SIGNS:
            for can in (random.getrandbits, random_carry):
                for sza, szb in sizes:
                    gen(f, sza, szb, can, signa, signb)
    print filename, 'written.'

gen_tests('mul', 2, operator.mul)
gen_tests('add', 2, operator.add)
gen_tests('sub', 2, operator.sub)
gen_tests('sqr', 1, lambda x: operator.pow(x, 2))
gen_tests('mod', 2, operator.mod, reject = lambda p, d: d == 0)
gen_tests('div', 2, operator.div, reject = lambda p, d: d == 0)
gen_tests('shl', 2, operator.ilshift, sizesb = range(1, 8))
