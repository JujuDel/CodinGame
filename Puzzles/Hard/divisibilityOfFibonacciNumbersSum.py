import sys
import math

def fibo(n, mod):
    if n <= 1:
        return abs(n - 1), n

    # Fibo(n//2-1) and Fibo(n//2)
    f_half_1, f_half = fibo(n // 2, mod)

    # https://en.wikipedia.org/wiki/Fibonacci_number#Matrix_form
    if n % 2 == 0:
        # F_2n-1 = F_n^2 + F_n-1^2
        f_2n_1 = f_half**2 % mod + f_half_1**2 % mod
        # F_2n = (2 * F_n-1 + F_n) * F_n
        f_2n = ((2 * f_half_1 % mod + f_half) % mod) * f_half
    else:
        # F_2n-1 = (2 * F_n-1 + F_n) * F_n
        f_2n_1 = ((2 * f_half_1 % mod + f_half) % mod) * f_half
        # F_2n = F_n+1^2 + F_n^2 = (F_n + F_n-1)^2 + F_n^2
        f_2n = (((f_half + f_half_1) % mod)**2 % mod) + (f_half**2 % mod)

    return f_2n_1 % mod, f_2n % mod

nb = int(input())

for i in range(nb):
    a, b, d = [int(j) for j in input().split()]

    # Fibo(a) and Fibo(a-1)
    f_a_1, f_a = fibo(a, d)

    # Continue Fibo until b and sum meanwhile
    S = f_a
    for _ in range(a + 1, b + 1):
        f_a_1, f_a = f_a, (f_a + f_a_1) % d
        S = (S + f_a) % d

    print(f'F_{a} + ... + F_{b} is ' + 'NOT ' * (S>0) + f'divisible by {d}')
