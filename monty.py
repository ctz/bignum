import dumbegcd 

R_shift = 32

WORD = 0xffffffff
WORD_MAX = WORD+1

def word(x, i):
    return (x >> (i * 32)) & WORD

def words(x):
    n = x.bit_length()
    return (n + 31) / 32

def dump(s, x):
    print '%s = %s' % (s, dhex(x))

def dhex(x):
    return '0x%x +%dw' % (x, words(x))

def montify(a, m):
    #print 'montify(%s, %s)' % (dhex(a), dhex(m))
    r = (a << R_shift) % m
    return r

def montmul(x, y, m):
    #print 'montmul(%s, %s, %s)' % (dhex(x), dhex(y), dhex(m))
    A = 0
    gcd, m_prime, _ = dumbegcd.egcd(m, WORD_MAX)
    assert gcd == 1
    m_prime = -m_prime
    dump('  x', x)
    dump('  y', y)
    dump('  m', m)

    for i in range(words(m)):
        u_i = ((word(A, 0) + word(x, i) * word(y, 0)) * m_prime) % WORD_MAX
        A = (A + word(x, i) * y + u_i * m) >> R_shift
    if A >= m:
        A -= m
    dump('  result-m', A)
    return A

def modexp(x, e, m):
    x = x % m
    e = e % m
    dump('x', x)
    dump('e', e)
    dump('m', m)
    RR = montify(montify(1, m), m)
    x_p = montmul(x, RR, m)
    dump('x~', x_p)
    A = montify(1, m)
    dump('A', A)

    l = e.bit_length()
    j = l - 1
    while j >= 0:
        A = montmul(A, A, m)
        e_j = (e >> j) & 1
        if e_j:
            A = montmul(A, x_p, m)
        j -= 1
    A = montmul(A, 1, m)
    return A

if __name__ == '__main__':
    r = modexp(0xe2e9, 0x410d, 0xb2f7)
    print 'modexp', hex(r)
    print 'eq ?=', hex(pow(0xe2e9, 0x410d, 0xb2f7))
