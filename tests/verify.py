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
    assert run('A', '1 4\n4 11 5 14 0\n') == ['0']


def verify_e():
    values = list(range(1, 1501))
    got = []
    for i in range(0, len(values), 1000):
        batch = values[i:i + 1000]
        got += list(map(int, run('E', str(len(batch)) + '\n' + '\n'.join(map(str, batch)))))
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
    assert run('E', '5\n48\n2\n4\n8\n16\n') == ['47', '1', '3', '7', '23']
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


def verify_f():
    from f_oracle import canonical, parse, solve
    trees, cases = {'()'}, []
    for n in range(1, 13):
        cases.extend(sorted(trees))
        if n == 12: break
        following = set()
        for s in trees:
            g = parse(s)
            for u in range(n):
                h = [x[:] for x in g] + [[u]]
                h[u].append(n)
                following.add(canonical(h))
        trees = following
    for _ in range(300):
        n = RNG.randrange(13, 24)
        g = [[] for _ in range(n)]
        for v in range(1, n):
            u = RNG.randrange(v); g[u].append(v); g[v].append(u)
        cases.append(canonical(g))
    data, expected = [], []
    for s in cases:
        g = parse(s)
        data.append(str(len(g)) + '\n' + ''.join(f'{u+1} {v+1}\n' for u in range(len(g)) for v in g[u] if u < v))
        expected.append(solve(s)[0])
    actual = list(map(int, run('F', str(len(data)) + '\n' + ''.join(data))))
    assert actual == expected
    assert run('F', '3\n1\n5\n1 2\n1 3\n1 4\n4 5\n13\n1 2\n2 3\n3 4\n2 5\n5 6\n5 7\n1 8\n8 9\n9 10\n8 11\n11 12\n11 13\n') == ['0', '3', '7']
    print(f'F: {len(cases)} trees checked against all legal folding choices, including all unlabelled trees up to 12 vertices; samples passed')


def verify_c():
    data, expected = [], []
    count = 300
    for trial in range(count):
        n = RNG.randrange(2, 85)
        edges = [(RNG.randrange(v), v) for v in range(1, n)]
        RNG.shuffle(edges)
        weights = [RNG.randrange(1, 101) for _ in range(n)]
        queries = []
        for _ in range(180):
            l = RNG.randrange(n - 1); r = RNG.randrange(l, n - 1); f = RNG.randrange(l, r + 1)
            queries.append((l + 1, r + 1, f + 1))
            p, sums = list(range(n)), weights[:]
            def find(u):
                while p[u] != u: u = p[u]
                return u
            best = max(weights)
            for e in range(l, r + 1):
                if e == f: continue
                a, b = map(find, edges[e])
                p[b] = a; sums[a] += sums[b]; best = max(best, sums[a])
            expected.append(best)
        data.append(str(n) + '\n' + ''.join(f'{u+1} {v+1}\n' for u, v in edges) +
                    ' '.join(map(str, weights)) + '\n' + str(len(queries)) + '\n' +
                    ''.join(f'{l} {r} {f}\n' for l, r, f in queries))
    actual = list(map(int, run('C', str(count) + '\n' + ''.join(data))))
    assert actual == expected, next((i, a, b) for i, (a, b) in enumerate(zip(actual, expected)) if a != b)
    assert run('C', '2\n5\n1 2\n2 3\n3 4\n4 5\n10 20 30 40 50\n3\n1 4 2\n2 4 4\n1 1 1\n2\n1 2\n7 5\n1\n1 1 1\n') == ['120', '90', '50', '7']
    print(f'C: {len(expected)} queries on {count} random trees checked by rebuilding the forest; samples passed')


def convex_hull(points):
    points = sorted(set(points))
    def cross(a, b, c):
        return (b[0]-a[0]) * (c[1]-a[1]) - (b[1]-a[1]) * (c[0]-a[0])
    def half(seq):
        h = []
        for p in seq:
            while len(h) >= 2 and cross(h[-2], h[-1], p) <= 0: h.pop()
            h.append(p)
        return h
    h = half(points)[:-1] + half(points[::-1])[:-1]
    start = h.index((0, 0))
    return h[start+1:] + h[:start+1]


def slow_b(points, check_arcs=False):
    n = len(points)
    d2 = lambda a, b: (a[0]-b[0])**2 + (a[1]-b[1])**2
    radii = [max(d2(a, b) for b in points) for a in points]
    centers, perimeter = [], 0.
    for i in range(n):
        perimeter += math.sqrt(d2(points[i-1], points[i])); centers.append(perimeter)
    circles = [(x + shift * perimeter, r2, i) for shift in [-1, 0, 1]
               for i, (x, r2) in enumerate(zip(centers, radii))]
    split = [0., perimeter]
    for i, (a, ra, _) in enumerate(circles):
        for b, rb, _ in circles[i+1:]:
            x = (a + b) / 2 + (ra - rb) / (2 * (b - a))
            if 0 < x < perimeter: split.append(x)
    split.sort()
    def primitive(x, r2):
        r = math.sqrt(r2); x = min(r, max(-r, x))
        return (x * math.sqrt(max(0., r2-x*x)) + r2 * math.asin(x/r)) / 2
    area = 0.
    for l, r in zip(split, split[1:]):
        middle = (l+r)/2
        c, r2, _ = max(circles, key=lambda c: c[1]-(middle-c[0])**2)
        area += primitive(r-c, r2) - primitive(l-c, r2)
    if check_arcs:
        for j in range(51):
            x = perimeter * (j + .372) / 51
            actual = -1.
            for c, _, i in circles:
                a = points[i]; prev = points[i-1]; nxt = points[(i+1)%n]
                incoming = (a[0]-prev[0], a[1]-prev[1]); len1 = math.hypot(*incoming)
                outgoing = (nxt[0]-a[0], nxt[1]-a[1]); len2 = math.hypot(*outgoing)
                t = x-c
                for v in points:
                    dx, dy = v[0]-a[0], v[1]-a[1]
                    start = (dx*incoming[0]+dy*incoming[1])/len1
                    end = (dx*outgoing[0]+dy*outgoing[1])/len2
                    if start-1e-7 <= t <= end+1e-7:
                        actual = max(actual, dx*dx+dy*dy-t*t)
            full = max(r2-(x-c)**2 for c, r2, _ in circles)
            assert abs(actual-full) <= 1e-7 * max(full, 1), (points, x, actual, full)
    return area / perimeter


def verify_b():
    cases = [[(2, 0), (1, 5), (0, 0)], [(1, 0), (1, 1), (0, 1), (0, 0)]]
    # Regression: unrestricted farthest vertices do not move monotonically.
    cases.append([(4970,0), (9808,2282), (9629,5277), (277,6540),
                  (-3103,5513), (-4619,1984), (-3649,387), (0,0)])
    for _ in range(220):
        w, h = RNG.randrange(1, 10000), RNG.randrange(1, 10000)
        cases.append(convex_hull([(0, 0), (w, 0)] +
                     [(RNG.randrange(-w, 2*w), RNG.randrange(1, h+1)) for _ in range(20)]))
    cases += [[(10**9,0), (10**9,1), (0,1), (0,0)],
              [(1,0), (10**9,10**9), (-10**9,10**9-1), (0,0)]]
    data = str(len(cases)) + '\n' + ''.join(str(len(p))+'\n'+''.join(f'{x} {y}\n' for x,y in p) for p in cases)
    actual = list(map(float, run('B', data)))
    for i, (p, a) in enumerate(zip(cases, actual)):
        expected = slow_b(p, check_arcs=i < 60)
        assert abs(a-expected) <= 2e-10 * max(expected, 1), (p, a, expected)
    assert abs(actual[0]-4.908664519941197) < 1e-12
    assert abs(actual[1]-1.384172075579563) < 1e-12
    print(f'B: {len(cases)} polygons checked with all-pairs radii and all circle intersections; actual rotation arcs, skinny/large coordinates and samples passed')


if __name__ == '__main__':
    import sys
    for name in sys.argv[1:] or ['A', 'E', 'H', 'I', 'F', 'C', 'B']:
        globals()['verify_' + name.lower()]()
