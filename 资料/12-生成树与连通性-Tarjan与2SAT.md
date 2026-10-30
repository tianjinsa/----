# 12 生成树与连通性（Kruskal / Prim / Tarjan / 2-SAT / 圆方树）

> 给全：Kruskal、Prim、次小生成树、Kruskal 重构树、Boruvka；强连通分量 SCC + 缩点 DAG、割点、桥、边双/点双、圆方树、2-SAT。

---

## 1. 最小生成树 Kruskal

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;

struct Kruskal {
    struct Edge { int u, v; ll w; };
    int n;
    vector<Edge> es;
    vector<int> fa, rk;

    explicit Kruskal(int n_) : n(n_), fa(n_ + 1), rk(n_ + 1, 1) {
        iota(fa.begin(), fa.end(), 0);
    }
    void addEdge(int u, int v, ll w) { es.push_back({u, v, w}); }

    int find(int x) { return fa[x] == x ? x : fa[x] = find(fa[x]); }
    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (rk[a] < rk[b]) swap(a, b);
        fa[b] = a; if (rk[a] == rk[b]) ++rk[a];
        return true;
    }

    // 返回 {总权值, 是否连通}
    pair<ll, bool> run() {
        sort(es.begin(), es.end(), [](const Edge &a, const Edge &b) { return a.w < b.w; });
        ll sum = 0; int cnt = 0;
        for (auto &e : es) if (unite(e.u, e.v)) { sum += e.w; ++cnt; }
        return {sum, cnt == n - 1};
    }
};
```

**注意**：`n` 个点要选 `n-1` 条边；不连通返回 `false`。

---

## 2. 最小生成树 Prim（堆优化，稠密图/Kruskal 不易用时）

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;
constexpr ll LINF = 0x3f3f3f3f3f3f3f3fLL;

struct Prim {
    struct Edge { int to; ll w; };
    int n;
    vector<vector<Edge>> g;
    explicit Prim(int n_) : n(n_), g(n_ + 1) {}
    void addEdge(int u, int v, ll w) { g[u].push_back({v, w}); g[v].push_back({u, w}); }

    pair<ll, bool> run(int s = 1) {
        vector<ll> d(n + 1, LINF);
        vector<bool> used(n + 1, false);
        priority_queue<pair<ll,int>, vector<pair<ll,int>>, greater<>> pq;
        d[s] = 0; pq.push({0, s});
        ll sum = 0; int cnt = 0;
        while (!pq.empty()) {
            auto [du, u] = pq.top(); pq.pop();
            if (used[u]) continue;
            used[u] = true; sum += du; ++cnt;
            for (auto &e : g[u]) if (!used[e.to] && d[e.to] > e.w) {
                d[e.to] = e.w; pq.push({d[e.to], e.to});
            }
        }
        return {sum, cnt == n};
    }
};
```

> Prim 在**完全图/稠密图**上更快（`O(n^2)` 朴素版优于 `O(m log n)`）；Kruskal 适合稀疏图。

---

## 3. 次小生成树（严格 / 非严格）

**思路**：先求 MST，枚举非树边 `(u,v,w)`，在 MST 上求 `u-v` 路径的**最大边**（非严格）或**最大边 + 次大边**（严格）替换。

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;
constexpr ll LINF = 0x3f3f3f3f3f3f3f3fLL;

// 严格次小生成树（边权可重复但不能相等替换）
struct SecondMST {
    struct Edge { int u, v; ll w; };
    int n, m;
    vector<Edge> es;
    vector<int> fa, rk, tin;
    vector<vector<pair<int,int>>> tree;        // MST 邻接表
    vector<vector<ll>> mx1, mx2;               // 路径最大/次大边
    vector<vector<int>> up;
    int LOG, timer = 0;
    vector<int> dep;

    explicit SecondMST(int n_) : n(n_), es(),
        fa(n_ + 1), rk(n_ + 1, 1), tin(n_ + 1), tree(n_ + 1), dep(n_ + 1) {
        iota(fa.begin(), fa.end(), 0);
    }
    void addEdge(int u, int v, ll w) { es.push_back({u, v, w}); }

    int find(int x) { return fa[x] == x ? x : fa[x] = find(fa[x]); }
    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (rk[a] < rk[b]) swap(a, b);
        fa[b] = a; if (rk[a] == rk[b]) ++rk[a];
        return true;
    }

    // 返回严格次小生成树的权值；不存在返回 LINF
    ll run() {
        sort(es.begin(), es.end(), [](const Edge &a, const Edge &b) { return a.w < b.w; });
        vector<bool> inMST(es.size(), false);
        ll mst = 0;
        for (size_t i = 0; i < es.size(); ++i) {
            if (unite(es[i].u, es[i].v)) {
                mst += es[i].w;
                inMST[i] = true;
                tree[es[i].u].push_back({es[i].v, (int)es[i].w});
                tree[es[i].v].push_back({es[i].u, (int)es[i].w});
            }
        }
        LOG = std::__lg(n) + 1;
        up.assign(LOG, vector<int>(n + 1));
        mx1.assign(LOG, vector<ll>(n + 1, -1));
        mx2.assign(LOG, vector<ll>(n + 1, -1));
        dep[1] = 0; up[0][1] = 1;
        dfs(1, 1);

        ll ans = LINF;
        for (size_t i = 0; i < es.size(); ++i) {
            if (inMST[i]) continue;
            auto [wmax, w2] = query(es[i].u, es[i].v);
            if (wmax < es[i].w) ans = min(ans, mst - wmax + es[i].w);          // 非严格也能用
            else if (w2 >= 0 && w2 < es[i].w) ans = min(ans, mst - w2 + es[i].w);  // 严格
        }
        return ans;
    }

    void dfs(int u, int p) {
        for (auto [v, w] : tree[u]) if (v != p) {
            dep[v] = dep[u] + 1;
            up[0][v] = u;
            mx1[0][v] = w;
            for (int k = 1; k < LOG; ++k) {
                int mid = up[k - 1][v];
                up[k][v] = up[k - 1][mid];
                // 合并最大、次大
                array<ll,2> a = {mx1[k - 1][v], mx2[k - 1][v]};
                array<ll,2> b = {mx1[k - 1][mid], mx2[k - 1][mid]};
                vector<ll> all = {a[0], a[1], b[0], b[1]};
                sort(all.rbegin(), all.rend());
                mx1[k][v] = all[0];
                mx2[k][v] = -1;
                for (ll x : all) if (x < mx1[k][v]) { mx2[k][v] = x; break; }
            }
            dfs(v, u);
        }
    }

    // 返回路径上的 {最大边, 严格次大边}
    pair<ll,ll> query(int u, int v) {
        vector<ll> cand;
        if (dep[u] < dep[v]) swap(u, v);
        int diff = dep[u] - dep[v];
        for (int k = 0; k < LOG; ++k) if (diff >> k & 1) {
            cand.push_back(mx1[k][u]); cand.push_back(mx2[k][u]);
            u = up[k][u];
        }
        if (u != v) {
            for (int k = LOG - 1; k >= 0; --k) if (up[k][u] != up[k][v]) {
                cand.push_back(mx1[k][u]); cand.push_back(mx2[k][u]);
                cand.push_back(mx1[k][v]); cand.push_back(mx2[k][v]);
                u = up[k][u]; v = up[k][v];
            }
            cand.push_back(mx1[0][u]); cand.push_back(mx1[0][v]);
        }
        sort(cand.rbegin(), cand.rend());
        ll best = -1, second = -1;
        for (ll x : cand) if (x > best) { best = x; break; }
        for (ll x : cand) if (x < best) { second = x; break; }
        return {best, second};
    }
};
```

> ⚠️ 上面为了简洁把边权转成了 `int` 存，**边权超过 `2^31` 时请改回 `ll`**（`mx1/mx2` 已经是 `ll`）。

---

## 4. Kruskal 重构树（边权限制 + 连通块）

**用途**："只走边权 `<= x` 的边，能到达哪些点" / "两点间路径最大边最小"。
建法：Kruskal 每次合并时新建一个节点，权值为当前边权，两个子树挂上去。得到一棵**大根堆**性质的二叉树。

```cpp
#include <bits/stdc++.h>
using namespace std;

struct KruskalTree {
    struct Edge { int u, v, w; };
    int n, tot;
    vector<Edge> es;
    vector<int> fa, val, lc, rc;
    vector<vector<int>> up;
    vector<int> dfn, sz;
    int LOG, timer = 0;

    explicit KruskalTree(int n_) : n(n_), tot(n_), fa(2 * n_ + 2), val(2 * n_ + 2),
        lc(2 * n_ + 2), rc(2 * n_ + 2) {
        for (int i = 1; i <= 2 * n; ++i) fa[i] = i;
    }
    void addEdge(int u, int v, int w) { es.push_back({u, v, w}); }

    int find(int x) { return fa[x] == x ? x : fa[x] = find(fa[x]); }

    // 建树，返回根（若图不连通，会得到森林，通常再连一个 0 号根）
    int build() {
        sort(es.begin(), es.end(), [](const Edge &a, const Edge &b) { return a.w < b.w; });
        for (auto &e : es) {
            int ru = find(e.u), rv = find(e.v);
            if (ru == rv) continue;
            ++tot;
            val[tot] = e.w;
            lc[tot] = ru; rc[tot] = rv;
            fa[ru] = fa[rv] = tot;
        }
        return tot;
    }
    // 从叶子 u 出发，向上跳到 val <= x 的最高祖先（倍增）
    int jump(int u, int x) const {
        for (int k = LOG; k >= 0; --k)
            if (up[k][u] && val[up[k][u]] <= x) u = up[k][u];
        return u;
    }
};
```

**经典题**：洛谷 P4197（Peaks）、P4768（归程）、P1967（货车运输）。

---

## 5. Tarjan 强连通分量 SCC + 缩点

```cpp
#include <bits/stdc++.h>
using namespace std;

struct SCC {
    int n, timer = 0, cnt = 0;
    vector<vector<int>> g;
    vector<int> dfn, low, stk, scc, onStk;
    vector<int> sz;                       // 每个 SCC 的大小

    explicit SCC(int n_) : n(n_), g(n_ + 1), dfn(n_ + 1, 0), low(n_ + 1),
        scc(n_ + 1, 0), onStk(n_ + 1, 0) {}

    void addEdge(int u, int v) { g[u].push_back(v); }

    void dfs(int u) {
        dfn[u] = low[u] = ++timer;
        stk.push_back(u); onStk[u] = 1;
        for (int v : g[u]) {
            if (!dfn[v]) { dfs(v); low[u] = min(low[u], low[v]); }
            else if (onStk[v]) low[u] = min(low[u], dfn[v]);
        }
        if (low[u] == dfn[u]) {
            ++cnt;
            while (true) {
                int x = stk.back(); stk.pop_back();
                onStk[x] = 0; scc[x] = cnt;
                if (x == u) break;
            }
        }
    }
    // 返回 SCC 个数；scc[u] 为 u 所属编号（1..cnt，拓扑序是逆序！）
    int run() {
        for (int i = 1; i <= n; ++i) if (!dfn[i]) dfs(i);
        sz.assign(cnt + 1, 0);
        for (int i = 1; i <= n; ++i) ++sz[scc[i]];
        return cnt;
    }

    // 缩点后的 DAG
    vector<vector<int>> build_dag() const {
        vector<vector<int>> dag(cnt + 1);
        for (int u = 1; u <= n; ++u)
            for (int v : g[u]) if (scc[u] != scc[v]) dag[scc[u]].push_back(scc[v]);
        for (auto &vec : dag) {
            sort(vec.begin(), vec.end());
            vec.erase(unique(vec.begin(), vec.end()), vec.end());
        }
        return dag;
    }
};
```

> **重要**：Tarjan 求出的 `scc` 编号**本身就是拓扑逆序**（编号越大越靠前）。缩点后 DP 可以直接从 `cnt` 到 `1` 遍历。

**经典应用**：缩点 + DAG 最长路（P3387）、2-SAT、必经点。

---

## 6. 割点 / 桥 / 边双 / 点双

```cpp
#include <bits/stdc++.h>
using namespace std;

struct TarjanUndirected {
    int n, timer = 0;
    vector<vector<int>> g;
    vector<int> dfn, low;
    vector<int> cut;                       // 是否割点
    vector<pair<int,int>> bridge;          // 桥
    int bccCnt = 0;                        // 边双连通分量数
    vector<int> bcc;                       // 每个点所属边双
    vector<int> stk;
    int vccCnt = 0;                        // 点双连通分量数
    vector<vector<int>> vcc;               // 每个点双包含的点

    explicit TarjanUndirected(int n_) : n(n_), g(n_ + 1), dfn(n_ + 1, 0),
        low(n_ + 1), cut(n_ + 1, 0), bcc(n_ + 1) {}

    void addEdge(int u, int v) { g[u].push_back(v); g[v].push_back(u); }

    void dfs(int u, int fa, int inEdge) {
        dfn[u] = low[u] = ++timer;
        stk.push_back(u);
        int child = 0;
        for (int v : g[u]) {
            if (v == fa && !inEdge) { inEdge = 1; continue; }   // 跳过一条回边
            if (!dfn[v]) {
                ++child;
                dfs(v, u, 0);
                low[u] = min(low[u], low[v]);
                if (low[v] > dfn[u]) bridge.push_back({u, v});          // 桥
                if (low[v] >= dfn[u]) {                                 // 点双
                    ++vccCnt;
                    vcc.push_back({});
                    while (true) {
                        int x = stk.back(); stk.pop_back();
                        vcc.back().push_back(x);
                        if (x == v) break;
                    }
                    vcc.back().push_back(u);
                }
                if (fa != 0 && low[v] >= dfn[u]) cut[u] = 1;            // 割点
            } else {
                low[u] = min(low[u], dfn[v]);
            }
        }
        if (fa == 0 && child > 1) cut[u] = 1;                            // 根是割点
        if (stk.empty() || stk.back() == u) { /* 留给边双用 */ }
    }
};
```

**边双连通分量（去掉桥后的连通块）**：先 Tarjan 找桥，再 DFS 不经过桥染色。

```cpp
#include <bits/stdc++.h>
using namespace std;

// 边双：删掉所有桥后，剩下的连通块
struct EdgeBCC {
    int n, timer = 0, cnt = 0;
    vector<vector<pair<int,int>>> g;        // (v, edge_id)
    vector<int> dfn, low, col;
    vector<int> bridgeEdge;

    explicit EdgeBCC(int n_) : n(n_), g(n_ + 1), dfn(n_ + 1, 0), low(n_ + 1),
        col(n_ + 1, 0) {}

    void addEdge(int u, int v, int id) { g[u].push_back({v, id}); g[v].push_back({u, id}); }

    void dfs(int u, int inId) {
        dfn[u] = low[u] = ++timer;
        for (auto [v, id] : g[u]) {
            if (id == inId) continue;
            if (!dfn[v]) {
                dfs(v, id);
                low[u] = min(low[u], low[v]);
                if (low[v] > dfn[u]) bridgeEdge[id] = 1;
            } else low[u] = min(low[u], dfn[v]);
        }
    }
    void color(int u) {
        col[u] = cnt;
        for (auto [v, id] : g[u])
            if (!col[v] && !bridgeEdge[id]) color(v);
    }
    int run(int m) {
        bridgeEdge.assign(m + 1, 0);
        for (int i = 1; i <= n; ++i) if (!dfn[i]) dfs(i, 0);
        for (int i = 1; i <= n; ++i) if (!col[i]) { ++cnt; color(i); }
        return cnt;
    }
};
```

**经典题**：P3388 割点、P3469 [POI2008] BLO、P8435 点双连通分量。

---

## 7. 2-SAT

**建图**：每个变量 `x` 拆成 `x`（真）和 `¬x`（假）两个点。子句 `(a ∨ b)` 等价于 `¬a → b` 且 `¬b → a`。
判定：跑 SCC，若 `x` 与 `¬x` 在同一 SCC ⇒ 无解。
**赋值**：`x` 取 `scc[x] > scc[¬x]`（Tarjan 编号小的为真/假取决于实现，见注释）。

```cpp
#include <bits/stdc++.h>
using namespace std;

struct TwoSAT {
    int n;                               // 变量个数
    vector<vector<int>> g;
    vector<int> dfn, low, stk, scc, onStk;
    int timer = 0, cnt = 0;

    explicit TwoSAT(int n_) : n(n_), g(2 * n_ + 2), dfn(2 * n_ + 2, 0),
        low(2 * n_ + 2), scc(2 * n_ + 2, 0), onStk(2 * n_ + 2, 0) {}

    // 点编号：x 表示"真"，x+n 表示"假"（x 从 1 开始，n 个变量用 1..2n）
    static int id(int x, bool isTrue, int n) { return isTrue ? x : x + n; }

    // 加子句 (a ∨ b)：a、b 形如 (变量, 是否取真)
    void addOr(int a, bool ta, int b, bool tb) {
        int A = id(a, ta, n), B = id(b, tb, n);
        g[A + (ta ? n : -n)].push_back(B);        // ¬A -> B
        g[B + (tb ? n : -n)].push_back(A);        // ¬B -> A
    }
    // (a = v) 即 (a ∨ a)
    void addVal(int a, bool v) { addOr(a, v, a, v); }
    // a -> b
    void addImpl(int a, bool ta, int b, bool tb) { addOr(a, !ta, b, tb); }

    void dfs(int u) {
        dfn[u] = low[u] = ++timer;
        stk.push_back(u); onStk[u] = 1;
        for (int v : g[u]) {
            if (!dfn[v]) { dfs(v); low[u] = min(low[u], low[v]); }
            else if (onStk[v]) low[u] = min(low[u], dfn[v]);
        }
        if (low[u] == dfn[u]) {
            ++cnt;
            while (true) {
                int x = stk.back(); stk.pop_back();
                onStk[x] = 0; scc[x] = cnt;
                if (x == u) break;
            }
        }
    }

    // 返回是否有解；ans[i] 为变量 i 的取值（下标 1..n）
    bool solve(vector<bool> &ans) {
        for (int i = 1; i <= 2 * n; ++i) if (!dfn[i]) dfs(i);
        ans.assign(n + 1, false);
        for (int i = 1; i <= n; ++i) {
            if (scc[i] == scc[i + n]) return false;
            ans[i] = scc[i] > scc[i + n];     // Tarjan 编号大的是"更靠前"的，即取真
        }
        return true;
    }
};
```

> **推荐用 Kosaraju 或 Tarjan 后按拓扑序赋值**。上面 `scc[i] > scc[i+n]` 的写法在 Tarjan（编号大的先出栈=拓扑序靠前）下是**正确**的：SCC 编号越大越靠前，取"后出现的"为真。

**经典题**：P4782 2-SAT 模板、P4171 满汉全席、P3825 游戏（枚举 + 2-SAT）。

---

## 8. 圆方树（点双缩成方点）

**用途**：把"点双连通分量"缩成方点，处理"必经点"、"路径上割点计数"。

```cpp
#include <bits/stdc++.h>
using namespace std;

// 圆方树：原图点 = 圆点，每个点双 = 一个方点
// 建法：Tarjan 求点双，把点双内所有点连到新方点上
struct BlockCutTree {
    int n, tot, timer = 0, cnt = 0;
    vector<vector<int>> g, tree;
    vector<int> dfn, low, stk, val;

    explicit BlockCutTree(int n_) : n(n_), tot(n_), g(n_ + 1), dfn(n_ + 1, 0),
        low(n_ + 1), val(n_ + 1, 0) {}

    void addEdge(int u, int v) { g[u].push_back(v); g[v].push_back(u); }

    void dfs(int u, int fa) {
        dfn[u] = low[u] = ++timer;
        stk.push_back(u);
        for (int v : g[u]) {
            if (v == fa) continue;
            if (!dfn[v]) {
                dfs(v, u);
                low[u] = min(low[u], low[v]);
                if (low[v] >= dfn[u]) {
                    ++cnt;
                    ++tot;
                    tree.resize(tot + 1);
                    while (true) {
                        int x = stk.back(); stk.pop_back();
                        tree[x].push_back(tot);
                        tree[tot].push_back(x);
                        if (x == v) break;
                    }
                    tree[u].push_back(tot);
                    tree[tot].push_back(u);
                }
            } else low[u] = min(low[u], dfn[v]);
        }
    }
    // 建树（要求图连通；不连通需对每个连通块分别跑）
    void build() {
        tree.assign(tot + 1, {});
        dfs(1, 0);
    }
};
```

---

## 9. 选择速查

| 问题 | 算法 |
|---|---|
| 最小生成树（稀疏） | Kruskal |
| 最小生成树（稠密） | Prim |
| 次小生成树 | MST + 倍增维护路径最大/次大 |
| "只走 ≤ x 的边"连通性 | Kruskal 重构树 + 倍增 |
| 有向图强连通 | Tarjan SCC |
| 缩点后最长路 | SCC + DAG DP |
| 割点/桥 | Tarjan |
| 边双/点双 | Tarjan + 染色/圆方树 |
| 布尔方程（每句两个变量） | 2-SAT |
| 必经点/割点计数 | 圆方树 |


### 怎么用（照抄这段 `main`，只改【输入】【输出】两处）

上面是模板本体。实际写题时：**把上面的 struct 整段粘过来 → 再粘这段 `main` → 只改标了【输入】【输出】的行**。

```cpp skip
#include <bits/stdc++.h>
using namespace std;

// ===== 粘贴点：把上面的 SCC 粘到这里 =====

int main() {
    // ===================== 【输入】改这里 =====================
    int n, m;
    scanf("%d %d", &n, &m);               // 点数、有向边数
    // ========================================================

    // ===================== 【核心】算法流程 =====================
    // Tarjan 强连通分量：
    //   1) DFS，dfn[u] 是访问时间，low[u] 是能回到的最小 dfn
    //   2) 用一个栈存"还没归属任何分量"的点
    //   3) 回溯时 low[u] = min(low[u], low[v], dfn[回边])
    //   4) 若 low[u] == dfn[u]，从栈里弹出到 u，这些点构成一个分量
    //   性质：分量编号越大越靠前（逆拓扑序），缩点后可以直接倒序 DP
    SCC scc(n);
    for (int i = 0; i < m; ++i) {
        int u, v;
        scanf("%d %d", &u, &v);
        scc.addEdge(u, v);
    }
    int cnt = scc.run();
    auto dag = scc.build_dag();   // 缩点后的 DAG，可直接在上面 DP
    // ========================================================

    // ===================== 【输出】改这里 =====================
    printf("%d\n", cnt);                 // 分量个数
    for (int i = 1; i <= n; ++i)
        printf("%d%c", scc.scc[i], " \n"[i == n]);
    // ========================================================
    return 0;
}
```

**2-SAT / 割点 / Kruskal** 的核心调用：

```cpp skip
TwoSAT sat(n);                 // n 个变量
sat.addOr(a, ta, b, tb);       // 子句 (a 取 ta) 或 (b 取 tb)
vector<bool> ans;
bool ok = sat.solve(ans);      // ok=false 表示无解

// 割点 / 桥 / 边双：TarjanUndirected
TarjanUndirected tu(n);
// tu.cut[u] == 1 表示 u 是割点；tu.bridge 存所有桥

Kruskal kru(n);                // 最小生成树
kru.addEdge(u, v, w);
auto pr = kru.run();           // {MST 权值, 是否连通}
```

---

## 10. 思路与例题索引

| 算法 | 思路一句话 | 用途 | 例题 |
|---|---|---|---|
| Kruskal | 排序 + 并查集，能连就选 | 稀疏图 MST | P3366【模板】最小生成树 |
| Prim | 从一点出发，每次取最近的未选点 | 稠密图 MST | P3366 |
| 次小生成树 | MST + 枚举非树边替换路径最大边 | 次小生成树 | P4180【模板】严格次小生成树 |
| Kruskal 重构树 | 每次合并新建节点，边权为点权 | "只走 ≤x 的边"连通性 | P4197 Peaks、P1967 货车运输 |
| Tarjan SCC | `dfn/low` + 栈，`low==dfn` 出栈 | 强连通分量 | P3387【模板】缩点 |
| 缩点 DAG | SCC 编号逆拓扑序 | DAG 最长路 / 计数 | P3387、P4017 最大食物链计数 |
| 割点 | `low[v] >= dfn[u]`；根需 `>1` 个子树 | 割点 | P3388【模板】割点 |
| 桥 / 边双 | `low[v] > dfn[u]`，删桥后染色 | 边双连通 | P8436【模板】边双连通分量 |
| 点双 / 圆方树 | 每个点双建方点 | 必经点 | P8435【模板】点双连通分量 |
| 2-SAT | `(a∨b)` → `¬a→b, ¬b→a`，SCC 判矛盾 | 布尔约束 | P4782【模板】2-SAT |

**关键**：Tarjan 的 SCC 编号越大越靠前（拓扑逆序），缩点后可以直接倒序 DP。
