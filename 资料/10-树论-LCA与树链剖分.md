# 10 树论：LCA / 树链剖分 / 虚树

> 树的题占了 ICPC 的一半。这里给全：DFS 序、子树操作、LCA 四种求法、树链剖分（点权/边权）、重链剖分配合线段树、树上差分、虚树、树的重心/直径、换根 DP 概念。

---

## 1. DFS 序与子树区间

```cpp
#include <bits/stdc++.h>
using namespace std;

// 一棵树，dfn 是 DFS 入序，sz 是子树大小，dfn[u]..dfn[u]+sz[u]-1 是 u 的子树
struct DfsOrder {
    int n, timer;
    vector<int> dfn, sz, dep, fa, rev;
    vector<vector<int>> g;

    explicit DfsOrder(int n_) : n(n_), timer(0), dfn(n_ + 1), sz(n_ + 1),
        dep(n_ + 1), fa(n_ + 1), rev(n_ + 1), g(n_ + 1) {}

    void addEdge(int u, int v) { g[u].push_back(v); g[v].push_back(u); }

    void dfs(int u, int p) {
        fa[u] = p;
        dep[u] = dep[p] + 1;
        dfn[u] = ++timer;
        rev[timer] = u;
        sz[u] = 1;
        for (int v : g[u]) if (v != p) {
            dfs(v, u);
            sz[u] += sz[v];
        }
    }
    void build(int root = 1) { dfs(root, 0); }

    // 把点权 a[u] 转成 dfn 序列上的值：sub[a[u]] = w
    vector<long long> to_array(const vector<long long> &w) const {   // w 下标 1..n
        vector<long long> a(n + 1);
        for (int u = 1; u <= n; ++u) a[dfn[u]] = w[u];
        return a;
    }
};
```

**用法**：`子树加 x、子树求和` → 把 `dfn` 序列交给线段树/树状数组，区间是 `[dfn[u], dfn[u]+sz[u]-1]`。

---

## 2. LCA 四种求法

### 2.1 倍增（最常用）

见 [08-ST表与倍增](08-ST表与倍增.md) 第 4 节。

### 2.2 树链剖分求 LCA（常数小）

```cpp
#include <bits/stdc++.h>
using namespace std;

struct HLD {
    int n;
    vector<int> fa, dep, sz, son, top, dfn, rnk;
    vector<vector<int>> g;
    int timer = 0;

    explicit HLD(int n_) : n(n_), fa(n_ + 1), dep(n_ + 1), sz(n_ + 1),
        son(n_ + 1, 0), top(n_ + 1), dfn(n_ + 1), rnk(n_ + 1), g(n_ + 1) {}

    void addEdge(int u, int v) { g[u].push_back(v); g[v].push_back(u); }

    void dfs1(int u, int p) {          // 求 fa/dep/sz/son
        fa[u] = p; dep[u] = dep[p] + 1; sz[u] = 1;
        int mx = 0;
        for (int v : g[u]) if (v != p) {
            dfs1(v, u);
            sz[u] += sz[v];
            if (sz[v] > mx) { mx = sz[v]; son[u] = v; }
        }
    }
    void dfs2(int u, int tp) {         // 求 top/dfn
        top[u] = tp; dfn[u] = ++timer; rnk[timer] = u;
        if (son[u]) dfs2(son[u], tp);
        for (int v : g[u]) if (v != fa[u] && v != son[u]) dfs2(v, v);
    }
    void build(int root = 1) {
        dep[root] = 0; fa[root] = 0;
        dfs1(root, 0); dfs2(root, root);
    }

    int lca(int u, int v) const {
        while (top[u] != top[v]) {
            if (dep[top[u]] < dep[top[v]]) swap(u, v);
            u = fa[top[u]];
        }
        return dep[u] < dep[v] ? u : v;
    }
    // 路径 u->v 拆成若干重链区间，交给线段树处理
    template <class F>
    void path_query(int u, int v, F f) {     // f(l, r) 处理 dfn 区间
        while (top[u] != top[v]) {
            if (dep[top[u]] < dep[top[v]]) swap(u, v);
            f(dfn[top[u]], dfn[u]);
            u = fa[top[u]];
        }
        if (dep[u] > dep[v]) swap(u, v);
        f(dfn[u], dfn[v]);
    }
};
```

> **注意**：`fa[root] = 0`，`dep[0] = 0`；`path_query` 里最后一段包含 LCA 本身（点权版本）；**边权版本**要 `f(dfn[u]+1, dfn[v])`。

### 2.3 欧拉序 + ST 表（O(1) 查询）

```cpp
#include <bits/stdc++.h>
using namespace std;

// 欧拉序：每个点进入时记录一次，回溯时再记录一次，长度 2n-1
// LCA(u,v) = 欧拉序中 [first[u], first[v]] 里深度最小的点
struct EulerLCA {
    int n, LOG;
    vector<vector<int>> g;
    vector<int> first, euler, dep;
    vector<vector<int>> st;
    vector<int> lg;

    explicit EulerLCA(int n_) : n(n_), g(n_ + 1), first(n_ + 1), dep(n_ + 1) {}

    void addEdge(int u, int v) { g[u].push_back(v); g[v].push_back(u); }

    void dfs(int u, int p) {
        first[u] = (int)euler.size();
        euler.push_back(u);
        dep[u] = dep[p] + 1;
        for (int v : g[u]) if (v != p) { dfs(v, u); euler.push_back(u); }
    }
    void build(int root = 1) {
        dep[0] = -1; dfs(root, 0);
        int m = (int)euler.size();
        LOG = std::__lg(m) + 1;
        st.assign(LOG, vector<int>(m));
        lg.assign(m + 1, 0);
        for (int i = 2; i <= m; ++i) lg[i] = lg[i >> 1] + 1;
        st[0] = euler;
        for (int k = 1; k < LOG; ++k)
            for (int i = 0; i + (1 << k) <= m; ++i) {
                int a = st[k - 1][i], b = st[k - 1][i + (1 << (k - 1))];
                st[k][i] = dep[a] < dep[b] ? a : b;
            }
    }
    int lca(int u, int v) const {
        int l = first[u], r = first[v];
        if (l > r) swap(l, r);
        int k = lg[r - l + 1];
        int a = st[k][l], b = st[k][r - (1 << k) + 1];
        return dep[a] < dep[b] ? a : b;
    }
};
```

### 2.4 Tarjan 离线 LCA（并查集）

```cpp
#include <bits/stdc++.h>
using namespace std;

// 离线：把询问挂到点上，DFS 回溯时合并
struct TarjanLCA {
    int n;
    vector<vector<int>> g;
    vector<vector<pair<int,int>>> qs;     // qs[u] = {(v, queryId)}
    vector<int> fa, ans;
    vector<bool> vis;

    explicit TarjanLCA(int n_) : n(n_), g(n_ + 1), qs(n_ + 1), fa(n_ + 1),
        ans(0), vis(n_ + 1, false) { iota(fa.begin(), fa.end(), 0); }

    void addEdge(int u, int v) { g[u].push_back(v); g[v].push_back(u); }
    void addQuery(int u, int v, int id) { qs[u].push_back({v, id}); qs[v].push_back({u, id}); }

    int find(int x) { return fa[x] == x ? x : fa[x] = find(fa[x]); }

    void dfs(int u) {
        vis[u] = true;
        for (int v : g[u]) if (!vis[v]) {
            dfs(v);
            fa[v] = u;                    // 回溯时把儿子并到父亲
        }
        for (auto [v, id] : qs[u])
            if (vis[v]) ans[id] = find(v);
    }
    vector<int> solve(int root = 1) {
        dfs(root);
        return ans;
    }
};
```

---

## 3. 树链剖分 + 线段树（路径/子树操作）★

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;

// 点权版本：路径加、路径和、子树加、子树和
struct HLDTree {
    int n;
    vector<vector<int>> g;
    vector<int> fa, dep, sz, son, top, dfn, rnk;
    vector<ll> val;                       // val[u] 点权
    int timer = 0;

    // 线段树（区间加 + 区间和）
    vector<ll> sum, lazy;

    explicit HLDTree(int n_) : n(n_), g(n_ + 1), fa(n_ + 1), dep(n_ + 1), sz(n_ + 1),
        son(n_ + 1, 0), top(n_ + 1), dfn(n_ + 1), rnk(n_ + 1), val(n_ + 1),
        sum(4 * n_ + 4, 0), lazy(4 * n_ + 4, 0) {}

    void addEdge(int u, int v) { g[u].push_back(v); g[v].push_back(u); }

    void dfs1(int u, int p) {
        fa[u] = p; dep[u] = dep[p] + 1; sz[u] = 1;
        int mx = 0;
        for (int v : g[u]) if (v != p) {
            dfs1(v, u); sz[u] += sz[v];
            if (sz[v] > mx) { mx = sz[v]; son[u] = v; }
        }
    }
    void dfs2(int u, int tp) {
        top[u] = tp; dfn[u] = ++timer; rnk[timer] = u;
        if (son[u]) dfs2(son[u], tp);
        for (int v : g[u]) if (v != fa[u] && v != son[u]) dfs2(v, v);
    }

    // ---- 线段树 ----
    void build(int p, int l, int r, const vector<ll> &a) {
        if (l == r) { sum[p] = a[l]; return; }
        int m = (l + r) >> 1;
        build(p << 1, l, m, a); build(p << 1 | 1, m + 1, r, a);
        sum[p] = sum[p << 1] + sum[p << 1 | 1];
    }
    void apply(int p, int l, int r, ll v) { sum[p] += v * (r - l + 1); lazy[p] += v; }
    void push(int p, int l, int r) {
        if (!lazy[p]) return;
        int m = (l + r) >> 1;
        apply(p << 1, l, m, lazy[p]);
        apply(p << 1 | 1, m + 1, r, lazy[p]);
        lazy[p] = 0;
    }
    void seg_add(int p, int l, int r, int ql, int qr, ll v) {
        if (ql <= l && r <= qr) { apply(p, l, r, v); return; }
        push(p, l, r);
        int m = (l + r) >> 1;
        if (ql <= m) seg_add(p << 1, l, m, ql, qr, v);
        if (qr > m) seg_add(p << 1 | 1, m + 1, r, ql, qr, v);
        sum[p] = sum[p << 1] + sum[p << 1 | 1];
    }
    ll seg_query(int p, int l, int r, int ql, int qr) {
        if (ql <= l && r <= qr) return sum[p];
        push(p, l, r);
        int m = (l + r) >> 1;
        ll res = 0;
        if (ql <= m) res += seg_query(p << 1, l, m, ql, qr);
        if (qr > m) res += seg_query(p << 1 | 1, m + 1, r, ql, qr);
        return res;
    }

    // ---- 对外接口 ----
    void build_all(int root = 1) {
        dep[root] = 0; dfs1(root, 0); dfs2(root, root);
        vector<ll> a(n + 1);
        for (int u = 1; u <= n; ++u) a[dfn[u]] = val[u];
        build(1, 1, n, a);
    }
    void path_add(int u, int v, ll x) {
        while (top[u] != top[v]) {
            if (dep[top[u]] < dep[top[v]]) swap(u, v);
            seg_add(1, 1, n, dfn[top[u]], dfn[u], x);
            u = fa[top[u]];
        }
        if (dep[u] > dep[v]) swap(u, v);
        seg_add(1, 1, n, dfn[u], dfn[v], x);
    }
    ll path_sum(int u, int v) {
        ll res = 0;
        while (top[u] != top[v]) {
            if (dep[top[u]] < dep[top[v]]) swap(u, v);
            res += seg_query(1, 1, n, dfn[top[u]], dfn[u]);
            u = fa[top[u]];
        }
        if (dep[u] > dep[v]) swap(u, v);
        res += seg_query(1, 1, n, dfn[u], dfn[v]);
        return res;
    }
    void subtree_add(int u, ll x) { seg_add(1, 1, n, dfn[u], dfn[u] + sz[u] - 1, x); }
    ll subtree_sum(int u) { return seg_query(1, 1, n, dfn[u], dfn[u] + sz[u] - 1); }
    int lca(int u, int v) const {
        while (top[u] != top[v]) {
            if (dep[top[u]] < dep[top[v]]) swap(u, v);
            u = fa[top[u]];
        }
        return dep[u] < dep[v] ? u : v;
    }
};
```

**边权版本**：把边权下放到深度较大的端点，`path_add/path_sum` 最后一段改成 `[dfn[u]+1, dfn[v]]`。

```cpp
// 边权版路径操作（只列差异）
// 1. 建树时：val[v] = w(u->v)，即把边权挂到儿子上
// 2. 路径查询/修改：最后一段用 dfn[u]+1 而不是 dfn[u]
void path_add_edge(int u, int v, ll x) {
    // while 循环内不变（top 那一段整条链都有效）
    // 最后一段：
    //   if (dep[u] > dep[v]) swap(u, v);
    //   if (dfn[u] + 1 <= dfn[v]) seg_add(1,1,n, dfn[u]+1, dfn[v], x);
}
```

---

## 4. 树上差分

| 需求 | 差分方式 |
|---|---|
| 点权：路径 `u-v` 加 `x` | `d[u]+=x; d[v]+=x; d[lca]-=x; d[fa[lca]]-=x;` |
| 边权：路径 `u-v` 加 `x` | `d[u]+=x; d[v]+=x; d[lca]-=2x;` |
| 点权：子树加（dfn 区间） | 线段树/树状数组 |
| 路径覆盖次数统计 | 边权差分后子树求和 |

```cpp
#include <bits/stdc++.h>
using namespace std;

// 点权路径差分 + 自底向上求和
struct TreeDiff {
    int n, LOG;
    vector<vector<int>> g, up;
    vector<int> dep, fa;
    vector<long long> d;

    explicit TreeDiff(int n_) : n(n_), LOG(std::__lg(n_ + 1) + 1),
        g(n_ + 1), up(LOG + 1, vector<int>(n_ + 1)), dep(n_ + 1),
        fa(n_ + 1), d(n_ + 1, 0) {}

    void addEdge(int u, int v) { g[u].push_back(v); g[v].push_back(u); }
    void dfs(int u, int p) {
        up[0][u] = p; dep[u] = dep[p] + 1;
        for (int k = 1; k < LOG; ++k) up[k][u] = up[k - 1][up[k - 1][u]];
        for (int v : g[u]) if (v != p) dfs(v, u);
    }
    void build(int root = 1) { dep[root] = 0; up[0][root] = root; dfs(root, root); }
    int lca(int u, int v) const {
        if (dep[u] < dep[v]) swap(u, v);
        int dd = dep[u] - dep[v];
        for (int k = 0; k < LOG; ++k) if (dd >> k & 1) u = up[k][u];
        if (u == v) return u;
        for (int k = LOG - 1; k >= 0; --k)
            if (up[k][u] != up[k][v]) { u = up[k][u]; v = up[k][v]; }
        return up[0][u];
    }
    void path_add(int u, int v, long long x) {        // 点权版本
        int w = lca(u, v);
        d[u] += x; d[v] += x; d[w] -= x;
        if (up[0][w] != w) d[up[0][w]] -= x;
    }
    // 自底向上累加，得到每个点的最终值
    void collect(int u, int p = 0) {
        for (int v : g[u]) if (v != p) { collect(v, u); d[u] += d[v]; }
    }
};
```

---

## 5. 虚树（关键点 + LCA 建小树）

**用途**：多次询问，每次给定 `k` 个关键点，`Σk` 不大，要在虚树上做 DP/统计。

**建法**：关键点按 `dfn` 排序，加入相邻点的 LCA，去重排序，用栈维护右链。

```cpp
#include <bits/stdc++.h>
using namespace std;

// 函数式虚树构建：返回虚树邻接表（下标 0..n）
// 参数：key 关键点列表；n 原树点数；dfn 入序；lca(u,v) 函数
vector<vector<int>> build_virtual_tree(vector<int> key, int n,
                                       const vector<int> &dfn,
                                       const function<int(int,int)> &lca) {
    auto cmp = [&](int a, int b) { return dfn[a] < dfn[b]; };
    sort(key.begin(), key.end(), cmp);
    key.erase(unique(key.begin(), key.end()), key.end());

    int k = (int)key.size();
    for (int i = 0; i + 1 < k; ++i) key.push_back(lca(key[i], key[i + 1]));
    sort(key.begin(), key.end(), cmp);
    key.erase(unique(key.begin(), key.end()), key.end());

    vector<vector<int>> vt(n + 1);
    vector<int> stk;
    for (int u : key) {
        // 栈顶若不是 u 的祖先，就弹栈（key 已含所有所需 LCA，弹到合适位置即可）
        while (!stk.empty() && lca(stk.back(), u) != stk.back()) stk.pop_back();
        if (!stk.empty()) vt[stk.back()].push_back(u);
        stk.push_back(u);
    }
    return vt;
}
```

**关键点**：

1. `key` 里**必须包含根**，或至少包含所有需要的 LCA（把 `root` 加进去最稳）。
2. **"两两相邻 LCA"** 指的是排序后**相邻两个关键点**的 LCA，不是所有点对。
3. 弹栈条件用 `lca(stk.back(), u) != stk.back()`，**不要**用 `dfn` 直接比较（边界容易错）。
4. 虚树边权 = 原树上两点距离（`dist(u,v) = dep[u]+dep[v]-2*dep[lca]`），DP 时要还原。
5. 虚树大小 `O(k)`。**多测时不要 `O(n)` 清空 `vt`**，只清本次用到的点：

```cpp skip
// 查询结束后只清理本次虚树用到的节点（片段，依赖 key/vt，单独编译不通过）
vector<int> used = key;                     // key 是去重后的节点集合
for (int u : used) vt[u].clear();
```

6. **虚树题最常见的坑**：`key` 没加根导致根不在虚树里，DP 时"虚根"的父子关系错位。

---

## 6. 树的重心 / 直径

```cpp
#include <bits/stdc++.h>
using namespace std;

// 重心：删掉后最大子树最小的点（最多两个）
struct TreeCenter {
    int n;
    vector<vector<int>> g;
    vector<int> sz;
    int best = INT_MAX, center = 0;

    explicit TreeCenter(int n_) : n(n_), g(n_ + 1), sz(n_ + 1) {}
    void addEdge(int u, int v) { g[u].push_back(v); g[v].push_back(u); }

    void dfs(int u, int p) {
        sz[u] = 1;
        int mx = 0;
        for (int v : g[u]) if (v != p) {
            dfs(v, u);
            sz[u] += sz[v];
            mx = max(mx, sz[v]);
        }
        mx = max(mx, n - sz[u]);
        if (mx < best) { best = mx; center = u; }
    }
    int find_center(int root = 1) { dfs(root, 0); return center; }
};

// 直径：两次 DFS（边权非负）/ 树形 DP（有负权时）
struct TreeDiameter {
    int n;
    vector<vector<pair<int,int>>> g;    // (v, w)
    long long best = 0;

    explicit TreeDiameter(int n_) : n(n_), g(n_ + 1) {}
    void addEdge(int u, int v, int w) { g[u].push_back({v, w}); g[v].push_back({u, w}); }

    // 两次 DFS：返回直径长度和端点
    pair<long long,int> farthest(int s) const {
        vector<long long> d(n + 1, -1);
        vector<int> stk{s};
        d[s] = 0;
        int far = s;
        while (!stk.empty()) {
            int u = stk.back(); stk.pop_back();
            if (d[u] > d[far]) far = u;
            for (auto [v, w] : g[u]) if (d[v] < 0) { d[v] = d[u] + w; stk.push_back(v); }
        }
        return {d[far], far};
    }
    long long diameter() const {
        auto [_, a] = farthest(1);
        auto [len, b] = farthest(a);
        (void)b;
        return len;
    }
};
```

---

## 7. 换根 DP（二次扫描）

**问题**：求每个点到所有点的距离之和。

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;

// 换根 DP：ans[u] = sum_v dist(u,v)
struct Reroot {
    int n;
    vector<vector<int>> g;
    vector<int> sz;
    vector<ll> down, ans;

    explicit Reroot(int n_) : n(n_), g(n_ + 1), sz(n_ + 1), down(n_ + 1, 0), ans(n_ + 1, 0) {}
    void addEdge(int u, int v) { g[u].push_back(v); g[v].push_back(u); }

    void dfs1(int u, int p) {
        sz[u] = 1;
        for (int v : g[u]) if (v != p) {
            dfs1(v, u);
            sz[u] += sz[v];
            down[u] += down[v] + sz[v];       // 子树内到 u 的距离和
        }
    }
    void dfs2(int u, int p) {
        for (int v : g[u]) if (v != p) {
            ans[v] = ans[u] - sz[v] + (n - sz[v]);   // 经典换根公式
            dfs2(v, u);
        }
    }
    vector<ll> solve(int root = 1) {
        dfs1(root, 0);
        ans[root] = down[root];
        dfs2(root, 0);
        return ans;
    }
};
```

**换根公式记忆**：`ans[v] = ans[u] - sz[v] + (n - sz[v])`（`v` 的子树点变近了 `sz[v]`，其余点变远了 `n-sz[v]`）。

---

## 8. 树论题单（按难度）

| 题 | 考点 |
|---|---|
| 洛谷 P3379 | LCA 模板 |
| 洛谷 P3384 | 树链剖分模板（路径加、子树加） |
| 洛谷 P3128 | 树上差分 |
| 洛谷 P2495 | 虚树 + 树形 DP |
| 洛谷 P3177 | 树形背包 |
| 洛谷 P3478 | 换根 DP |
| 洛谷 P1099 | 树的直径 + 二分 |
| CF 600E | dsu on tree |
| 洛谷 P4292 | 点分治 + 单调队列 |

---

## 9. 思路与例题索引

| 算法 | 思路一句话 | 用途 | 例题 |
|---|---|---|---|
| DFS 序 | 子树 = 区间 `[dfn, dfn+sz-1]` | 子树查询转区间 | P3178 树上操作 |
| 倍增 LCA | 先抬到同深度，再一起跳 | LCA | P3379【模板】LCA |
| 树剖 LCA | 跳链头，深度大的先跳 | LCA（常数小） | P3379 |
| 欧拉序 + ST | `first[u]..first[v]` 中深度最小的点 | `O(1)` LCA | P3379 加强版 |
| Tarjan 离线 LCA | DFS 回溯时并查集合并 | 离线 LCA | P3379 变体 |
| 树链剖分 | 重儿子 + 链头，路径拆成 `O(log n)` 段 | 路径加/路径和 | P3384【模板】树剖 |
| 边权树剖 | 边权下放到儿子，查询 `dfn[u]+1` | 路径边权 | P4315 月下毛景树 |
| 树上差分 | 点权：`u+1,v+1,lca-1,fa[lca]-1` | 路径覆盖计数 | P3128 Max Flow |
| 虚树 | 关键点 + 相邻 LCA，栈维护右链 | 多次小规模询问 | P2495 消耗战 |
| 树的重心 | 最大子树最小 | 点分治、树的重心 | P1395 会议 |
| 树的直径 | 两次 DFS / 树形 DP | 最长路径 | P1099 树网的核 |
| 换根 DP | `ans[v] = ans[u] - sz[v] + (n-sz[v])` | 全点到点距离和 | P3478 STA-Station |

**关键**：树剖后路径查询最后一段"包含 LCA"；边权版要 `+1`。
