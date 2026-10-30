# 11 最短路 Dijkstra / SPFA / Floyd / Johnson

> 你点名的内容。给全：Dijkstra（含 set 版、堆版、多源、次短路、最短路计数）、SPFA（含 SLF/判负环）、Floyd（含传递闭包、最小环、路径还原）、Johnson 全源最短路、最短路径树。

---

## 1. Dijkstra（堆优化，标准版）★

**适用**：**边权非负**。`O(m log n)`。

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;
constexpr ll LINF = 0x3f3f3f3f3f3f3f3fLL;

struct Dijkstra {
    struct Edge { int to; ll w; };
    int n;
    vector<vector<Edge>> g;

    explicit Dijkstra(int n_) : n(n_), g(n_ + 1) {}
    void addEdge(int u, int v, ll w) { g[u].push_back({v, w}); }
    void addBiEdge(int u, int v, ll w) { addEdge(u, v, w); addEdge(v, u, w); }

    // 返回 dist（下标 1..n，不可达为 LINF）
    vector<ll> run(int s) const {
        vector<ll> dist(n + 1, LINF);
        priority_queue<pair<ll,int>, vector<pair<ll,int>>, greater<>> pq;
        dist[s] = 0;
        pq.push({0, s});
        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d > dist[u]) continue;              // 过期剪枝（关键）
            for (auto &e : g[u]) {
                if (dist[e.to] > d + e.w) {
                    dist[e.to] = d + e.w;
                    pq.push({dist[e.to], e.to});
                }
            }
        }
        return dist;
    }
};
```

**注意**：
- `if (d > dist[u]) continue;` **必须有**，否则会 TLE/MLE。
- 若用 `dist[u] == d` 判断，注意多组重边；用 `>` 更安全。
- 边权为 `int` 时可以把 `ll` 换成 `int` 提速。

---

## 2. Dijkstra 变体

### 2.1 求路径 + 前驱

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;

struct DijkstraPath {
    struct Edge { int to; ll w; };
    int n;
    vector<vector<Edge>> g;
    explicit DijkstraPath(int n_) : n(n_), g(n_ + 1) {}
    void addEdge(int u, int v, ll w) { g[u].push_back({v, w}); }

    // 返回 dist 和 pre；路径用 pre 回溯
    pair<vector<ll>, vector<int>> run(int s) const {
        const ll LINF = 0x3f3f3f3f3f3f3f3fLL;
        vector<ll> dist(n + 1, LINF);
        vector<int> pre(n + 1, 0);
        priority_queue<pair<ll,int>, vector<pair<ll,int>>, greater<>> pq;
        dist[s] = 0; pq.push({0, s});
        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d > dist[u]) continue;
            for (auto &e : g[u]) if (dist[e.to] > d + e.w) {
                dist[e.to] = d + e.w;
                pre[e.to] = u;
                pq.push({dist[e.to], e.to});
            }
        }
        return {dist, pre};
    }
    static vector<int> get_path(int t, const vector<int> &pre) {  // 得到 s..t 的点序列
        vector<int> path;
        for (int u = t; u; u = pre[u]) path.push_back(u);
        reverse(path.begin(), path.end());
        return path;
    }
};
```

### 2.2 最短路径计数

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;
constexpr ll MOD = 1000000007;

// 最短路条数（边权非负）。注意：相等时累加计数，严格小于时重置
struct DijkstraCount {
    struct Edge { int to; ll w; };
    int n;
    vector<vector<Edge>> g;
    explicit DijkstraCount(int n_) : n(n_), g(n_ + 1) {}
    void addEdge(int u, int v, ll w) { g[u].push_back({v, w}); }

    vector<ll> run(int s) const {                    // 返回方案数
        const ll LINF = 0x3f3f3f3f3f3f3f3fLL;
        vector<ll> dist(n + 1, LINF), cnt(n + 1, 0);
        priority_queue<pair<ll,int>, vector<pair<ll,int>>, greater<>> pq;
        dist[s] = 0; cnt[s] = 1; pq.push({0, s});
        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d > dist[u]) continue;
            for (auto &e : g[u]) {
                if (dist[e.to] > d + e.w) {
                    dist[e.to] = d + e.w;
                    cnt[e.to] = cnt[u];
                    pq.push({dist[e.to], e.to});
                } else if (dist[e.to] == d + e.w) {
                    cnt[e.to] = (cnt[e.to] + cnt[u]) % MOD;
                }
            }
        }
        return cnt;
    }
};
```

> ⚠️ 如果图里有 **0 权边**，`cnt` 的更新顺序可能有问题（同一个点被多次弹入）。安全做法：先跑一遍 Dijkstra 得 `dist`，再按 `dist` 升序 DP 计数。

### 2.3 次短路（`dist` 和 `dist2`）

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;

// 严格次短路（边可重复走）
struct SecondShortest {
    struct Edge { int to; ll w; };
    int n;
    vector<vector<Edge>> g;
    explicit SecondShortest(int n_) : n(n_), g(n_ + 1) {}
    void addEdge(int u, int v, ll w) { g[u].push_back({v, w}); g[v].push_back({u, w}); }

    ll run(int s, int t) const {
        const ll LINF = 0x3f3f3f3f3f3f3f3fLL;
        vector<array<ll,2>> d(n + 1, {LINF, LINF});
        priority_queue<pair<ll,int>, vector<pair<ll,int>>, greater<>> pq;
        d[s][0] = 0; pq.push({0, s});
        while (!pq.empty()) {
            auto [du, u] = pq.top(); pq.pop();
            if (du > d[u][1]) continue;
            for (auto &e : g[u]) {
                ll nd = du + e.w;
                if (nd < d[e.to][0]) {
                    d[e.to][1] = d[e.to][0];
                    d[e.to][0] = nd;
                    pq.push({nd, e.to});
                    pq.push({d[e.to][1], e.to});
                } else if (nd > d[e.to][0] && nd < d[e.to][1]) {
                    d[e.to][1] = nd;
                    pq.push({nd, e.to});
                }
            }
        }
        return d[t][1];
    }
};
```

### 2.4 多源最短路 / 0-1 BFS / 分层图

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;

// 多源：初始化时把所有源点 dist 置 0 入堆即可
// 0-1 BFS：边权只有 0/1 时用 deque，O(n+m)
struct ZeroOneBFS {
    struct Edge { int to, w; };
    int n;
    vector<vector<Edge>> g;
    explicit ZeroOneBFS(int n_) : n(n_), g(n_ + 1) {}
    void addEdge(int u, int v, int w) { g[u].push_back({v, w}); }

    vector<int> run(int s, const vector<int> &sources = {}) const {
        vector<int> d(n + 1, INT_MAX);
        deque<int> dq;
        for (int x : sources) { d[x] = 0; dq.push_back(x); }
        if (sources.empty()) { d[s] = 0; dq.push_back(s); }
        while (!dq.empty()) {
            int u = dq.front(); dq.pop_front();
            for (auto &e : g[u]) if (d[e.to] > d[u] + e.w) {
                d[e.to] = d[u] + e.w;
                if (e.w == 0) dq.push_front(e.to);
                else dq.push_back(e.to);
            }
        }
        return d;
    }
};

// 分层图：把"使用 k 次特殊操作"建成 k+1 层，第 j 层的 u 编号为 u + j*n
// 普通边: u -> v (w)           => (u + j*n) -> (v + j*n)
// 特殊边: u -> v (0)           => (u + j*n) -> (v + (j+1)*n)
```

---

## 3. SPFA（可判负环）

**适用**：有负权边、无负环；或判负环。最坏 `O(nm)`，**容易被卡**，能不用就不用。

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;
constexpr ll LINF = 0x3f3f3f3f3f3f3f3fLL;

struct SPFA {
    struct Edge { int to; ll w; };
    int n;
    vector<vector<Edge>> g;
    explicit SPFA(int n_) : n(n_), g(n_ + 1) {}
    void addEdge(int u, int v, ll w) { g[u].push_back({v, w}); }

    // 返回 {dist, has_negative_cycle}
    pair<vector<ll>, bool> run(int s) const {
        vector<ll> dist(n + 1, LINF);
        vector<int> cnt(n + 1, 0);              // 入队次数
        vector<bool> inq(n + 1, false);
        queue<int> q;
        dist[s] = 0; q.push(s); inq[s] = true;
        while (!q.empty()) {
            int u = q.front(); q.pop(); inq[u] = false;
            for (auto &e : g[u]) if (dist[e.to] > dist[u] + e.w) {
                dist[e.to] = dist[u] + e.w;
                if (!inq[e.to]) {
                    q.push(e.to); inq[e.to] = true;
                    if (++cnt[e.to] >= n) return {dist, true};   // 负环
                }
            }
        }
        return {dist, false};
    }
};
```

**判负环的三种写法**：
1. 入队次数 `>= n`：某个点被松弛超过 n 次 ⇒ 有负环。
2. 最短路经过的边数 `>= n`：更准（用 `d_cnt[]` 维护）。
3. **所有点入队**跑一次：处理"图不连通、负环在别的连通块"。

```cpp skip
// 依赖上面的 SPFA 结构，片段，单独编译不通过
// 全局负环检测：把所有点初始入队，dist = 0
bool has_negative_cycle_global(const SPFA &s) {
    int n = s.n;
    vector<ll> dist(n + 1, 0);
    vector<int> cnt(n + 1, 0);
    vector<bool> inq(n + 1, true);
    queue<int> q;
    for (int i = 1; i <= n; ++i) q.push(i);
    while (!q.empty()) {
        int u = q.front(); q.pop(); inq[u] = false;
        for (auto &e : s.g[u]) if (dist[e.to] > dist[u] + e.w) {
            dist[e.to] = dist[u] + e.w;
            if (!inq[e.to]) {
                q.push(e.to); inq[e.to] = true;
                if (++cnt[e.to] >= n + 1) return true;
            }
        }
    }
    return false;
}
```

**SPFA 优化（比赛可加，但别指望救 TLE）**：SLF（队首比较 `dist`，小的插队首）、LLL、DFS 版判负环。

---

## 4. Floyd（全源最短路 / 传递闭包）

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;
constexpr ll LINF = 0x3f3f3f3f3f3f3f3fLL;

struct Floyd {
    int n;
    vector<vector<ll>> d;
    explicit Floyd(int n_) : n(n_), d(n_ + 1, vector<ll>(n_ + 1, LINF)) {
        for (int i = 1; i <= n; ++i) d[i][i] = 0;
    }
    void addEdge(int u, int v, ll w) { d[u][v] = min(d[u][v], w); }

    void run() {
        for (int k = 1; k <= n; ++k)
            for (int i = 1; i <= n; ++i) {
                if (d[i][k] == LINF) continue;
                for (int j = 1; j <= n; ++j)
                    if (d[i][j] > d[i][k] + d[k][j])
                        d[i][j] = d[i][k] + d[k][j];
            }
    }
    bool has_negative_cycle() const {
        for (int i = 1; i <= n; ++i) if (d[i][i] < 0) return true;
        return false;
    }
};
```

**注意**：
- `k` 必须在最外层。
- `d[i][k] == LINF` 剪枝能省一半时间。
- `n <= 400` 时 `O(n^3)` 可过；`n = 500` 时约 `1.25e8`，需要卡常。

### 4.1 传递闭包（可达性）

```cpp
#include <bits/stdc++.h>
using namespace std;

// bitset 优化：n <= 5000 也能过
vector<bitset<5005>> transitive_closure(int n, vector<pair<int,int>> edges) {
    vector<bitset<5005>> reach(n + 1);
    for (auto [u, v] : edges) reach[u][v] = 1;
    for (int i = 1; i <= n; ++i) reach[i][i] = 1;
    for (int k = 1; k <= n; ++k)
        for (int i = 1; i <= n; ++i)
            if (reach[i][k]) reach[i] |= reach[k];
    return reach;
}
```

### 4.2 最小环（无向图）

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;

// Floyd 求无向图最小环：在跑 Floyd 的过程中，用 d[i][j] + w[i][k] + w[k][j] 更新
// 注意必须用"还没被 k 更新过"的 d
ll min_cycle(int n, vector<vector<ll>> w) {           // w[i][j] 为边权，无边的 INF
    const ll INF = 0x3f3f3f3f3f3f3f3fLL;
    auto d = w;
    ll ans = INF;
    for (int k = 1; k <= n; ++k) {
        for (int i = 1; i < k; ++i)
            for (int j = i + 1; j < k; ++j)
                if (d[i][j] < INF && w[i][k] < INF && w[k][j] < INF)
                    ans = min(ans, d[i][j] + w[i][k] + w[k][j]);
        for (int i = 1; i <= n; ++i)
            for (int j = 1; j <= n; ++j)
                if (d[i][k] + d[k][j] < d[i][j]) d[i][j] = d[i][k] + d[k][j];
    }
    return ans;
}
```

---

## 5. Johnson 全源最短路（稀疏图 / 有负权）

**思路**：加超级源点 `0`，向所有点连 `w=0` 的边，SPFA 求势能 `h[v]`；然后把边权改成 `w + h[u] - h[v] >= 0`，对每个点跑 Dijkstra。`O(nm log n)`。

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;
constexpr ll LINF = 0x3f3f3f3f3f3f3f3fLL;

struct Johnson {
    struct Edge { int to; ll w; };
    int n;
    vector<vector<Edge>> g;
    explicit Johnson(int n_) : n(n_), g(n_ + 1) {}
    void addEdge(int u, int v, ll w) { g[u].push_back({v, w}); }

    // 返回 n+1 x n+1 的 dist；有负环返回空
    vector<vector<ll>> run() {
        // 1) 超级源点 SPFA 求势能 h
        vector<ll> h(n + 1, 0);
        vector<int> cnt(n + 1, 0);
        vector<bool> inq(n + 1, true);
        queue<int> q;
        for (int i = 1; i <= n; ++i) q.push(i);
        while (!q.empty()) {
            int u = q.front(); q.pop(); inq[u] = false;
            for (auto &e : g[u]) if (h[e.to] > h[u] + e.w) {
                h[e.to] = h[u] + e.w;
                if (!inq[e.to]) {
                    q.push(e.to); inq[e.to] = true;
                    if (++cnt[e.to] > n) return {};      // 负环
                }
            }
        }
        // 2) 重赋权 + n 次 Dijkstra
        vector<vector<ll>> dist(n + 1, vector<ll>(n + 1, LINF));
        for (int s = 1; s <= n; ++s) {
            priority_queue<pair<ll,int>, vector<pair<ll,int>>, greater<>> pq;
            dist[s][s] = 0; pq.push({0, s});
            while (!pq.empty()) {
                auto [d, u] = pq.top(); pq.pop();
                if (d > dist[s][u]) continue;
                for (auto &e : g[u]) {
                    ll w2 = e.w + h[u] - h[e.to];        // >= 0
                    if (dist[s][e.to] > d + w2) {
                        dist[s][e.to] = d + w2;
                        pq.push({dist[s][e.to], e.to});
                    }
                }
            }
        }
        // 3) 还原真实距离
        for (int s = 1; s <= n; ++s)
            for (int t = 1; t <= n; ++t)
                if (dist[s][t] < LINF) dist[s][t] += h[t] - h[s];
        return dist;
    }
};
```

---

## 6. 最短路树（SPT）与次短路应用

- **最短路树**：每个点取一条最短路径的前驱，构成一棵树（可能有多个，任取）。
- **求 `1 -> n` 的所有最短路可能经过的边**：正反跑两次 Dijkstra，若 `d1[u] + w + dn[v] == d1[n]` 则边 `(u,v)` 在某条最短路上。
- **最短路 DAG 上的 DP**：把满足上式的边建成 DAG，按 `d1` 排序后 DP（计数、最长路等）。

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;

// 判断边 (u,v,w) 是否在某条 1->n 的最短路上
// 需要 d1 = dist(1, *), dn = dist(n, *)
```

---

## 7. 选择建议

| 场景 | 算法 | 复杂度 |
|---|---|---|
| 单源、非负权 | **Dijkstra + 堆** | `O(m log n)` |
| 单源、0/1 权 | 0-1 BFS | `O(n+m)` |
| 单源、有负权无负环 | SPFA | 最坏 `O(nm)` |
| 单源、负权、要稳 | Bellman-Ford | `O(nm)` |
| 全源、稠密 `n<=400` | Floyd | `O(n^3)` |
| 全源、稀疏、有负权 | Johnson | `O(nm log n)` |
| 只判负环 | SPFA 入队 `>= n` | — |
| 网格图非负权 | Dijkstra（注意堆常数） | — |
| 边权只有 1 | BFS | `O(n+m)` |
| 次短路 | Dijkstra 维护两个距离 | `O(m log n)` |

**常见坑**：
- 加边用 `int` 还是 `ll`：边权和答案一旦可能 `> 2^31` 就必须 `ll`。
- 多组数据记得**清空 `g`**。
- 稀疏图用 `vector<vector<Edge>>`，极密或卡常时用**链式前向星**。
- `priority_queue` 里存 `pair<ll,int>`，`greater<>` 直接可用（`pair` 自带比较）。

---

## 8. 思路与例题索引

| 算法 | 思路一句话 | 用途 | 例题 |
|---|---|---|---|
| Dijkstra | 从已确定最短的点扩展，堆取最小 | 非负权单源 | P4779【模板】单源最短路 |
| 最短路计数 | 相等时累加，严格小则重置 | 路径方案数 | P1608 路径统计 |
| 次短路 | 维护两个 dist，同时更新 | 次短路 | P2865 [USACO06NOV] Roadblocks |
| 0-1 BFS | 边权只有 0/1，用 deque 头插尾插 | 网格、开关 | P4667 [BalticOI2011] 电子眼 |
| 分层图 | 第 j 层表示用了 j 次特殊操作 | 有 k 次免费边 | P4568 [JLOI2011] 飞行路线 |
| SPFA 判负环 | 入队次数 >= n | 负环 | P3385【模板】负环 |
| Floyd | `k` 在最外层，`d[i][k]==INF` 剪枝 | 全源最短路 | P2910 [USACO08OPEN] Clear And Present Danger |
| Floyd 传递闭包 | bitset 按行或 | 可达性 | P2419 [USACO08JAN] Cow Contest |
| Floyd 最小环 | 用"未被 k 更新"的 d 更新答案 | 无向图最小环 | P6175 无向图的最小环 |
| Johnson | 势能重赋权 + n 次 Dijkstra | 稀疏全源 + 负权 | P5905【模板】Johnson 全源最短路 |

**判断**：非负 → Dijkstra；有负权 → SPFA/Johnson；`n<=400` → Floyd。
