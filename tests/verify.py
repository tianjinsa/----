"""Small exhaustive/random oracles. Run with: python tests/verify.py"""
from pathlib import Path
import itertools
import math
import random
import subprocess

ROOT = Path(__file__).resolve().parents[1]
RNG = random.Random(20261001)
MOD = 998244353


def run(name, data):
    return subprocess.check_output([str(ROOT / 'build' / (name + '.exe'))],
                                   input=data, text=True, timeout=20).split()


def verify_a():
    cases = [([{1}, {0, 1, 7}, {1, 2, 3, 4, 5, 7}], 3)]
    for _ in range(160):
        m = RNG.randrange(1, 4)
        sets = [set(RNG.sample(range(1 << m), RNG.randrange(1, (1 << m) + 1)))
                for _ in range(RNG.randrange(1, 7))]
        cases.append((sets, m))
    for sets, m in cases:
        data = f'{len(sets)} {m}\n' + ''.join(
            f'{len(s)} ' + ' '.join(map(str, s)) + '\n' for s in sets)
        output = list(map(int, run('A', data)))
        answer = set(output[1:])
        assert len(answer) == output[0]
        def valid(extra):
            return all(all((x ^ y) in (s | extra) for x in (s | extra)
                           for y in (s | extra)) for s in sets)
        assert valid(answer), (sets, answer)
        optimum = min(mask.bit_count() for mask in range(1 << (1 << m))
                      if valid({i for i in range(1 << m) if mask >> i & 1}))
        assert len(answer) == optimum, (sets, answer, optimum)
    print(f'A: {len(cases)} cases checked against exhaustive search')


def verify_e():
    values = list(range(1, 1501))
    got = list(map(int, run('E', str(len(values)) + '\n' + '\n'.join(map(str, values)))))
    for n, actual in zip(values, got):
        expected = 0
        for a in range(1, n + 1):
            if math.gcd(a, n) != 1:
                continue
            x, order = a % n, 1
            while x != 1 % n:
                x = x * a % n
                order += 1
            expected += order
        assert actual == expected, (n, actual, expected)
    assert run('E', '5\n1\n5\n8\n54\n9653618\n') == ['1', '11', '7', '183', '9856882167209']
    print(f'E: {len(values)} values checked by direct multiplicative orders; sample passed')


def verify_h():
    permutations = []
    for n in range(1, 7):
        permutations.extend(p for p in itertools.permutations(range(n)) if n == 1 or p[0] < p[-1])
    for n in [7, 8, 9, 31, 32, 33, 999, 1000]:
        p = list(range(n)); RNG.shuffle(p)
        if p[0] > p[-1]: p.reverse()
        permutations.append(p)
    proc = subprocess.Popen([str(ROOT / 'build' / 'H.exe')], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, text=True, bufsize=1)
    def send(s):
        proc.stdin.write(str(s) + '\n'); proc.stdin.flush()
    send(len(permutations))
    for p in permutations:
        n, queries = len(p), 0
        send(n)
        while True:
            line = proc.stdout.readline().split()
            assert line, 'H exited before answering'
            if line[0] == '!':
                assert list(map(int, line[1:])) == list(p), (p, line)
                assert queries <= n * (n - 1).bit_length()
                break
            assert line[0] == '?' and len(line) == 3
            m, v = map(int, line[1:])
            assert 0 <= m < 1 << (n - 1).bit_length() and -1 <= v < n
            queries += 1
            inside = [((x & m).bit_count() % 2) ^ (x == v) for x in range(n)]
            send(sum(inside[a] != inside[b] for a, b in zip(p, p[1:])) % 3)
    proc.stdin.close()
    assert proc.wait(timeout=10) == 0
    # The interactor's -1 must terminate the program immediately.
    assert run('H', '1\n2\n-1\n') == ['?', '1', '-1']
    print(f'H: {len(permutations)} interactive cases; query budget and -1 handling passed')


def verify_i():
    cases, expected = [], []
    for _ in range(220):
        n = RNG.randrange(3, 10)
        children = [[] for _ in range(n)]
        children[0] = [1, 2]
        for v in range(3, n): children[RNG.randrange(v)].append(v)
        for ch in children: RNG.shuffle(ch)
        edges = [(u, v) for u in range(n) for v in children[u]]
        leaves = []
        def dfs(u):
            if not children[u]: leaves.append(u)
            for v in children[u]: dfs(v)
        dfs(0)
        edges += list(zip(leaves, leaves[1:]))
        total = 0
        for mask in range(1 << len(edges)):
            parent = list(range(n))
            def find(u):
                while parent[u] != u: u = parent[u]
                return u
            components = n
            for i, (u, v) in enumerate(edges):
                if mask >> i & 1:
                    a, b = find(u), find(v)
                    if a != b: parent[a] = b; components -= 1
            total += components
        expected.append(total * pow(1 << len(edges), -1, MOD) % MOD)
        cases.append(str(n) + '\n' + ''.join(str(len(ch)) + ' ' +
                     ' '.join(str(v + 1) for v in ch) + '\n' for ch in children))
    got = list(map(int, run('I', str(len(cases)) + '\n' + ''.join(cases))))
    assert got == expected, next((i, a, b) for i, (a, b) in enumerate(zip(got, expected)) if a != b)
    assert run('I', '2\n3\n2 2 3\n0\n0\n6\n3 6 2 3\n2 4 5\n0\n0\n0\n0\n') == ['374341634', '694091779']
    print(f'I: {len(cases)} trees checked by enumerating every retained-edge subset; samples passed')


if __name__ == '__main__':
    import sys
    for name in sys.argv[1:] or ['A', 'E', 'H', 'I']:
        globals()['verify_' + name.lower()]()
