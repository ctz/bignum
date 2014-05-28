import random
import operator
import itertools
import math
import optparse
import sys

TESTS = 2

SIGNS = (1, -1)
SIZES = (16, 32, 64, 128, 192, 512, 1024, 2048, )
SHIFT_SIZES = range(1, 8)
EXP_SIZES = SIZES[:6]

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

        code = random.getrandbits(2)

        if code == 0:
            a |= 0xffffffff << shift
        elif code == 1:
            a ^= ((a >> shift) & 0xffffffff) << shift
        elif code == 2:
            a |= 0xff000000 << shift
        elif code == 3:
            a |= 0xf0000000 << shift
    return a

def gen_tests(f, function, nargs, op, reject = lambda *x: False, sizesa = SIZES, sizesb = SIZES, sizesc = SIZES):
    def gen(sizes, mkcandidate, signs):
        for _ in range(TESTS):
            first = True
            candidates = [1] * nargs
            while first or reject(*candidates):
                for i in range(nargs):
                    candidates[i] = mkcandidate(sizes[i]) * signs[i]
                first = False
            args = ', '.join(str(x) for x in candidates)
            print >>f, 'check("%s(%s) == %d");' % (function, args, op(*candidates))

    sizes = itertools.product(*[sizesa, sizesb, sizesc][:nargs])
    signs = itertools.product(*([SIGNS] * nargs))

    for sg in signs:
        for can in (random.getrandbits, random_carry):
            for sz in sizes:
                gen(sz, can, sg)

def gen_tests_with_file(fout, funcname, *args, **kwargs):
    if fout is not None:
        gen_tests(fout, funcname, *args, **kwargs)
    else:
        filename = 'test-%s.inc' % funcname
        with open(filename, 'w') as f:
            gen_tests(f, funcname, *args, **kwargs)
        print filename, 'written.'

def emit_tests(fout = None):
    gen_tests_with_file(fout, 'mul', 2, operator.mul)
    gen_tests_with_file(fout, 'add', 2, operator.add)
    gen_tests_with_file(fout, 'sub', 2, operator.sub)
    gen_tests_with_file(fout, 'sqr', 1, lambda x: operator.pow(x, 2))
    gen_tests_with_file(fout, 'mod', 2, operator.mod, reject = lambda p, d: d == 0)
    gen_tests_with_file(fout, 'div', 2, operator.div, reject = lambda p, d: d == 0)
    gen_tests_with_file(fout, 'shl', 2, operator.ilshift, sizesb = SHIFT_SIZES)
    gen_tests_with_file(fout, 'shr', 2, operator.irshift, sizesb = SHIFT_SIZES)
    gen_tests_with_file(fout, 'modmul', 3, lambda x, y, p: (x * y) % p)
    gen_tests_with_file(fout, 'modexp', 3, pow, sizesb = EXP_SIZES)

if __name__ == '__main__':
    op = optparse.OptionParser()
    op.add_option('-c', '--continuous', action = 'store_true',
                  default = False,
                  help = 'Continuously generate tests to stdout.')
    opts, _ = op.parse_args()

    if opts.continuous:
        while True:
            emit_tests(sys.stdout)
    else:
        emit_tests()

