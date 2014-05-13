import random
import operator

TESTS = 16

def gen_random(name, op):
    def gen(f, sz, signa = 1, signb = 1):
        for _ in range(TESTS):
            a = random.getrandbits(sz) * signa
            b = random.getrandbits(sz) * signb
            print >>f, 'check("%s(%d, %d) == %d");' % (name, a, b, op(a, b))

    filename = 'test-%s.inc' % name
    with open(filename, 'w') as f:
        for signa, signb in ((1, 1), (-1, 1), (1, -1), (-1, -1)):
            gen(f, 16, signa, signb)
            gen(f, 32, signa, signb)
            gen(f, 64, signa, signb)
            gen(f, 128, signa, signb)
            gen(f, 1024, signa, signb)
    print filename, 'written.'

gen_random('mul', operator.mul)
gen_random('add', operator.add)

