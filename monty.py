import dumbegcd 

def dumb_montmul_bitwise(a, b, r):
    n = a.bit_length()
    R = 2 ** n

    dest = 0
    for i in range(n):
        if (a >> i) & 1:
            dest += b
        if dest & 1:
            dest += r
        dest >>= 1
    return (dest * R) % r

def dumb_montmul(a, b, r):
    a = montify(a, r)
    return demontify(a * b, r)

def montmul2(a, b, n):
    nwords = words(n)
    R = 2 ** (nwords * 32)
    gcd, np, _ = dumbegcd.egcd(n, R)
    assert gcd == 1
    np = -np

    T = a * b
    M = (T * np) % R
    T = T + M * n
    rc = (T * R) % n
    return demontify(rc, n)

def montify(a, N):
    assert a < N
    bits = words(N) * 32
    rc = (a * 2 ** bits) % N
    return rc

def words(x):
    n = x.bit_length()
    return (n + 31) / 32

def demontify(C, N):
    beta = 2 ** 32
    nprime = words(N)
    gcd, c, _ = dumbegcd.egcd(N, beta)
    assert gcd == 1
    c = -c
    R = C
    for i in range(nprime):
        betai = beta ** i
        q = ((R / betai) * c) % beta
        R = R + N * q * betai
    R = R / (beta ** nprime)
    if R >= N:
        R -= N
    return R

def word(x, i):
    return (x >> (i * 32)) & 0xffffffff

def mask(i):
    return (1 << i) - 1

def modexp(m, d, N):
    l = d.bit_length()
    k = 2
    tab = [ montify(1, N) ]
    for j in range(1, 2 ** k):
        tab.append((tab[j - 1] * m) % N)
    print ', '.join(hex(x) for x in tab)

    r = 1
    j = l - 1

    print bin(d)

    while j >= 0:
        i = max(j - k + 1, 0)
        b = (d >> i) & mask(j - i + 1)

        for s in range(j - i + 1):
            print 'sqr'
            r = pow(r, 2, N)
        print 'after-sqr', hex(r)
        print 'mul'
        r = demontify(r * tab[b], N)
        print 'int', hex(r)
        j = i - 1
    return r

if __name__ == '__main__':
    """
    a, b = 123124461412312312312315192031023812038120312, 1231283712938127
    r = 161521746670640296426473658228859984306663144318152681524054709078245736590366297248377298082656939330673286493230336261991466938596691073112968626710792148904239628873374506302653492009810626437582587089465395941375496004739918498276676334238241465498030036586063929902368192004233172032080188726965600617167L

    print 'modmul', hex((a * b) % r)
    
    print 'dumb_montmul', hex(dumb_montmul(a, b, r))
    print 'dumb_montmul_bitwise', hex(dumb_montmul_bitwise(a, b, r))
    print 'montmul2', hex(montmul2(a, b, r))

    assert a == demontify(montify(a, r), r)

    print 'modexp', hex(pow(a, b, r))

    print 'mont_modexp', hex(modexp(a, b, r))
    """

    print 'modexp', hex(modexp(0xe2e9, 0x410d, 0xb2f7))
    print 'eq ?=', hex(pow(0xe2e9, 0x410d, 0xb2f7))
