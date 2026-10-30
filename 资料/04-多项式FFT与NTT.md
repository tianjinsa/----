# 04 多项式：FFT / NTT

> 卷积 `C[k] = Σ A[i] * B[k-i]`，FFT `O(n log n)` 实数，NTT 取模。
> **比赛首选 NTT（模 998244353）**；模数不是 998244353 时用三模 CRT 或 MTT（拆位 FFT）。

---

## 1. NTT（998244353，原根 3）

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;

namespace NTT {
    constexpr int MOD = 998244353;      // 2^23 * 119 + 1
    constexpr int G = 3;                // 原根

    ll qpow(ll a, ll b) {
        ll r = 1; a %= MOD;
        while (b) { if (b & 1) r = r * a % MOD; a = a * a % MOD; b >>= 1; }
        return r;
    }

    // 原地 NTT/INTT。invert=true 时做逆变换（含除以 n）
    void ntt(vector<int> &a, bool invert) {
        int n = (int)a.size();
        // 位逆序置换
        for (int i = 1, j = 0; i < n; ++i) {
            int bit = n >> 1;
            for (; j & bit; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) swap(a[i], a[j]);
        }
        for (int len = 2; len <= n; len <<= 1) {
            ll w = qpow(G, (MOD - 1) / len);
            if (invert) w = qpow(w, MOD - 2);
            int half = len >> 1;
            for (int i = 0; i < n; i += len) {
                ll wn = 1;
                for (int j = 0; j < half; ++j) {
                    int u = a[i + j];
                    int v = (int)(wn * a[i + j + half] % MOD);
                    int x = u + v; if (x >= MOD) x -= MOD;
                    int y = u - v; if (y < 0) y += MOD;
                    a[i + j] = x;
                    a[i + j + half] = y;
                    wn = wn * w % MOD;
                }
            }
        }
        if (invert) {
            ll inv_n = qpow(n, MOD - 2);
            for (int &x : a) x = (int)(x * inv_n % MOD);
        }
    }

    // 卷积（结果长度 = a.size()+b.size()-1）
    vector<int> multiply(vector<int> a, vector<int> b) {
        if (a.empty() || b.empty()) return {};
        int need = (int)a.size() + (int)b.size() - 1;
        int n = 1; while (n < need) n <<= 1;
        a.resize(n); b.resize(n);
        ntt(a, false); ntt(b, false);
        for (int i = 0; i < n; ++i) a[i] = (ll)a[i] * b[i] % MOD;
        ntt(a, true);
        a.resize(need);
        return a;
    }
}
```

**用法**：

```cpp skip
// 洛谷 P3803 【模板】多项式乘法（依赖上面的 namespace NTT，单独编译会报错，故标记 skip）
#include <bits/stdc++.h>
using namespace std;
int main() {
    int n, m;
    scanf("%d %d", &n, &m);
    vector<int> a(n + 1), b(m + 1);
    for (int &x : a) scanf("%d", &x);
    for (int &x : b) scanf("%d", &x);
    vector<int> c = NTT::multiply(a, b);
    for (int i = 0; i < (int)c.size(); ++i) printf("%d%c", c[i], " \n"[i + 1 == (int)c.size()]);
    return 0;
}
```

> 上面这块需要 `NTT` 命名空间在同一文件里。实际抄的时候把整个 `namespace NTT` 一起贴上即可。

---

## 2. 迭代版 NTT（位逆序预处理，更快）

比赛里 NTT 会被卡常数，下面这版把位逆序和单位根预处理出来，**比上面快 2~3 倍**，推荐背这个：

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;

template <int MOD, int G>
struct PolyNTT {
    vector<int> rev, roots{0, 1};

    ll qpow(ll a, ll b) const {
        ll r = 1; a %= MOD;
        while (b) { if (b & 1) r = r * a % MOD; a = a * a % MOD; b >>= 1; }
        return r;
    }

    void ensure_roots(int n) {
        if ((int)roots.size() >= n) return;
        int k = __builtin_ctz((unsigned)roots.size());
        roots.resize(n);
        while ((1 << k) < n) {
            int e = qpow(G, (MOD - 1) >> (k + 1));
            for (int i = 1 << (k - 1); i < (1 << k); ++i) {
                roots[2 * i] = roots[i];
                roots[2 * i + 1] = (ll)roots[i] * e % MOD;
            }
            ++k;
        }
    }

    void ntt(vector<int> &a, bool invert) {
        int n = (int)a.size();
        ensure_roots(n);
        if ((int)rev.size() != n) {
            rev.assign(n, 0);
            for (int i = 1; i < n; ++i) rev[i] = (rev[i >> 1] >> 1) | ((i & 1) ? n >> 1 : 0);
        }
        for (int i = 0; i < n; ++i) if (i < rev[i]) swap(a[i], a[rev[i]]);
        for (int len = 1; len < n; len <<= 1) {
            for (int i = 0; i < n; i += len << 1) {
                for (int j = 0; j < len; ++j) {
                    int u = a[i + j];
                    int v = (ll)a[i + j + len] * roots[len + j] % MOD;
                    int x = u + v; if (x >= MOD) x -= MOD;
                    int y = u - v; if (y < 0) y += MOD;
                    a[i + j] = x;
                    a[i + j + len] = y;
                }
            }
        }
        if (invert) {
            reverse(a.begin() + 1, a.end());
            ll inv_n = qpow(n, MOD - 2);
            for (int &x : a) x = (ll)x * inv_n % MOD;
        }
    }

    vector<int> multiply(const vector<int> &a, const vector<int> &b) {
        if (a.empty() || b.empty()) return {};
        int need = (int)a.size() + (int)b.size() - 1;
        int n = 1; while (n < need) n <<= 1;
        vector<int> fa(a.begin(), a.end()), fb(b.begin(), b.end());
        fa.resize(n); fb.resize(n);
        ntt(fa, false); ntt(fb, false);
        for (int i = 0; i < n; ++i) fa[i] = (ll)fa[i] * fb[i] % MOD;
        ntt(fa, true);
        fa.resize(need);
        return fa;
    }
};
// 用法：PolyNTT<998244353, 3> poly;
```

> `invert` 用的技巧：**逆变换 = 正变换后把 `a[1..n-1]` 翻转，再除以 n**。这是标准结论，比每次算逆原根快。

---

## 3. FFT（实数，long double 防精度）

```cpp
#include <bits/stdc++.h>
using namespace std;
using cd = complex<double>;
constexpr double PI = acos(-1.0);

void fft(vector<cd> &a, bool invert) {
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2 * PI / len * (invert ? -1 : 1);
        cd wlen(cos(ang), sin(ang));
        for (int i = 0; i < n; i += len) {
            cd w(1);
            for (int j = 0; j < len / 2; ++j) {
                cd u = a[i + j], v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
    if (invert)
        for (cd &x : a) x /= n;
}

vector<long long> convolve_ll(const vector<long long> &a, const vector<long long> &b) {
    if (a.empty() || b.empty()) return {};
    int need = (int)a.size() + (int)b.size() - 1;
    int n = 1; while (n < need) n <<= 1;
    vector<cd> fa(a.begin(), a.end()), fb(b.begin(), b.end());
    fa.resize(n); fb.resize(n);
    fft(fa, false); fft(fb, false);
    for (int i = 0; i < n; ++i) fa[i] *= fb[i];
    fft(fa, true);
    vector<long long> res(need);
    for (int i = 0; i < need; ++i) res[i] = llround(fa[i].real());
    return res;
}
```

**精度**：系数 `<= 1e5`、长度 `<= 2e5` 时，`double` 基本够；再大用 `long double` 或拆位法。

---

## 4. 拆位 FFT（MTT，任意模数）

把系数拆成 `a = a1 * BASE + a0`（`BASE = 2^15`），做 4 次 FFT，再合并：

```cpp
#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using cd = complex<double>;
constexpr double PI = acos(-1.0);
constexpr int BASE = 1 << 15;

void fft(vector<cd> &a, bool invert) {
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2 * PI / len * (invert ? -1 : 1);
        cd wlen(cos(ang), sin(ang));
        for (int i = 0; i < n; i += len) {
            cd w(1);
            for (int j = 0; j < len / 2; ++j) {
                cd u = a[i + j], v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
    if (invert) for (cd &x : a) x /= n;
}

// 任意模数卷积：系数 < 2^30，结果 mod p
vector<int> mtt(const vector<int> &A, const vector<int> &B, ll mod) {
    int need = (int)A.size() + (int)B.size() - 1;
    int n = 1; while (n < need) n <<= 1;
    vector<cd> a1(n), a2(n), b1(n), b2(n);
    for (int i = 0; i < (int)A.size(); ++i) {
        a1[i] = cd(A[i] / BASE, 0);
        a2[i] = cd(A[i] % BASE, 0);
    }
    for (int i = 0; i < (int)B.size(); ++i) {
        b1[i] = cd(B[i] / BASE, 0);
        b2[i] = cd(B[i] % BASE, 0);
    }
    fft(a1, false); fft(a2, false); fft(b1, false); fft(b2, false);
    vector<cd> c1(n), c2(n), c3(n);
    for (int i = 0; i < n; ++i) {
        c1[i] = a1[i] * b1[i];                 // 高位 * 高位
        c2[i] = a1[i] * b2[i] + a2[i] * b1[i]; // 交叉项
        c3[i] = a2[i] * b2[i];                 // 低位 * 低位
    }
    fft(c1, true); fft(c2, true); fft(c3, true);
    vector<int> res(need);
    for (int i = 0; i < need; ++i) {
        ll x1 = llround(c1[i].real()) % mod;
        ll x2 = llround(c2[i].real()) % mod;
        ll x3 = llround(c3[i].real()) % mod;
        ll v = ((x1 % mod) * (BASE % mod) % mod * (BASE % mod)
                + x2 * (BASE % mod) + x3) % mod;
        res[i] = (int)((v % mod + mod) % mod);
    }
    return res;
}
```

---

## 5. 多项式常用结论

| 操作 | 复杂度 | 备注 |
|---|---|---|
| 卷积 | `O(n log n)` | NTT |
| 多项式求逆 | `O(n log n)` | 牛顿迭代，见 27 |
| 多项式开根 | `O(n log n)` | 需要逆元 |
| 多项式 ln | `O(n log n)` | 求导+积分+逆 |
| 多项式 exp | `O(n log n)` | 牛顿迭代 |
| 多项式 pow | `O(n log n log k)` | `exp(k ln f)` |
| 多项式除法/取模 | `O(n log n)` | 反转 + 逆 |
| 多点求值/插值 | `O(n log^2 n)` | 分治+卷积 |

> 完整"多项式全家桶"在 [27-多项式全家桶与生成函数](27-多项式全家桶与生成函数.md)。

**单位根判定**：`MOD = c * 2^k + 1` 且 `c` 奇时支持长度 `<= 2^k` 的 NTT。
常见：`998244353 = 119*2^23+1`（支持最长 `2^23`）、`1004535809 = 479*2^21+1`、`469762049 = 7*2^26+1`。

---

## 6. 思路与例题索引

| 算法 | 思路一句话 | 用途 | 例题 |
|---|---|---|---|
| NTT | 把系数转成点值相乘再转回，用原根代替单位根 | 模 998244353 的卷积 | P3803【模板】多项式乘法 |
| 迭代 NTT | 预处理位逆序 + 单位根，逆变换用"翻转+除 n" | 卡常数 | P3803、P1919 |
| FFT | 复数单位根，`complex<double>` | 实数卷积 | P1919 A*B Problem |
| 拆位 MTT | 系数拆成高低 15 位，做 4 次 FFT | 任意模数 | P4245 |
| 三模 CRT | 3 个 NTT 模数 + CRT | 任意模数（系数大） | P4245 |

**判断**：模数是 998244353 直接用 NTT；不是就 MTT/三模。系数 < 1e5 且长度 < 2e5 时 double FFT 够用。
