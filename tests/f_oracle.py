"""Canonicalize small trees and enumerate every legal diameter fold."""
from functools import lru_cache

def canonical(g):
    def rooted(u, p):
        return '(' + ''.join(sorted(rooted(v, u) for v in g[u] if v != p)) + ')'
    degree = [len(x) for x in g]
    leaves = [i for i,d in enumerate(degree) if d <= 1]
    count = len(g)
    while count > 2:
        count -= len(leaves)
        nxt = []
        for u in leaves:
            for v in g[u]:
                degree[v] -= 1
                if degree[v] == 1: nxt.append(v)
        leaves = nxt
    return min(rooted(u,-1) for u in leaves)

def parse(s):
    g=[]; stack=[]
    for c in s:
        if c=='(':
            v=len(g); g.append([])
            if stack:
                g[v].append(stack[-1]);g[stack[-1]].append(v)
            stack.append(v)
        else:stack.pop()
    return g

def paths(g):
    n=len(g); diameter=-1; result=[]
    for u in range(n):
        parent=[-1]*n; parent[u]=u; q=[u]
        for v in q:
            for w in g[v]:
                if parent[w]<0:parent[w]=v;q.append(w)
        for v in range(u+1,n):
            p=[v]
            while p[-1]!=u:p.append(parent[p[-1]])
            if len(p)>diameter:diameter=len(p);result=[p]
            elif len(p)==diameter:result.append(p)
    return result

def fold(g,p):
    mapping=list(range(len(g)))
    for i,v in enumerate(p):mapping[v]=p[min(i,len(p)-1-i)]
    vertices=sorted(set(mapping)); ids={u:i for i,u in enumerate(vertices)}
    h=[set() for _ in vertices]
    for u in range(len(g)):
        for v in g[u]:
            a,b=ids[mapping[u]],ids[mapping[v]]
            if a!=b:h[a].add(b)
    return canonical(h)

@lru_cache(None)
def solve(s):
    if s=='()':return 0,0
    g=parse(s)
    nxt=set(fold(g,p) for p in paths(g))
    values=[solve(x) for x in nxt]
    return 1+max(x[0] for x in values),1+min(x[1] for x in values)
