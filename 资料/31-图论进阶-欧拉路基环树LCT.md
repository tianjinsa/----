# 31 图论进阶（欧拉路 / 基环树 / 斯坦纳树 / k 短路 / LCT）

> 基础图论（最短路/生成树/Tarjan/网络流）见 [11](11-最短路-Dijkstra与Floyd.md)、[12](12-生成树与连通性-Tarjan与2SAT.md)、[13](13-匹配与网络流.md)。

---

## 1. 欧拉路 / 欧拉回路

**判定**：

- **无向图欧拉回路**：所有点度数为偶，且非孤立点连通。
- **无向图欧拉路**：恰有 0 或 2 个奇度点（有 2 个时从一个奇点出发）。
- **有向图欧拉回路**：所有点入度 = 出度，且基图连通。
- **有向图欧拉路**：至多一个点 `out-in=1`，至多一个点 `in-out=1`，其余相等。

**求解（Hierholzer，边删即走）**：

```cpp
#include <bits/stdc++.h>
using namespace std;

// 无向图欧拉路（边编号 1..m）
struct EulerPath {
    struct Edge { int to, id; };
    int n, m;
    vector<vector<Edge>> g;
    vector<Edge> es;                       // es[id] = {u, v}
    vector<bool> used;
    vector<int> path;                      // 经过的点序列

    explicit EulerPath(int n_, int m_) : n(n_), m(m_), g(n_ + 1),
        used(m_ + 1, false), es(m_ + 1) {}

    void addEdge(int id, int u, int v) {
        es[id] = {u, v};
        g[u].push_back({v, id});
        g[v].push_back({u, id});
    }

    void dfs(int u) {
        while (!g[u].empty()) {
            Edge e = g[u].back();
            g[u].pop_back();
            if (used[e.id]) continue;
            used[e.id] = true;
            dfs(e.to);
            path.push_back(u);             // 后序加入
        }
    }
    // 返回点序列（需要 reverse）；无解返回空
    vector<int> run(int start) {
        dfs(start);
        if ((int)path.size() != m + 1) return {};
        reverse(path.begin(), path.end());
        return path;
    }
};
```

**实例**：P2731 骑马修栅栏、P1341 无序字母对、P7771【模板】欧拉路径。

---

## 2. 基环树

**判定**：`n` 个点 `n` 条边的连通图（恰好一个环）。

**处理套路**：
1. **找环**：拓扑排序剥叶子，剩下的就是环。
2. **环上 DP**：把环拆成链（复制一遍），做区间 DP 或单调队列。
3. **树上部分**：先处理每个环上点的子树，再合并到环。

```cpp
#include <bits/stdc++.h>
using namespace std;

// 找基环树的环（拓扑剥叶）
struct BaseCycleTree {
    int n;
    vector<vector<int>> g;
    vector<int> deg, onCycle;

    explicit BaseCycleTree(int n_) : n(n_), g(n_ + 1), deg(n_ + 1, 0), onCycle(n_ + 1, 0) {}
    void addEdge(int u, int v) { g[u].push_back(v); g[v].push_back(u); ++deg[u]; ++deg[v]; }

    // 返回环上的点（按环序）
    vector<int> find_cycle() {
        queue<int> q;
        vector<int> d = deg;
        vector<bool> removed(n + 1, false);
        for (int i = 1; i <= n; ++i) if (d[i] == 1) q.push(i);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            removed[u] = true;
            for (int v : g[u]) if (!removed[v] && --d[v] == 1) q.push(v);
        }
        vector<int> cyc;
        for (int i = 1; i <= n; ++i) if (!removed[i]) cyc.push_back(i);
        return cyc;                        // 未排序，需要按环序再走一遍
    }
};
```

**实例**：P2607 [ZJOI2008] 骑士、P4381 [IOI2008] Island、P1453 城市环路。

---

## 3. 斯坦纳树

**用途**：在图上选一个连通子图，使**指定的 `k` 个关键点**连通，且总边权最小。
**复杂度**：`O(3^k n + 2^k (m log n))`，`k <= 10` 左右。

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;
constexpr ll LINF = 0x3f3f3f3f3f3f3f3fLL;

// 斯坦纳树：n 个点，k 个关键点（0..k-1）
struct SteinerTree {
    struct Edge { int to; ll w; };
    int n, k;
    vector<vector<Edge>> g;
    vector<int> key;

    SteinerTree(int n_, int k_) : n(n_), k(k_), g(n_ + 1) {}
    void addEdge(int u, int v, ll w) { g[u].push_back({v, w}); g[v].push_back({u, w}); }

    ll solve() {
        int full = 1 << k;
        vector<vector<ll>> dp(1 << k, vector<ll>(n + 1, LINF));
        for (int i = 0; i < k; ++i) dp[1 << i][key[i]] = 0;

        for (int mask = 1; mask < full; ++mask) {
            // 1. 枚举子集合并
            for (int sub = (mask - 1) & mask; sub; sub = (sub - 1) & mask) {
                int other = mask ^ sub;
                if (sub > other) continue;              // 避免重复
                for (int v = 1; v <= n; ++v)
                    dp[mask][v] = min(dp[mask][v], dp[sub][v] + dp[other][v]);
            }
            // 2. SPFA / Dijkstra 松弛
            priority_queue<pair<ll,int>, vector<pair<ll,int>>, greater<>> pq;
            for (int v = 1; v <= n; ++v) if (dp[mask][v] < LINF) pq.push({dp[mask][v], v});
            while (!pq.empty()) {
                auto [d, u] = pq.top(); pq.pop();
                if (d > dp[mask][u]) continue;
                for (auto &e : g[u]) if (dp[mask][e.to] > d + e.w) {
                    dp[mask][e.to] = d + e.w;
                    pq.push({dp[mask][e.to], e.to});
                }
            }
        }
        ll ans = LINF;
        for (int v = 1; v <= n; ++v) ans = min(ans, dp[full - 1][v]);
        return ans;
    }
};
```

**实例**：P6192【模板】最小斯坦纳树、P4294 [WC2008] 游览计划。

---

## 4. k 短路

**用途**：求 `s -> t` 的第 `k` 短路径（可重复经过点/边）。
**做法**：`A*`（用 `t` 的最短路做估价函数）或 **可持久化左偏树 + 最短路图**。

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;
constexpr ll LINF = 0x3f3f3f3f3f3f3f3fLL;

// A* 求第 k 短路（允许重复走边），复杂度 O(k m log n)
struct KthShortest {
    struct Edge { int to; ll w; };
    int n;
    vector<vector<Edge>> g, rg;

    explicit KthShortest(int n_) : n(n_), g(n_ + 1), rg(n_ + 1) {}
    void addEdge(int u, int v, ll w) { g[u].push_back({v, w}); rg[v].push_back({u, w}); }

    ll solve(int s, int t, int k) {
        // 1. 反向图跑 Dijkstra，得 h[v] = dist(v, t)
        vector<ll> h(n + 1, LINF);
        priority_queue<pair<ll,int>, vector<pair<ll,int>>, greater<>> pq;
        h[t] = 0; pq.push({0, t});
        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d > h[u]) continue;
            for (auto &e : rg[u]) if (h[e.to] > d + e.w) {
                h[e.to] = d + e.w;
                pq.push({h[e.to], e.to});
            }
        }
        if (h[s] == LINF) return -1;
        // 2. A*：f = g + h
        priority_queue<pair<ll,int>, vector<pair<ll,int>>, greater<>> q;
        q.push({h[s], s});
        vector<int> cnt(n + 1, 0);
        while (!q.empty()) {
            auto [f, u] = q.top(); q.pop();
            ll d = f - h[u];                       // 实际走过的距离
            if (++cnt[u] > k) continue;
            if (u == t && cnt[u] == k) return d;
            for (auto &e : g[u]) q.push({d + e.w + h[e.to], e.to});
        }
        return -1;
    }
};
```

**实例**：P4467 [SCOI2007] k短路、P2483【模板】k 短路 / [SDOI2010] 魔法猪学院。

---

## 5. LCT（Link-Cut Tree）

**用途**：动态树 —— 加边、删边、路径查询、动态换根。
**注意**：代码长、常数大，**只在明确"动态树"时用**。

```cpp
#include <bits/stdc++.h>
using namespace std;

// LCT：支持 link / cut / split(路径) / 路径信息维护
struct LCT {
    struct Node {
        int ch[2] = {0, 0}, fa = 0;
        int val = 0, mx = 0, mxId = 0;
        bool rev = false;
    };
    vector<Node> tr;

    explicit LCT(int n) : tr(n + 1) {}
    bool is_root(int x) {
        int f = tr[x].fa;
        return !f || (tr[f].ch[0] != x && tr[f].ch[1] != x);
    }
    void push_up(int x) {
        tr[x].mx = tr[x].val; tr[x].mxId = x;
        for (int c = 0; c < 2; ++c) {
            int y = tr[x].ch[c];
            if (y && tr[y].mx > tr[x].mx) { tr[x].mx = tr[y].mx; tr[x].mxId = tr[y].mxId; }
        }
    }
    void apply_rev(int x) {
        if (!x) return;
        swap(tr[x].ch[0], tr[x].ch[1]);
        tr[x].rev ^= 1;
    }
    void push_down(int x) {
        if (tr[x].rev) { apply_rev(tr[x].ch[0]); apply_rev(tr[x].ch[1]); tr[x].rev = false; }
    }
    void push_all(int x) {
        if (!is_root(x)) push_all(tr[x].fa);
        push_down(x);
    }
    void rotate(int x) {
        int y = tr[x].fa, z = tr[y].fa;
        int k = (tr[y].ch[1] == x);
        int b = tr[x].ch[k ^ 1];
        if (!is_root(y)) tr[z].ch[tr[z].ch[1] == y] = x;
        tr[x].fa = z;
        tr[x].ch[k ^ 1] = y; tr[y].fa = x;
        tr[y].ch[k] = b; if (b) tr[b].fa = y;
        push_up(y); push_up(x);
    }
    void splay(int x) {
        push_all(x);
        while (!is_root(x)) {
            int y = tr[x].fa, z = tr[y].fa;
            if (!is_root(y)) {
                if ((tr[y].ch[1] == x) == (tr[z].ch[1] == y)) rotate(y);
                else rotate(x);
            }
            rotate(x);
        }
    }
    void access(int x) {
        for (int y = 0; x; y = x, x = tr[x].fa) {
            splay(x);
            tr[x].ch[1] = y;
            push_up(x);
        }
    }
    void make_root(int x) { access(x); splay(x); apply_rev(x); }
    int find_root(int x) {
        access(x); splay(x);
        while (tr[x].ch[0]) { push_down(x); x = tr[x].ch[0]; }
        splay(x);
        return x;
    }
    void link(int x, int y) { make_root(x); if (find_root(y) != x) tr[x].fa = y; }
    void cut(int x, int y) {
        make_root(x); access(y); splay(y);
        if (tr[y].ch[0] == x && !tr[x].ch[1]) { tr[y].ch[0] = 0; tr[x].fa = 0; push_up(y); }
    }
    // 路径 x-y 的查询：先 split
    void split(int x, int y) { make_root(x); access(y); splay(y); }
    int query_max(int x, int y) { split(x, y); return tr[y].mx; }
};
```

**实例**：P3690【模板】动态树（Link Cut Tree）、P2147 [SDOI2008] 洞穴勘测、P3203 [HNOI2010] 弹飞绵羊。

---

## 6. 其他图论结论

| 问题 | 方法 |
|---|---|
| 二分图最大独立集 | `n - 最大匹配` |
| 二分图最小点覆盖 | `最大匹配` |
| 最小路径覆盖（DAG） | `n - 最大匹配` |
| 有向图最小环 | Floyd / 枚举边 + 最短路 |
| 无向图最小环 | Floyd（见 11 号文件） |
| 支配树 | Lengauer-Tarjan（极少数题） |
| 2-SAT | Tarjan SCC（见 12） |
| 强连通缩点 DAG | Tarjan（见 12） |
| 圆方树 | 点双缩点（见 12） |
| 欧拉路 | Hierholzer |
| 基环树 | 拓扑剥环 + 环上 DP |
| 斯坦纳树 | 状压 + 最短路 |
| k 短路 | A* |
| 动态树 | LCT |
| 最小割树 | Gomory-Hu（见 13 讨论） |

---

## 7. Gomory-Hu 最小割树（补充）

**用途**：`n` 个点的无向图，`q` 次询问两点间最小割。建树后 = 树上路径最小值。
**建法**：跑 `n-1` 次最大流，每次把点集分成两侧，递归建树。

```cpp
// 结构：
// 1. 取两个点 s, t，跑最大流得到割值 w，并得到 s 所在的点集 S
// 2. 在最小割树上连边 s-t，权值 w
// 3. 对 S 内部和 V\S 内部递归
// 4. 查询：树上路径最小值（倍增 / 树剖）
// 复杂度：O(n * maxflow)
```

**实例**：P4897【模板】最小割树（Gomory-Hu Tree）。


### 怎么用（照抄这段 `main`，只改【输入】【输出】两处）

上面是模板本体。实际写题时：**把上面的 struct 整段粘过来 → 再粘这段 `main` → 只改标了【输入】【输出】的行**。

```cpp skip
#include <bits/stdc++.h>
using namespace std;

// ===== 粘贴点：把上面的 EulerPath 粘到这里 =====

int main() {
    // ===================== 【输入】改这里 =====================
    int n, m;
    scanf("%d %d", &n, &m);               // 点数、边数
    // ========================================================

    // ===================== 【核心】算法流程 =====================
    // Hierholzer 求欧拉路：
    //   1) 从起点 DFS，每走到一条没用过的边就标记已用
    //   2) 走到死路时，把当前点"后序"加入答案，然后回溯
    //   3) 最后把答案 reverse，就是欧拉路
    // 判定：恰有 0 或 2 个奇度点（有 2 个时从奇点出发）
    EulerPath ep(n, m);
    for (int i = 1; i <= m; ++i) {
        int u, v;
        scanf("%d %d", &u, &v);
        ep.addEdge(i, u, v);
    }
    vector<int> path = ep.run(1);          // 空表示没有欧拉路
    // ========================================================

    // ===================== 【输出】改这里 =====================
    if (path.empty()) puts("NO");
    else {
        for (int i = 0; i < (int)path.size(); ++i)
            printf("%d%c", path[i], " \n"[i + 1 == (int)path.size()]);
    }
    // ========================================================
    return 0;
}
```

**基环树 / 斯坦纳树 / k 短路 / LCT** 的核心调用：

```cpp skip
BaseCycleTree bct(n);           // n 点 n 边，恰一个环
bct.addEdge(u, v);
vector<int> cyc = bct.find_cycle();   // 拓扑剥叶子，剩下的就是环

SteinerTree st(n, k);           // k 个关键点
st.key = {1, 5, 9};
st.addEdge(u, v, w);
ll cost = st.solve();

KthShortest ks(n);
ks.addEdge(u, v, w);
ll d = ks.solve(s, t, k);       // 第 k 短路

LCT lct(n);
lct.link(u, v); lct.cut(u, v);
int mx = lct.query_max(u, v);
```

---

## 8. 思路与例题索引

| 算法 | 思路一句话 | 用途 | 例题 |
|---|---|---|---|
| 欧拉路 (Hierholzer) | DFS 走到死路再回溯加边 | 一笔画 | P2731 骑马修栅栏、P7771【模板】欧拉路径 |
| 基环树找环 | 拓扑剥叶子，剩下的成环 | 基环树题 | P2607 骑士、P4381 Island |
| 基环树 DP | 环拆链 + 树上 DP | 环上选点 | P1453 城市环路 |
| 斯坦纳树 | 状压关键点 + 最短路松弛 | 连通指定点 | P6192【模板】最小斯坦纳树 |
| k 短路 | A*（`h` = 到终点的最短路） | 第 k 短路径 | P4467 k短路、P2483 |
| LCT | `access` + `splay` 维护路径 | 动态树 | P3690【模板】动态树 |
| Gomory-Hu | `n-1` 次最大流建树 | 任意两点最小割 | P4897【模板】最小割树 |

**选择**：路径计数/连通 → 欧拉路；一个环 + 树 → 基环树；动态加删边 → LCT。
