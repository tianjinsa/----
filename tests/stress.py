"""Maximum-size performance and structural checks; outputs stay under build/."""
import math
import random
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RNG = random.Random(106701)


def timed(name, data, expected=None):
    source = ROOT / 'build' / f'stress-{name}.in'
    source.write_text(data, encoding='ascii')
    start = time.perf_counter()
    with source.open('r', encoding='ascii') as f:
        result = subprocess.run([str(ROOT / 'build' / (name+'.exe'))], stdin=f,
                                text=True, capture_output=True, check=True, timeout=180)
    elapsed = time.perf_counter()-start
    if expected is not None:
        assert list(map(int, result.stdout.split())) == expected
    print(f'{name}: {elapsed:.3f} s; {len(result.stdout.split())} answers; '
          f'first answer {result.stdout.split()[0]}', flush=True)


def stress_c():
    n = q = 200000
    queries = []
    for _ in range(q):
        l = RNG.randrange(1, n); r = RNG.randrange(l, n); f = RNG.randrange(l, r+1)
        queries.append((l,r,f))
    for shape in ['path', 'star', 'random']:
        if shape == 'path': edges = [(v-1,v) for v in range(2,n+1)]
        elif shape == 'star': edges = [(1,v) for v in range(2,n+1)]
        else:
            edges = [(RNG.randrange(1,v),v) for v in range(2,n+1)];RNG.shuffle(edges)
        data = f'1\n{n}\n' + ''.join(f'{u} {v}\n' for u,v in edges) + '100 '*n + f'\n{q}\n' + ''.join(f'{l} {r} {f}\n' for l,r,f in queries)
        expected = None
        if shape == 'path': expected = [100*max(f-l+1,r-f+1,1) for l,r,f in queries]
        if shape == 'star': expected = [100*(r-l+1) for l,r,f in queries]
        print(f'C {shape}:', flush=True); timed('C',data,expected)


def stress_f():
    n = 50000
    for shape in ['path','star','random']:
        if shape=='path': edges=[(v-1,v) for v in range(2,n+1)]; expected=[(n-1).bit_length()]
        elif shape=='star': edges=[(1,v) for v in range(2,n+1)]; expected=[n-1]
        else: edges=[(RNG.randrange(1,v),v) for v in range(2,n+1)]; expected=None
        print(f'F {shape}:',flush=True)
        timed('F', f'1\n{n}\n'+''.join(f'{u} {v}\n' for u,v in edges),expected)


def stress_a():
    # Nested subspaces stay active; unlike dense random sets they do not
    # immediately force the full universe.
    n=200000
    rows=['4 0 1 2 3\n']*(n-1)+['3 0 1 4\n']
    timed('A',f'{n} 20\n'+''.join(rows))
    values=RNG.sample(range(1<<20),1000000)
    timed('A','1 20\n1000000 '+ ' '.join(map(str,values))+'\n')


def stress_i():
    n=100000
    data=f'1\n{n}\n'+str(n-1)+' '+' '.join(map(str,range(2,n+1)))+'\n'+'0\n'*(n-1)
    timed('I',data)
    data=f'1\n{n}\n2 2 3\n0\n'+''.join(f'1 {i+1}\n' for i in range(3,n))+'0\n'
    timed('I',data)


def stress_b():
    from verify import convex_hull
    n=100000; radius=500000000
    points=[(round(radius*math.cos(-math.pi/2+math.pi/n+2*math.pi*i/n)),
             round(radius*math.sin(-math.pi/2+math.pi/n+2*math.pi*i/n))) for i in range(n)]
    bottom=min(points,key=lambda p:(p[1],p[0]))
    points=[(x-bottom[0],y-bottom[1]) for x,y in points]
    points=convex_hull(points)
    print(f'B polygon vertices: {len(points)}',flush=True)
    timed('B','1\n'+str(len(points))+'\n'+''.join(f'{x} {y}\n' for x,y in points))


if __name__=='__main__':
    for name in sys.argv[1:] or ['A','I','F','C','B']:
        globals()['stress_'+name.lower()]()
