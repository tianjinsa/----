# 19 字符串基础（哈希 / KMP / Z / Manacher）

> 基础字符串算法，模板短、必须会。进阶（SAM/PAM/AC 自动机/后缀数组）见 [26-字符串进阶](26-字符串进阶-SAM与AC自动机.md)。

---

## 1. 字符串哈希

**思路**：把字符串看成 `base` 进制数取模，用前缀哈希做 `O(1)` 子串比较。
**要点**：`base` 取 `131/13331`，用 `unsigned long long` 自然溢出（省一次取模，防卡可用双哈希）。

```cpp
#include <bits/stdc++.h>
using namespace std;
using ull = unsigned long long;

struct StrHash {
    int n;
    ull base;
    vector<ull> h, p;

    explicit StrHash(const string &s, ull base_ = 131) : n((int)s.size()), base(base_) {
        h.assign(n + 1, 0);
        p.assign(n + 1, 1);
        for (int i = 0; i < n; ++i) {
            h[i + 1] = h[i] * base + (ull)(s[i] - 'a' + 1);
            p[i + 1] = p[i] * base;
        }
    }
    // 子串 [l, r]（0-based）的哈希
    ull get(int l, int r) const {
        return h[r + 1] - h[l] * p[r - l + 1];
    }
    // 最长公共前缀
    int lcp(int i, int j) const {
        int lo = 0, hi = min(n - i, n - j);
        while (lo < hi) {
            int mid = (lo + hi + 1) / 2;
            if (get(i, i + mid - 1) == get(j, j + mid - 1)) lo = mid;
            else hi = mid - 1;
        }
        return lo;
    }
};
```

**双哈希（防被卡）**：

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;

struct DoubleHash {
    static constexpr ll M1 = 1000000007, M2 = 1000000009, B1 = 131, B2 = 13331;
    vector<ll> h1, h2, p1, p2;
    int n;
    explicit DoubleHash(const string &s) : n((int)s.size()) {
        h1.assign(n + 1, 0); h2.assign(n + 1, 0);
        p1.assign(n + 1, 1); p2.assign(n + 1, 1);
        for (int i = 0; i < n; ++i) {
            h1[i + 1] = (h1[i] * B1 + s[i]) % M1;
            h2[i + 1] = (h2[i] * B2 + s[i]) % M2;
            p1[i + 1] = p1[i] * B1 % M1;
            p2[i + 1] = p2[i] * B2 % M2;
        }
    }
    pair<ll,ll> get(int l, int r) const {
        ll a = (h1[r + 1] - h1[l] * p1[r - l + 1] % M1 + M1) % M1;
        ll b = (h2[r + 1] - h2[l] * p2[r - l + 1] % M2 + M2) % M2;
        return {a, b};
    }
};
```

**用途**：判回文、判重复子串、最长重复子串（二分 + 哈希）、字符串集合去重、O(1) 子串比较。

---

## 2. KMP

**思路**：`nxt[i]` 表示 `s[0..i]` 的最长"相等前后缀"长度。失配时用 `nxt` 跳，避免回退主串指针。
**复杂度**：`O(n + m)`。

```cpp
#include <bits/stdc++.h>
using namespace std;

struct KMP {
    string pat;
    vector<int> nxt;                    // nxt[i] = pat[0..i] 的最长相同前后缀长度
    explicit KMP(const string &p) : pat(p) {
        int m = (int)pat.size();
        nxt.assign(m, 0);
        for (int i = 1, j = 0; i < m; ++i) {
            while (j && pat[i] != pat[j]) j = nxt[j - 1];
            if (pat[i] == pat[j]) ++j;
            nxt[i] = j;
        }
    }
    // 返回所有出现位置（0-based 起始下标）
    vector<int> search(const string &text) const {
        vector<int> res;
        int m = (int)pat.size();
        for (int i = 0, j = 0; i < (int)text.size(); ++i) {
            while (j && text[i] != pat[j]) j = nxt[j - 1];
            if (text[i] == pat[j]) ++j;
            if (j == m) { res.push_back(i - m + 1); j = nxt[j - 1]; }
        }
        return res;
    }
};
```

**用途**：
- 单模式串匹配。
- **求最小循环节**：`n - nxt[n-1]`（若 `n % (n - nxt[n-1]) == 0` 则是循环节）。
- 求所有 border、求字符串周期。
- 统计前缀出现次数：沿 `nxt` 链累加。

**实例：P4391 无线传输** —— 求最小循环节，答案 `n - nxt[n-1]`。

---

## 3. Z 函数（扩展 KMP）

**思路**：`z[i]` = `s[i..]` 与 `s` 的最长公共前缀长度。
**复杂度**：`O(n)`。

```cpp
#include <bits/stdc++.h>
using namespace std;

struct ZFunc {
    vector<int> z;
    explicit ZFunc(const string &s) {
        int n = (int)s.size();
        z.assign(n, 0);
        z[0] = n;
        for (int i = 1, l = 0, r = 0; i < n; ++i) {
            if (i <= r) z[i] = min(r - i + 1, z[i - l]);
            while (i + z[i] < n && s[z[i]] == s[i + z[i]]) ++z[i];
            if (i + z[i] - 1 > r) { l = i; r = i + z[i] - 1; }
        }
    }
};

// 模式串匹配：构造 pat + '#' + text，z[i] == pat.size() 的位置就是匹配
```

**用途**：模式匹配、求字符串周期、求"每个后缀与原串的 LCP"、回文相关。

**实例：CF 432D 前缀出现次数** —— `z[i]` 与 `nxt` 结合统计每个前缀出现次数。

---

## 4. Manacher（最长回文子串）

**思路**：插 `#` 把奇偶回文统一；用已求出的右边界加速。`O(n)`。

```cpp
#include <bits/stdc++.h>
using namespace std;

struct Manacher {
    vector<int> rad;                      // rad[i] = 以 t[i] 为中心的回文半径
    string t;

    explicit Manacher(const string &s) {
        t = "^";
        for (char c : s) { t += c; t += '#'; }
        t += '$';
        int m = (int)t.size();
        rad.assign(m, 0);
        int c = 0, rr = 0;
        for (int i = 1; i < m - 1; ++i) {
            if (i < rr) rad[i] = min(rr - i, rad[2 * c - i]);
            while (t[i + rad[i]] == t[i - rad[i]]) ++rad[i];
            if (i + rad[i] > rr) { c = i; rr = i + rad[i]; }
        }
    }
    int longest() const {
        int best = 0;
        for (int i = 1; i + 1 < (int)t.size(); ++i) best = max(best, rad[i] - 1);
        return best;
    }
    // 原串 [l, r] 是否回文（0-based）
    bool is_palindrome(int l, int r) const {
        int center = l + r + 2;
        return rad[center] - 1 >= r - l + 1;
    }
};
```

**用途**：最长回文子串、回文子串计数、判断任意区间回文（`O(1)`）。

**实例：P3805 Manacher 模板**、P1659 拉拉队排练（回文长度计数 + 快速幂）。

---

## 5. 字典树 Trie

**思路**：把字符串挂在树的边上，节点表示前缀。`O(L)` 插入/查询。

```cpp
#include <bits/stdc++.h>
using namespace std;

struct Trie {
    struct Node {
        int ch[26];
        int cnt;                          // 经过该节点的串数
        int end;                          // 以该节点结尾的串数
        Node() { memset(ch, 0, sizeof ch); cnt = end = 0; }
    };
    vector<Node> tr;
    Trie() { tr.push_back(Node{}); }      // 0 是根

    void insert(const string &s) {
        int u = 0;
        for (char c : s) {
            int x = c - 'a';
            if (!tr[u].ch[x]) { tr[u].ch[x] = (int)tr.size(); tr.push_back(Node{}); }
            u = tr[u].ch[x];
            ++tr[u].cnt;
        }
        ++tr[u].end;
    }
    // 该串作为前缀出现过几次
    int count_prefix(const string &s) const {
        int u = 0;
        for (char c : s) {
            int x = c - 'a';
            if (!tr[u].ch[x]) return 0;
            u = tr[u].ch[x];
        }
        return tr[u].cnt;
    }
    bool contains(const string &s) const {
        int u = 0;
        for (char c : s) {
            int x = c - 'a';
            if (!tr[u].ch[x]) return false;
            u = tr[u].ch[x];
        }
        return tr[u].end > 0;
    }
};
```

**用途**：前缀统计、字符串去重、`01-Trie`（异或最大值/第 k 大异或，见 29 号文件的线性基/01-Trie）。

**实例：P2580 于是他错误的点名开始了** —— 判是否出现过 + 是否重复点名；用 `end` 和 `cnt` 区分。

---

## 6. `unordered_map` / `unordered_set` 防被卡

> **为什么会被卡**：`unordered_map` 的桶数是**质数**（如 `1031`），元素落桶规则是
> `bucket = hash(key) % bucket_count`。如果出题人构造了**全部同余**的 key
> （例如所有 key 都是桶数的倍数），所有元素会挤进同一个桶，链表退化，
> `O(1)` 变成 `O(n)`。

**实测**（g++ 15.2，`-O2`，`reserve(100000)` 后）：

| 数据 | 10 万次插入耗时 |
|---|---|
| 正常 key `0,1,2,...` | **2.6 ms** |
| 恶意 key `0, B, 2B, 3B, ...`（`B` = 桶数） | **8523 ms**（慢 3300 倍） |

### 6.1 三招防卡（从便宜到彻底）

```cpp
// 第 1 招：自定义哈希，给 key 加随机扰动（最常用、最有效）
struct CustomHash {
    static uint64_t splitmix64(uint64_t x) {
        x += 0x9e3779b97f4a7c15ULL;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }
    // 固定的随机种子：同一进程内一致，但外部无法预测
    static uint64_t FIXED_RANDOM;
    size_t operator()(uint64_t x) const {
        return splitmix64(x + FIXED_RANDOM);
    }
};
uint64_t CustomHash::FIXED_RANDOM =
    chrono::steady_clock::now().time_since_epoch().count();

unordered_map<long long, int, CustomHash> mp;
unordered_set<long long, CustomHash> st;
```

```cpp skip
// 第 2 招：先 reserve，避免反复 rehash（顺便把负载因子压到 0.5 以下）
mp.reserve(200000);              // 预估元素个数
mp.max_load_factor(0.7);         // 默认是 1.0，压到 0.7 更好
mp.rehash(200000);               // 强制立刻把桶数扩到位
```

```cpp skip
// 第 3 招：稳妥起见，能排序 + 二分就别用哈希
vector<long long> v;             // 存所有 key
sort(v.begin(), v.end());
bool found = binary_search(v.begin(), v.end(), key);   // O(log n)，永不被卡
```

### 6.2 为什么 `reserve` 不能完全防卡

`reserve` 只改变**桶数**，不改变 `hash(key) % B` 的分布规律。
只要出题人知道你的桶数 `B`，仍可以构造 `B` 的倍数让所有 key 同余。
所以**第 1 招（自定义哈希）才是根本**，第 2 招只是加速。

### 6.3 什么时候必须防

| 场景 | 要不要防 |
|---|---|
| 题目数据是随机的 | 不必 |
| 题目是 Codeforces / AtCoder 的 hack 赛 | **必须**（有人专门卡 `unordered_map`） |
| 用 `map`（红黑树） | 本来就不会被卡，只是常数大 |
| 用 `unordered_map` 存 `string` | 必须，`std::hash<string>` 也可能被卡 |
| 只需要"存在性查询" | 直接排序 + 二分，最简单 |

### 6.4 字符串哈希本身也可能被卡

双哈希（两个模数）已能防住绝大多数构造；如果还担心：

```cpp
// 用固定随机 base（不要用 131、13331 这些常见值）
static const uint64_t BASE = chrono::steady_clock::now().time_since_epoch().count() | 1;
```

**记忆要点**：
- `unordered_map` 的桶数是**质数**，`hash % bucket_count` 决定落桶。
- 卡法：让所有 key 都同余于 `bucket_count`。
- 根本解法：**自定义哈希加随机扰动**；辅助解法：`reserve` + `max_load_factor(0.7)`。
- 能排序二分就排序二分。


---

## 7. 常见套路对照

| 需求 | 方法 |
|---|---|
| 单模式匹配 | KMP |
| 多模式匹配 | AC 自动机（26） |
| O(1) 子串比较 | 哈希 |
| 最长回文 | Manacher |
| 前缀/字典序 | Trie |
| 后缀排序 / LCP | 后缀数组（26） |
| 本质不同子串数 | SAM（26） |
| 最小循环节 | KMP 的 `n - nxt[n-1]` |
| 最长重复子串 | 二分 + 哈希 / 后缀数组 |
| 字符串集合判重 | 哈希 / Trie |

---

## 8. 思路与例题索引

| 算法 | 思路一句话 | 用途 | 例题 |
|---|---|---|---|
| 字符串哈希 | 前缀 hash，子串 `O(1)` 比较 | 判重、判回文、LCP | P3375 变体、P5546 公共串 |
| 双哈希 | 两个模数同时比较 | 防被卡 | P5270 无论怎样神树大人都会删库跑路 |
| KMP | `nxt` 存最长相同前后缀，失配跳 | 单模式匹配、最小循环节 | P3375【模板】KMP |
| 最小循环节 | `n - nxt[n-1]`（能整除时） | 周期问题 | P4391 无线传输、P3435 |
| Z 函数 | `z[i]` = 后缀与全串 LCP | 模式匹配、周期 | P5410【模板】扩展 KMP |
| Manacher | 插 `#` + 右边界加速 | 最长回文、判回文 | P3805【模板】Manacher |
| Trie | 边表示字符，节点表示前缀 | 前缀统计、去重 | P2580 于是他错误的点名开始了 |
| 01-Trie | 二进制建 Trie | 异或最大值 | P4551 最长异或路径 |

**注意**：哈希能解决的（最长重复子串等）优先哈希，代码短。
