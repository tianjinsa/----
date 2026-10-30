# 23 平衡树（FHQ Treap / Splay）

> **只保留 FHQ Treap 和 Splay**。红黑树、AVL、跳表不收录：`map/set` 能替代，且比赛几乎不手写。
> 平衡树在比赛里的真实用途：**区间翻转（文艺平衡树）**、**动态区间操作**、**排名/前驱后继带插入删除**、**可持久化**。
> **如果 `set/map` 能干，就别写平衡树**。

---

## 1. 什么时候必须用平衡树

| 需求 | 用 `set/map` 够吗 | 用平衡树 |
|---|---|---|
| 插入/删除/查排名/前驱后继 | ✅ `set` | 不必 |
| 第 k 小（带删除） | ❌ | FHQ / Splay |
| **区间翻转**（文艺平衡树） | ❌ | FHQ / Splay |
| 区间加、区间求和 + 分裂合并 | ❌ | FHQ |
| 可持久化平衡树 | ❌ | 可持久化 FHQ |
| 维护数列（P3391、P1503） | ❌ | FHQ / Splay |

**结论**：遇到"区间翻转 / 分裂合并 / 动态第 k 小 + 修改"才写平衡树。

---

## 2. FHQ Treap（无旋 Treap）★ 推荐

**原理**：每个节点有 `(val, pri)`。按 `val` 满足 BST，按 `pri` 满足小根堆。用 **split（按值或按大小分裂）** 和 **merge** 代替旋转。

### 2.1 普通平衡树（P3369）

```cpp
#include <bits/stdc++.h>
using namespace std;

// FHQ Treap：插入、删除、排名、第 k 小、前驱、后继
struct FHQ {
    struct Node {
        int l = 0, r = 0, val = 0, pri = 0, sz = 1;
    };
    vector<Node> tr;
    int root = 0;
    mt19937 rng{chrono::steady_clock::now().time_since_epoch().count()};

    FHQ(int maxNodes = 200005) {
        tr.reserve(maxNodes);
        tr.push_back(Node{});                  // 0 号为空节点
    }
    int newNode(int v) {
        tr.push_back(Node{});
        int p = (int)tr.size() - 1;
        tr[p].val = v;
        tr[p].pri = (int)(rng() & 0x7fffffff);
        tr[p].sz = 1;
        return p;
    }
    void push_up(int p) { tr[p].sz = tr[tr[p].l].sz + tr[tr[p].r].sz + 1; }

    // 按值分裂：左树全部 <= v，右树全部 > v
    void split_val(int p, int v, int &a, int &b) {
        if (!p) { a = b = 0; return; }
        if (tr[p].val <= v) { a = p; split_val(tr[p].r, v, tr[p].r, b); }
        else { b = p; split_val(tr[p].l, v, a, tr[p].l); }
        push_up(p);
    }
    // 按大小分裂：左树恰好 k 个
    void split_sz(int p, int k, int &a, int &b) {
        if (!p) { a = b = 0; return; }
        int lsz = tr[tr[p].l].sz;
        if (lsz < k) { a = p; split_sz(tr[p].r, k - lsz - 1, tr[p].r, b); }
        else { b = p; split_sz(tr[p].l, k, a, tr[p].l); }
        push_up(p);
    }
    // 合并：a 中所有值 <= b 中所有值
    int merge(int a, int b) {
        if (!a || !b) return a + b;
        if (tr[a].pri < tr[b].pri) { tr[a].r = merge(tr[a].r, b); push_up(a); return a; }
        tr[b].l = merge(a, tr[b].l); push_up(b); return b;
    }

    void insert(int v) {
        int a, b;
        split_val(root, v, a, b);
        root = merge(merge(a, newNode(v)), b);
    }
    void erase(int v) {                        // 删除一个 v
        int a, b, c;
        split_val(root, v - 1, a, b);
        split_val(b, v, b, c);
        if (b) b = merge(tr[b].l, tr[b].r);    // 丢掉根
        root = merge(merge(a, b), c);
    }
    int rank_of(int v) {                       // v 的排名（比 v 小的个数 + 1）
        int a, b;
        split_val(root, v - 1, a, b);
        int ans = tr[a].sz + 1;
        root = merge(a, b);
        return ans;
    }
    int kth(int k) {                           // 第 k 小
        int p = root;
        while (p) {
            int lsz = tr[tr[p].l].sz;
            if (k == lsz + 1) return tr[p].val;
            if (k <= lsz) p = tr[p].l;
            else { k -= lsz + 1; p = tr[p].r; }
        }
        return -1;
    }
    int prev_less(int v) {                     // 严格小于 v 的最大值
        int a, b;
        split_val(root, v - 1, a, b);
        int p = a;
        while (p && tr[p].r) p = tr[p].r;
        int ans = p ? tr[p].val : -1;
        root = merge(a, b);
        return ans;
    }
    int next_greater(int v) {                  // 严格大于 v 的最小值
        int a, b;
        split_val(root, v, a, b);
        int p = b;
        while (p && tr[p].l) p = tr[p].l;
        int ans = p ? tr[p].val : -1;
        root = merge(a, b);
        return ans;
    }
};
```

**复杂度**：期望 `O(log n)`（随机优先级）。

### 2.2 FHQ 维护区间（文艺平衡树 / 区间翻转）★

> **这是 FHQ 最不可替代的用途**：把序列按下标挂到平衡树上，`split` 出区间，打**翻转标记**，最后中序遍历输出。

**实例：P3391【模板】文艺平衡树** —— `m` 次翻转区间 `[l, r]`。

```cpp
#include <bits/stdc++.h>
using namespace std;

struct FHQSequence {
    struct Node {
        int l = 0, r = 0, val = 0, pri = 0, sz = 1;
        bool rev = false;                       // 翻转标记
    };
    vector<Node> tr;
    int root = 0;
    mt19937 rng{chrono::steady_clock::now().time_since_epoch().count()};

    FHQSequence(int maxNodes = 200005) {
        tr.reserve(maxNodes);
        tr.push_back(Node{});
    }
    int newNode(int v) {
        tr.push_back(Node{});
        int p = (int)tr.size() - 1;
        tr[p].val = v;
        tr[p].pri = (int)(rng() & 0x7fffffff);
        return p;
    }
    void push_up(int p) { tr[p].sz = tr[tr[p].l].sz + tr[tr[p].r].sz + 1; }
    void apply_rev(int p) {
        if (!p) return;
        swap(tr[p].l, tr[p].r);
        tr[p].rev ^= 1;
    }
    void push_down(int p) {
        if (!tr[p].rev) return;
        apply_rev(tr[p].l);
        apply_rev(tr[p].r);
        tr[p].rev = false;
    }

    void split_sz(int p, int k, int &a, int &b) {   // 前 k 个分到 a
        if (!p) { a = b = 0; return; }
        push_down(p);
        int lsz = tr[tr[p].l].sz;
        if (lsz < k) { a = p; split_sz(tr[p].r, k - lsz - 1, tr[p].r, b); }
        else { b = p; split_sz(tr[p].l, k, a, tr[p].l); }
        push_up(p);
    }
    int merge(int a, int b) {
        if (!a || !b) return a + b;
        if (tr[a].pri < tr[b].pri) {
            push_down(a);
            tr[a].r = merge(tr[a].r, b);
            push_up(a);
            return a;
        }
        push_down(b);
        tr[b].l = merge(a, tr[b].l);
        push_up(b);
        return b;
    }

    void build(int n) {                          // 建 1..n 的序列
        for (int i = 1; i <= n; ++i) root = merge(root, newNode(i));
    }
    void reverse_range(int l, int r) {
        int a, b, c;
        split_sz(root, l - 1, a, b);
        split_sz(b, r - l + 1, b, c);
        apply_rev(b);
        root = merge(a, merge(b, c));
    }
    void output(int p, vector<int> &out) const {
        if (!p) return;
        // 注意：output 里也要 push_down，但 const 不能改，这里用非 const 版本
    }
    void inorder(int p, vector<int> &out) {
        if (!p) return;
        push_down(p);
        inorder(tr[p].l, out);
        out.push_back(tr[p].val);
        inorder(tr[p].r, out);
    }
};
```

> **翻转的正确性**：翻转 = 交换左右儿子 + 打标记；`split_sz` 和 `merge` 前必须 `push_down`。

### 2.3 可持久化 FHQ（了解）

去掉"按值/按大小分裂后原地改"，改成**分裂时复制节点**即可。用途少，代码长，**比赛出现频率低**。

---

## 3. Splay（伸展树）

**原理**：每次访问后把节点 `splay` 到根，均摊 `O(log n)`。支持**区间操作**（把 `l-1` 转到根、`r+1` 转到根的右儿子，则 `[l,r]` 就是右儿子的左子树）。

**什么时候用**：需要**区间操作 + 按位置访问**（LCT 的基础）、或题目要求"删除后重新插入"。

```cpp
#include <bits/stdc++.h>
using namespace std;

// Splay（按值排序）：插入、删除、排名、第 k 小、前驱、后继
struct Splay {
    struct Node {
        int ch[2] = {0, 0}, fa = 0, val = 0, cnt = 0, sz = 0;
    };
    vector<Node> tr;
    int root = 0;
    int tot = 0;

    Splay(int maxNodes = 200005) : tr(maxNodes) {}

    void push_up(int x) { tr[x].sz = tr[tr[x].ch[0]].sz + tr[tr[x].ch[1]].sz + tr[x].cnt; }
    bool is_right(int x) { return tr[tr[x].fa].ch[1] == x; }

    void rotate(int x) {
        int y = tr[x].fa, z = tr[y].fa, k = is_right(x);
        int b = tr[x].ch[k ^ 1];
        tr[x].fa = z;
        if (z) tr[z].ch[is_right(y)] = x;
        tr[x].ch[k ^ 1] = y;
        tr[y].fa = x;
        tr[y].ch[k] = b;
        if (b) tr[b].fa = y;
        push_up(y);
        push_up(x);
    }
    void splay(int x, int goal = 0) {
        while (tr[x].fa != goal) {
            int y = tr[x].fa, z = tr[y].fa;
            if (z != goal) {
                if (is_right(x) == is_right(y)) rotate(y);
                else rotate(x);
            }
            rotate(x);
        }
        if (!goal) root = x;
    }

    void insert(int v) {
        if (!root) {
            root = ++tot;
            tr[root] = Node{{0,0}, 0, v, 1, 1};
            return;
        }
        int x = root, fa = 0;
        while (x && tr[x].val != v) { fa = x; x = tr[x].ch[v > tr[x].val]; }
        if (x) { ++tr[x].cnt; ++tr[x].sz; splay(x); return; }
        x = ++tot;
        tr[x] = Node{{0,0}, fa, v, 1, 1};
        tr[fa].ch[v > tr[fa].val] = x;
        splay(x);
    }

    void find(int v) {
        int x = root;
        while (tr[x].val != v && tr[x].ch[v > tr[x].val]) x = tr[x].ch[v > tr[x].val];
        splay(x);
    }
    int rank_of(int v) {                       // 严格小于 v 的个数 + 1
        find(v);
        if (tr[root].val >= v) return tr[tr[root].ch[0]].sz + 1;
        return tr[tr[root].ch[0]].sz + tr[root].cnt + 1;
    }
    int kth(int k) {                           // 第 k 小
        int x = root;
        while (x) {
            int lsz = tr[tr[x].ch[0]].sz;
            if (k <= lsz) x = tr[x].ch[0];
            else if (k <= lsz + tr[x].cnt) { splay(x); return tr[x].val; }
            else { k -= lsz + tr[x].cnt; x = tr[x].ch[1]; }
        }
        return -1;
    }
    int prev_less(int v) {
        find(v);
        int lsz = tr[tr[root].ch[0]].sz;
        return lsz ? kth(lsz) : -1;
    }
    int next_greater(int v) {
        find(v);
        int extra = (tr[root].val == v ? tr[root].cnt : 0);
        return kth(rank_of(v) + extra);
    }

    void erase(int v) {
        find(v);
        int x = root;
        if (tr[x].cnt > 1) { --tr[x].cnt; --tr[x].sz; return; }
        // 合并左右子树
        int l = tr[x].ch[0], r = tr[x].ch[1];
        if (!l) { root = r; tr[r].fa = 0; return; }
        if (!r) { root = l; tr[l].fa = 0; return; }
        int p = l;
        while (tr[p].ch[1]) p = tr[p].ch[1];
        splay(p, x);
        tr[p].ch[1] = r;
        tr[r].fa = p;
        tr[p].fa = 0;
        root = p;
        push_up(root);
    }
};
```

**Splay 的问题**：常数大、代码长、易写错。**能用 FHQ 就用 FHQ**，Splay 只在"必须按位置区间操作"或"LCT"时用。

---

## 4. LCT（Link-Cut Tree）

**用途**：动态树（加边、删边、路径查询）。竞赛出现频率中等，代码长。
**建议**：除非题目明确是"动态树"，否则别写。

```cpp
// 完整 LCT 见 31 号文件（图论进阶）
```

---

## 5. 选型速查

| 需求 | 结构 |
|---|---|
| 插入/删除/排名/前驱后继 | `set`（**首选**） |
| 动态第 k 小 + 修改 | FHQ Treap / 整体二分 |
| **区间翻转** | FHQ（文艺平衡树） |
| **区间加/区间和 + 分裂合并** | FHQ |
| 可持久化 | 可持久化 FHQ / 主席树 |
| 动态树 | LCT |
| 区间推平 | ODT（数据随机） |

**通用提醒**：
1. FHQ 的 `split`/`merge` **必须** `push_up`，带标记的必须 `push_down`。
2. 随机优先级用 `mt19937`，别用 `rand()`。
3. 节点数组要**预留足够空间**（`insert` 次数 + 初始点数）。
4. 多测要重置 `root` 和 `tr.size()`。


### 怎么用（照抄这段 `main`，只改【输入】【输出】两处）

上面是模板本体。实际写题时：**把上面的 struct 整段粘过来 → 再粘这段 `main` → 只改标了【输入】【输出】的行**。

```cpp skip
#include <bits/stdc++.h>
using namespace std;

// ===== 粘贴点：把上面的 FHQSequence（文艺平衡树）粘到这里 =====

int main() {
    // ===================== 【输入】改这里 =====================
    int n, m;
    scanf("%d %d", &n, &m);               // 1..n 的序列，m 次翻转
    // ========================================================

    // ===================== 【核心】算法流程 =====================
    // 把序列按"下标"挂到平衡树上（中序遍历就是原序列）：
    //   翻转 [l,r]：split 出 [1,l-1] [l,r] [r+1,n] 三段
    //              → 中间那段打"翻转标记"（交换左右儿子 + rev ^= 1）
    //              → 再 merge 回去
    // 所有 split/merge 之前必须先 push_down，把标记推给儿子
    FHQSequence t;
    t.build(n);

    for (int i = 0; i < m; ++i) {
        int l, r;
        scanf("%d %d", &l, &r);
        t.reverse_range(l, r);
    }
    // ========================================================

    // ===================== 【输出】改这里 =====================
    vector<int> out;
    t.inorder(t.root, out);               // 中序遍历 = 当前序列
    for (int i = 0; i < (int)out.size(); ++i)
        printf("%d%c", out[i], " \n"[i + 1 == (int)out.size()]);
    // ========================================================
    return 0;
}
```

**普通平衡树（第 k 小 / 前驱后继）**：

```cpp skip
FHQ f;
f.insert(v);            // 插入
f.erase(v);             // 删除一个 v
int r = f.rank_of(v);   // v 的排名
int k = f.kth(k);       // 第 k 小
int p = f.prev_less(v); // 严格小于 v 的最大值
int q = f.next_greater(v);
```

---

## 6. 思路与例题索引

| 算法 | 思路一句话 | 用途 | 例题 |
|---|---|---|---|
| FHQ Treap | `split/merge` 代替旋转，`pri` 随机 | 动态第 k 小、前驱后继 | P3369【模板】普通平衡树 |
| FHQ 文艺平衡树 | 按大小 `split` + 翻转标记 | 区间翻转 | P3391【模板】文艺平衡树 |
| FHQ 区间操作 | `split` 出区间后整段打标记 | 区间加、区间和 | P1503 鬼子进村（变体） |
| Splay | 每次访问 splay 到根 | 区间操作、LCT 基础 | P3369、P3391 |
| 可持久化 FHQ | 分裂时复制节点 | 历史版本 | P3835【模板】可持久化平衡树 |

**建议**：能用 `set` 就用 `set`；必须区间翻转/分裂合并才写 FHQ。
