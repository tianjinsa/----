# 26 字符串进阶（SAM / PAM / AC 自动机 / 后缀数组）

> 基础（哈希/KMP/Manacher/Trie）见 [19-字符串基础](19-字符串基础-哈希KMPManacher.md)。
> 这些结构**含金量高、不可替代**，但代码较长。按"用途"决定要不要写。

---

## 1. 后缀自动机 SAM

### 1.1 用途

- **本质不同子串个数** = `Σ (len[v] - len[link[v]])`。
- 所有子串出现次数（在 `link` 树上做子树和）。
- 最长重复子串、两个串的最长公共子串。
- 子串排名 / 字典序第 k 小子串。

### 1.2 原理

一个自动机识别**所有子串**，状态数 `<= 2n-1`，转移数 `O(n)`。
每个状态代表一组 `endpos` 相同的子串，长度区间是 `(len[link[v]], len[v]]`。

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;

struct SuffixAutomaton {
    struct State {
        int len = 0, link = -1;
        int nxt[26];
        int occ = 0;                    // 该状态的 endpos 大小（出现次数）
        State() { memset(nxt, -1, sizeof nxt); }
    };
    vector<State> st;
    int last = 0;

    explicit SuffixAutomaton(int maxLen = 0) {
        st.reserve(maxLen ? 2 * maxLen + 2 : 2);
        st.push_back(State{});          // 0 是初始状态（空串）
    }

    void extend(int c) {
        int cur = (int)st.size();
        st.push_back(State{});
        st[cur].len = st[last].len + 1;
        st[cur].occ = 1;                // 每个前缀贡献一次
        int p = last;
        while (p != -1 && st[p].nxt[c] == -1) {
            st[p].nxt[c] = cur;
            p = st[p].link;
        }
        if (p == -1) {
            st[cur].link = 0;
        } else {
            int q = st[p].nxt[c];
            if (st[p].len + 1 == st[q].len) {
                st[cur].link = q;
            } else {
                int clone = (int)st.size();
                st.push_back(st[q]);            // 复制 q
                st[clone].len = st[p].len + 1;
                st[clone].occ = 0;              // clone 不直接贡献
                while (p != -1 && st[p].nxt[c] == q) {
                    st[p].nxt[c] = clone;
                    p = st[p].link;
                }
                st[q].link = st[cur].link = clone;
            }
        }
        last = cur;
    }

    // 本质不同子串个数
    ll distinct_substrings() const {
        ll res = 0;
        for (int i = 1; i < (int)st.size(); ++i) res += st[i].len - st[st[i].link].len;
        return res;
    }

    // 每个状态的出现次数：按 len 从大到小在 link 树上累加
    vector<int> occurrence_count() {
        int sz = (int)st.size();
        vector<int> cnt(sz, 0);
        for (int i = 0; i < sz; ++i) cnt[i] = st[i].occ;
        // 按 len 桶排序
        vector<int> order(sz), bucket(sz + 1, 0);
        for (int i = 0; i < sz; ++i) ++bucket[st[i].len];
        for (int i = 1; i <= sz; ++i) bucket[i] += bucket[i - 1];
        for (int i = sz - 1; i >= 0; --i) order[--bucket[st[i].len]] = i;
        for (int i = sz - 1; i >= 1; --i) {
            int v = order[i];
            if (st[v].link >= 0) cnt[st[v].link] += cnt[v];
        }
        return cnt;
    }

    // 两个串的最长公共子串：对 s 建 SAM，在 t 上跑
    int longest_common_substring(const string &t) const {
        int v = 0, l = 0, best = 0;
        for (char ch : t) {
            int c = ch - 'a';
            while (v && st[v].nxt[c] == -1) { v = st[v].link; l = st[v].len; }
            if (st[v].nxt[c] != -1) { v = st[v].nxt[c]; ++l; }
            else { v = 0; l = 0; }
            best = max(best, l);
        }
        return best;
    }
};

// 用法：SuffixAutomaton sam(s.size());
//       for (char c : s) sam.extend(c - 'a');
//       cout << sam.distinct_substrings();
```

### 1.3 实例

| 题 | 做法 |
|---|---|
| P3804【模板】后缀自动机 | `Σ len[v]-len[link[v]]`，或按状态统计出现次数 |
| P2408 不同子串个数 | `distinct_substrings()` |
| P3975 弦论 | SAM 上 DP 求第 k 小子串 |
| SP1811 LCS | 两串最长公共子串 |
| P4248 差异 | 后缀数组 + 单调栈（或 SAM 上贡献） |

---

## 2. 回文自动机 PAM（回文树）

### 2.1 用途

- **本质不同回文子串个数**。
- 每个回文子串的出现次数。
- 以每个位置结尾的最长回文后缀（配合 DP 做回文划分）。

### 2.2 原理

两个根：`0`（偶回文，长 0）和 `1`（奇回文，长 -1）。每个节点代表一个本质不同的回文串。

```cpp
#include <bits/stdc++.h>
using namespace std;

struct PalindromicTree {
    struct Node {
        int nxt[26];
        int fail = 0, len = 0, cnt = 0;
        Node() { memset(nxt, 0, sizeof nxt); }
    };
    vector<Node> tr;
    string s;
    int last = 1;                       // 最后插入的回文节点

    explicit PalindromicTree(int maxLen = 0) {
        tr.reserve(maxLen ? maxLen + 2 : 2);
        tr.push_back(Node{});           // 0: 偶根, len = 0
        tr.push_back(Node{});           // 1: 奇根, len = -1
        tr[0].fail = 1;
        tr[1].len = -1;
        s.clear();
    }

    // 找 s[i] 能接上的最长回文后缀
    int get_fail(int x, int i) {
        while (i - tr[x].len - 1 < 0 || s[i - tr[x].len - 1] != s[i]) x = tr[x].fail;
        return x;
    }

    void extend(char ch) {
        int c = ch - 'a';
        s.push_back(ch);
        int i = (int)s.size() - 1;
        int cur = get_fail(last, i);
        if (!tr[cur].nxt[c]) {
            int now = (int)tr.size();
            tr.push_back(Node{});
            tr[now].len = tr[cur].len + 2;
            if (tr[now].len == 1) {
                tr[now].fail = 0;
            } else {
                int f = get_fail(tr[cur].fail, i);
                tr[now].fail = tr[f].nxt[c];
            }
            tr[cur].nxt[c] = now;
        }
        last = tr[cur].nxt[c];
        ++tr[last].cnt;
    }

    // 统计每个回文串出现次数（在 fail 树上累加），必须最后调用
    void count_occurrence() {
        for (int i = (int)tr.size() - 1; i >= 2; --i) tr[tr[i].fail].cnt += tr[i].cnt;
    }

    int distinct_palindromes() const { return (int)tr.size() - 2; }
};
```

### 2.3 实例

| 题 | 做法 |
|---|---|
| P5496【模板】回文自动机 | 输出每个位置结尾的本质不同回文数 |
| P3649 [APIO2014] 回文串 | `len * cnt` 最大 |
| P4287 [SHOI2011] 双倍回文 | PAM + 跳 fail |

---

## 3. AC 自动机

### 3.1 用途

**多模式串**匹配：一次扫描文本，统计所有模式串出现次数。
典型题：P3808【模板】AC 自动机（简单版）、P3796（加强版）、P5357（二次加强版）。

### 3.2 原理

Trie + `fail` 指针（失配时跳到最长后缀对应的节点）。查询时沿 `fail` 链累加。

```cpp
#include <bits/stdc++.h>
using namespace std;

struct AhoCorasick {
    struct Node {
        int nxt[26];
        int fail = 0;
        int cnt = 0;                     // 以该节点结尾的模式串个数
        Node() { memset(nxt, 0, sizeof nxt); }
    };
    vector<Node> tr;
    vector<int> matchCnt;                // 每个模式串出现次数（最后沿 fail 树累加）

    AhoCorasick() { tr.push_back(Node{}); }

    void insert(const string &s, int id) {
        int u = 0;
        for (char ch : s) {
            int c = ch - 'a';
            if (!tr[u].nxt[c]) {
                tr[u].nxt[c] = (int)tr.size();
                tr.push_back(Node{});
            }
            u = tr[u].nxt[c];
        }
        tr[u].cnt += 1;                  // 若有多个相同模式串，用 vector 记录 id
        (void)id;
    }

    void build() {                       // BFS 建 fail
        queue<int> q;
        for (int c = 0; c < 26; ++c)
            if (tr[0].nxt[c]) q.push(tr[0].nxt[c]);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int c = 0; c < 26; ++c) {
                int v = tr[u].nxt[c];
                if (v) {
                    tr[v].fail = tr[tr[u].fail].nxt[c];
                    q.push(v);
                } else {
                    tr[u].nxt[c] = tr[tr[u].fail].nxt[c];   // 路径压缩
                }
            }
        }
    }

    // 扫描文本，返回每个节点的命中次数
    vector<int> query(const string &t) {
        vector<int> hit(tr.size(), 0);
        int u = 0;
        for (char ch : t) {
            u = tr[u].nxt[ch - 'a'];
            ++hit[u];
        }
        // 沿 fail 树从深到浅累加
        vector<int> order;
        vector<int> deg(tr.size(), 0);
        for (int i = 1; i < (int)tr.size(); ++i) ++deg[tr[i].fail];
        queue<int> q;
        for (int i = 0; i < (int)tr.size(); ++i) if (!deg[i]) q.push(i);
        while (!q.empty()) {
            int v = q.front(); q.pop();
            order.push_back(v);
            int f = tr[v].fail;
            if (--deg[f] == 0) q.push(f);
        }
        for (int v : order) if (v) hit[tr[v].fail] += hit[v];
        return hit;
    }
};
```

**实例**：P3808 直接输出匹配到的模式串个数；P5357 需要每个模式串的出现次数 → 用 `fail` 树 + 子树和。

---

## 4. 后缀数组 SA

### 4.1 用途

- **后缀排序**、`LCP` 数组。
- 本质不同子串数 = `n(n+1)/2 - Σ lcp[i]`。
- 最长重复子串、两个后缀的最长公共前缀。
- 配合并查集/单调栈做"区间最小值之和"。

### 4.2 原理

`sa[i]` = 排名第 `i` 的后缀起始位置；`rk[i]` = 后缀 `i` 的排名；`lcp[i] = LCP(sa[i-1], sa[i])`。

```cpp
#include <bits/stdc++.h>
using namespace std;

// O(n log n) 倍增法求后缀数组
struct SuffixArray {
    string s;
    int n;
    vector<int> sa, rk, lcp;

    explicit SuffixArray(const string &str) : s(str), n((int)str.size()) { build(); }

    void build() {
        sa.assign(n, 0);
        rk.assign(n, 0);
        vector<int> tmp(n);
        for (int i = 0; i < n; ++i) { sa[i] = i; rk[i] = (unsigned char)s[i]; }
        for (int k = 1; k < n; k <<= 1) {
            auto cmp = [&](int a, int b) {
                if (rk[a] != rk[b]) return rk[a] < rk[b];
                int ra = (a + k < n) ? rk[a + k] : -1;
                int rb = (b + k < n) ? rk[b + k] : -1;
                return ra < rb;
            };
            sort(sa.begin(), sa.end(), cmp);
            tmp[sa[0]] = 0;
            for (int i = 1; i < n; ++i)
                tmp[sa[i]] = tmp[sa[i - 1]] + (cmp(sa[i - 1], sa[i]) ? 1 : 0);
            rk = tmp;
        }
    }

    // Kasai：lcp[i] = LCP(sa[i-1], sa[i])，lcp[0] = 0
    void build_lcp() {
        lcp.assign(n, 0);
        int h = 0;
        for (int i = 0; i < n; ++i) {
            if (rk[i] == 0) continue;
            int j = sa[rk[i] - 1];
            while (i + h < n && j + h < n && s[i + h] == s[j + h]) ++h;
            lcp[rk[i]] = h;
            if (h) --h;
        }
    }

    // 本质不同子串数
    long long distinct_substrings() {
        build_lcp();
        long long total = 1LL * n * (n + 1) / 2;
        long long repeat = 0;
        for (int i = 1; i < n; ++i) repeat += lcp[i];
        return total - repeat;
    }
};
```

> **注意**：上面用的是 `O(n log^2 n)`（`sort` 带比较器）。要 `O(n log n)` 用**基数排序**版本，代码更长。
> 比赛里 `n <= 2e5` 时 `sort` 版通常够用（约 1e8 次比较，需要卡常）。

### 4.3 实例

| 题 | 做法 |
|---|---|
| P3809【模板】后缀排序 | 输出 `sa` |
| P2408 不同子串个数 | `n(n+1)/2 - Σlcp` |
| P4248 差异 | `Σ lcp(i,j)` = 单调栈求 `lcp` 区间贡献 |
| P2178 品酒大会 | 并查集按 `lcp` 合并 |

---

## 5. 选型速查

| 需求 | 结构 |
|---|---|
| 本质不同子串数 | SAM / SA |
| 子串出现次数 | SAM（link 树） |
| 回文串统计 | PAM |
| 多模式串匹配 | AC 自动机 |
| 后缀排序 / LCP | 后缀数组 |
| 字典序第 k 小子串 | SAM + DP / SA |
| 最长公共子串（两串） | SAM / SA + 二分 |

**代码量提醒**：这四个结构都偏长。**能用哈希 + 二分解决的（如最长重复子串），优先哈希**。


### 怎么用（照抄这段 `main`，只改【输入】【输出】两处）

上面是模板本体。实际写题时：**把上面的 struct 整段粘过来 → 再粘这段 `main` → 只改标了【输入】【输出】的行**。

```cpp skip
#include <bits/stdc++.h>
using namespace std;
using ll = long long;

// ===== 粘贴点：把上面的 SuffixAutomaton 粘到这里 =====

int main() {
    // ===================== 【输入】改这里 =====================
    char buf[1000006];
    scanf("%s", buf);
    string s = buf;
    // ========================================================

    // ===================== 【核心】算法流程 =====================
    // 后缀自动机 SAM：
    //   逐字符 extend(c)，每个状态代表一组 endpos 相同的子串
    //   状态 v 代表的子串长度区间是 (len[link[v]], len[v]]
    //   → 本质不同子串数 = Σ (len[v] - len[link[v]])
    //   → 出现次数：在 link 树上做子树和（见 occurrence_count）
    SuffixAutomaton sam((int)s.size());
    for (char c : s) sam.extend(c - 'a');

    ll distinct = sam.distinct_substrings();
    // ========================================================

    // ===================== 【输出】改这里 =====================
    printf("%lld\n", distinct);
    // ========================================================
    return 0;
}
```

**PAM / AC 自动机 / 后缀数组** 的核心调用：

```cpp skip
PalindromicTree pam((int)s.size());
for (char c : s) pam.extend(c);
pam.count_occurrence();               // 先累加 fail 树
int d = pam.distinct_palindromes();   // 本质不同回文数

AhoCorasick ac;
ac.insert(pat, id);                   // 插入模式串
ac.build();                           // BFS 建 fail
auto hit = ac.query(text);            // 每个节点的命中次数

SuffixArray sa(s);
sa.build_lcp();
ll cnt = sa.distinct_substrings();
```

---

## 6. 思路与例题索引

| 算法 | 思路一句话 | 用途 | 例题 |
|---|---|---|---|
| SAM | 每个状态代表一组 `endpos` 相同的子串 | 本质不同子串、出现次数 | P3804【模板】后缀自动机 |
| SAM 出现次数 | `link` 树子树和 | 子串计数 | P2408 不同子串个数 |
| SAM 最长公共子串 | 在 t 上沿转移/`link` 跑 | 两串 LCS | SP1811 LCS |
| PAM | 偶根 0 + 奇根 1，`fail` 跳 | 回文串统计 | P5496【模板】回文自动机 |
| PAM 计数 | `fail` 树累加 | 回文出现次数 | P3649 [APIO2014] 回文串 |
| AC 自动机 | Trie + `fail`（最长后缀） | 多模式串匹配 | P3808【模板】AC 自动机 |
| AC + fail 树 | 子树和统计每个模式串次数 | 每个模式串出现次数 | P5357【模板】AC 自动机（二次加强版） |
| 后缀数组 | 倍增排序 + Kasai 求 LCP | 后缀排序、不同子串 | P3809【模板】后缀排序 |
| SA + 单调栈 | `lcp` 区间贡献 | 差异、品酒大会 | P4248 差异、P2178 品酒大会 |

**选择**：子串统计 → SAM；回文 → PAM；多模式匹配 → AC；后缀序 → SA。
