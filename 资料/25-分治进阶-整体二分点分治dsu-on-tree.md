# 25 分治进阶（整体二分 / 点分治 / dsu on tree / 长链剖分）

> 这四类分治"看着难，套路固定"，是全套资料里**含金量最高**的部分之一。
> 每个都给「**用途 → 思路 → 模板 → 实例**」。

---

## 1. 整体二分

### 1.1 用途

把"**多次二分答案**"合并成一次分治。适用于：

- 多次询问"第 k 小"，答案可二分，且 `check` 能批量做。
- **带修改**的区间第 k 小（P2617 Dynamic Rankings）。
- 区间第 k 小、矩形第 k 小、树上第 k 小（可离线）。

### 1.2 思路

`solve(l, r, 询问集合)`：答案在 `[l, r]` 中。
取 `mid`，用树状数组模拟"值在 `[l, mid]` 的元素"，对每个询问统计其区间内有多少个，决定分到左半还是右半。

```cpp
#include <bits/stdc++.h>
using namespace std;

// 整体二分：静态区间第 k 小
// 事件类型 1: 加入一个数；2: 询问
struct ParallelBinarySearch {
    struct Event {
        int type;          // 1 = 加点, 2 = 询问
        int l, r, k, id;   // 加点：l = 位置；询问：l,r 区间, k 第 k 小
    };
    int n, m;
    vector<Event> ev;
    vector<int> ans;
    vector<int> bit;
    int maxVal;

    ParallelBinarySearch(int n_, int m_, int maxVal_) : n(n_), m(m_), maxVal(maxVal_) {
        ans.assign(m_, 0);
        bit.assign(n_ + 2, 0);
    }

    void add(int i, int v) { for (; i <= n; i += i & -i) bit[i] += v; }
    int sum(int i) const { int s = 0; for (; i > 0; i -= i & -i) s += bit[i]; return s; }

    void solve(int l, int r, vector<Event> &cur) {
        if (cur.empty()) return;
        if (l == r) {
            for (auto &e : cur) if (e.type == 2) ans[e.id] = l;
            return;
        }
        int mid = (l + r) >> 1;
        vector<Event> left, right;
        vector<int> touched;
        for (auto &e : cur) {
            if (e.type == 1) {
                if (e.k <= mid) { add(e.l, 1); touched.push_back(e.l); left.push_back(e); }
                else right.push_back(e);
            } else {
                int cnt = sum(e.r) - sum(e.l - 1);
                if (cnt >= e.k) left.push_back(e);
                else { e.k -= cnt; right.push_back(e); }
            }
        }
        for (int i : touched) add(i, -1);
        solve(l, mid, left);
        solve(mid + 1, r, right);
    }

    // 建事件：加点 (type=1, l=位置, k=值)；询问 (type=2, l, r, k, id)
    vector<int> run() {
        solve(1, maxVal, ev);
        return ans;
    }
};
```

> **注意**：递归前必须把 BIT 清空（上面用 `touched` 撤销）。`maxVal` 是值域上界。

**实例（P2617 动态区间第 k 小）**：把"修改"拆成"删旧值 + 加新值"两个事件，按时间顺序排好，整体二分时按时间处理即可。

---

## 2. 点分治

### 2.1 用途

统计**树上路径**满足某条件的对数 / 条数。典型题：

- P3806【模板】点分治 1（路径长度恰为 `k`）。
- P4178 Tree（路径长度 `<= k`）。
- P2634 [国家集训队] 聪聪可可（路径长度 `% 3 == 0`）。
- P4149 [IOI2011] Race（路径长度为 `k` 的最小边数）。

### 2.2 思路

每次找到**当前树的重心**，统计"经过重心的路径"，然后递归处理各子树。
复杂度 `O(n log n)`。

```cpp
#include <bits/stdc++.h>
using namespace std;

// 点分治模板：统计路径长度恰好为 K 的点对个数
struct CentroidDecomp {
    struct Edge { int to, w; };
    int n, K;
    vector<vector<Edge>> g;
    vector<int> sz, mx;
    vector<bool> removed;
    bool found = false;                    // 本题只问是否存在

    explicit CentroidDecomp(int n_, int K_) : n(n_), K(K_), g(n_ + 1),
        sz(n_ + 1), mx(n_ + 1), removed(n_ + 1, false) {}

    void addEdge(int u, int v, int w) { g[u].push_back({v, w}); g[v].push_back({u, w}); }

    // ---- 求重心 ----
    int get_size(int u, int p) {
        sz[u] = 1;
        for (auto [v, w] : g[u]) if (v != p && !removed[v]) sz[u] += get_size(v, u);
        return sz[u];
    }
    int get_centroid(int u, int p, int total) {
        for (auto [v, w] : g[u])
            if (v != p && !removed[v] && sz[v] > total / 2) return get_centroid(v, u, total);
        return u;
    }

    // ---- 统计经过重心的路径 ----
    vector<int> dists;
    void collect(int u, int p, int d) {
        if (d > K) return;
        dists.push_back(d);
        for (auto [v, w] : g[u]) if (v != p && !removed[v]) collect(v, u, d + w);
    }

    // 统计"经过 u 的路径"中长度恰为 K 的对数
    int count_paths(int u) {
        dists.clear();
        dists.push_back(0);
        for (auto [v, w] : g[u]) if (!removed[v]) {
            vector<int> sub;
            int before = (int)dists.size();
            collect(v, u, w);
            // 新加入的点与已有集合配对
            vector<int> added(dists.begin() + before, dists.end());
            sort(added.begin(), added.end());
            for (int i = 0; i < before; ++i) {
                int need = K - dists[i];
                if (need < 0) continue;
                if (binary_search(added.begin(), added.end(), need)) found = true;
            }
        }
        return 0;
    }

    void decompose(int u) {
        get_size(u, 0);
        int c = get_centroid(u, 0, sz[u]);
        removed[c] = true;
        count_paths(c);
        for (auto [v, w] : g[c]) if (!removed[v]) decompose(v);
        removed[c] = false;                // 视题目要求，可保留
    }
};
```

**关键点**：
1. `removed` 标记重心，递归子树。
2. 每次统计"经过当前重心"的路径，**不重复**。
3. 用**容斥**去掉同一子树内部的路径（上面是简化版）。
4. 求"恰好 K"时，用 `vector` 排序 + 二分 / 哈希表。

### 2.3 点分治 + 容斥（标准写法）

```cpp
// 标准结构：
// 1. 求重心 c
// 2. 统计所有经过 c 的路径（含同一子树内的）
// 3. 减去每个子树内部的路径（容斥）
// 4. 递归各子树
int solve_paths(int c, int K) {
    int ans = 0;
    // 统计所有点对（含子树内部）
    // 对每个子树：减去子树内部点对
    return ans;
}
```

---

## 3. dsu on tree（树上启发式合并）

### 3.1 用途

统计**每个子树**的某种"众数/颜色数/出现次数"信息。典型题：

- CF 600E Lomsat gelral（子树众数颜色和）。
- P4103 [HEOI2014] 大工程（部分子问题）。
- P1192 类型。

### 3.2 思路

DFS 时**保留重儿子的贡献**，轻儿子暴力重算。每个点只被合并 `O(log n)` 次。
```cpp
#include <bits/stdc++.h>
using namespace std;

// dsu on tree 标准模板（子树众数）
struct DSUOnTree {
    int n;
    vector<vector<int>> g;
    vector<int> col, sz, heavy, cnt;
    vector<long long> ans;
    int maxCnt = 0;
    long long sumAns = 0;

    DSUOnTree(int n_, int maxColor) : n(n_), g(n_ + 1), col(n_ + 1), sz(n_ + 1),
        heavy(n_ + 1, 0), cnt(maxColor + 1, 0), ans(n_ + 1) {}

    void addEdge(int u, int v) { g[u].push_back(v); g[v].push_back(u); }

    void dfs_size(int u, int p) {
        sz[u] = 1;
        for (int v : g[u]) if (v != p) {
            dfs_size(v, u);
            sz[u] += sz[v];
            if (!heavy[u] || sz[v] > sz[heavy[u]]) heavy[u] = v;
        }
    }

    void insert(int u, int p, int delta) {
        int c = col[u];
        cnt[c] += delta;
        if (delta > 0) {
            if (cnt[c] > maxCnt) { maxCnt = cnt[c]; sumAns = c; }
            else if (cnt[c] == maxCnt) sumAns += c;
        }
        for (int v : g[u]) if (v != p) insert(v, u, delta);
    }

    void dfs(int u, int p, bool keepFlag) {
        for (int v : g[u]) if (v != p && v != heavy[u]) dfs(v, u, false);
        if (heavy[u]) dfs(heavy[u], u, true);

        // 把轻儿子和自身加入
        for (int v : g[u]) if (v != p && v != heavy[u]) insert(v, u, 1);
        {
            int c = col[u];
            ++cnt[c];
            if (cnt[c] > maxCnt) { maxCnt = cnt[c]; sumAns = c; }
            else if (cnt[c] == maxCnt) sumAns += c;
        }
        ans[u] = sumAns;

        if (!keepFlag) {
            insert(u, p, -1);
            maxCnt = 0;
            sumAns = 0;
        }
    }

    vector<long long> run(int root = 1) {
        dfs_size(root, 0);
        dfs(root, 0, false);
        return ans;
    }
};
```

**复杂度**：`O(n log n)`（每个点被加入 `O(log n)` 次）。

---

## 4. 长链剖分（优化"与深度有关"的 DP）

### 4.1 用途

`f[u][d]` 型 DP（与深度相关），用**长链剖分 + 指针共用内存**把 `O(n^2)` 降到 `O(n)`。
典型题：CF 1009F Dominant Indices、P3565 [POI2014] HOT-Hotels。

### 4.2 思路

每个点选**深度最大的儿子**作为重儿子；重儿子的 DP 数组**平移一位**直接复用，避免 `O(深度)` 的复制。


```cpp
#include <bits/stdc++.h>
using namespace std;

// 长链剖分：CF 1009F
struct LongChainDecomp {
    int n;
    vector<vector<int>> g;
    vector<int> dep, son, ans;
    vector<int> buf;
    vector<int*> f;

    explicit LongChainDecomp(int n_) : n(n_), g(n_ + 1), dep(n_ + 1), son(n_ + 1, 0),
        ans(n_ + 1), buf(n_ + 2), f(n_ + 1, nullptr) {}

    void addEdge(int u, int v) { g[u].push_back(v); g[v].push_back(u); }

    void dfs1(int u, int p) {
        dep[u] = 1;
        for (int v : g[u]) if (v != p) {
            dfs1(v, u);
            if (dep[v] + 1 > dep[u]) { dep[u] = dep[v] + 1; son[u] = v; }
        }
    }

    int pool_ptr = 0;
    void dfs2(int u, int p) {
        f[u][0] = 1;
        if (son[u]) {
            f[son[u]] = f[u] + 1;           // 复用
            dfs2(son[u], u);
            ans[u] = ans[son[u]] + 1;
        }
        for (int v : g[u]) if (v != p && v != son[u]) {
            f[v] = buf.data() + pool_ptr;
            pool_ptr += dep[v];
            dfs2(v, u);
            for (int j = 1; j <= dep[v]; ++j) {
                f[u][j] += f[v][j - 1];
                if (f[u][j] > f[u][ans[u]] ||
                   (f[u][j] == f[u][ans[u]] && j < ans[u])) ans[u] = j;
            }
        }
        if (f[u][ans[u]] == 1) ans[u] = 0;
    }

    vector<int> run(int root = 1) {
        dfs1(root, 0);
        f[root] = buf.data();
        pool_ptr = dep[root];
        dfs2(root, 0);
        return ans;
    }
};
```

**复杂度**：`O(n)`（每条长链的总长 `O(n)`）。

---

## 5. 选型速查

| 问题 | 方法 | 复杂度 |
|---|---|---|
| 多次询问第 k 小 | 整体二分 / 主席树 | `O((n+m) log n)` |
| 树上路径计数 | 点分治 | `O(n log n)` |
| 子树信息统计 | dsu on tree | `O(n log n)` |
| 深度相关树上 DP | 长链剖分 | `O(n)` |
| 带修区间第 k 小 | 整体二分 / 树套树 | `O(n log^2 n)` |
| 路径最值 | 树剖 / 倍增 | `O(n log n)` |

**通用提醒**：
1. 点分治要**容斥**，别把同一子树的路径重复算。
2. dsu on tree 的 `insert` 带 `delta` 参数，撤销时 `delta = -1`。
3. 长链剖分的 `f[u]` 是指针，**内存池要够**（`O(n)`）。
4. 整体二分的 BIT 每次要清空。


### 怎么用（照抄这段 `main`，只改【输入】【输出】两处）

上面是模板本体。实际写题时：**把上面的 struct 整段粘过来 → 再粘这段 `main` → 只改标了【输入】【输出】的行**。

```cpp skip
#include <bits/stdc++.h>
using namespace std;

// ===== 粘贴点：把上面的 ParallelBinarySearch 粘到这里 =====

int main() {
    // ===================== 【输入】改这里 =====================
    int n, m, maxVal;
    scanf("%d %d %d", &n, &m, &maxVal);   // n 个数，m 次询问，值域上界
    vector<int> a(n + 1);
    for (int i = 1; i <= n; ++i) scanf("%d", &a[i]);
    // ========================================================

    // ===================== 【核心】算法流程 =====================
    // 整体二分：把所有询问一次性分治
    //   solve(l, r, 询问集合) 表示这些询问的答案落在 [l, r]
    //   取 mid，用树状数组模拟"值 <= mid 的数都在数组里"
    //   对每个询问查它区间内有多少个，>= k 就分到左半，否则 k 减掉分到右半
    //   时间复杂度 O((n + m) log n log V)
    ParallelBinarySearch pbs(n, m, maxVal);
    for (int i = 1; i <= n; ++i)
        pbs.ev.push_back({1, i, 0, a[i], 0});      // 加点事件
    for (int i = 0; i < m; ++i) {
        int l, r, k;
        scanf("%d %d %d", &l, &r, &k);
        pbs.ev.push_back({2, l, r, k, i});         // 询问事件
    }
    vector<int> ans = pbs.run();
    // ========================================================

    // ===================== 【输出】改这里 =====================
    for (int i = 0; i < m; ++i) printf("%d\n", ans[i]);
    // ========================================================
    return 0;
}
```

**点分治 / dsu on tree / 长链剖分** 的核心调用：

```cpp skip
// 点分治：统计树上路径长度恰为 K 的点对
CentroidDecomp cd(n, K);
cd.addEdge(u, v, w);
cd.decompose(1);              // 结果在 cd.found / cd 内部统计

// dsu on tree：子树众数
DSUOnTree d(n, maxColor);
d.addEdge(u, v);
d.col[u] = c;
auto res = d.run(1);          // res[u] = 子树 u 的答案

// 长链剖分：与深度相关的树上 DP
LongChainDecomp lc(n);
lc.addEdge(u, v);
auto res2 = lc.run(1);
```

---

## 6. 思路与例题索引

| 算法 | 思路一句话 | 用途 | 例题 |
|---|---|---|---|
| 整体二分 | 一次分治处理所有询问的二分 | 多次第 k 小、带修 | P2617 Dynamic Rankings、P1527 矩阵乘法 |
| 点分治 | 每次找重心，统计经过重心的路径 | 树上路径计数 | P3806【模板】点分治 1、P4178 Tree |
| 点分治 + 容斥 | 减去同一子树内部 | 路径长度 = k 的对数 | P2634 聪聪可可、P4149 Race |
| dsu on tree | 保留重儿子，轻儿子暴力 | 子树众数/颜色数 | CF 600E、P3899 [湖南集训] 更为厉害 |
| 长链剖分 | 重儿子数组复用，指针平移 | 深度相关 DP | CF 1009F、P3565 HOT-Hotels |

**判断**：多次询问第 k 小 → 整体二分；树上路径统计 → 点分治；子树信息 → dsu on tree。
