import math

def even(v):
    return v & 1 == 0

def egcd(x, y):
    assert x > 0 and y > 0
    g = 1
    while even(x) and even(y):
        x = x / 2
        y = y / 2
        g = g * 2
    u, v = x, y
    A, B, C, D = 1, 0, 0, 1

    while u != 0:
        while even(u):
            u = u / 2
            if even(A) and even(B):
                A = A / 2
                B = B / 2
            else:
                A = (A + y) / 2
                B = (B - x) / 2

        while even(v):
            v = v / 2
            if even(C) and even(D):
                C = C / 2
                D = D / 2
            else:
                C = (C + y) / 2
                D = (D - x) / 2

        if u >= v:
            u = u - v
            A = A - C
            B = B - D
        else:
            v = v - u
            C = C - A
            D = D - B

    return g * v, C, D

if __name__ == '__main__':
    x, y = 47581, 63516
    gcd, a, b = egcd(x, y)
    assert gcd == a * x + b * y

    x, y = 1591, 42095
    gcd, a, b = egcd(x, y)
    assert gcd == a * x + b * y
