# ICPC 位运算、位处理函数与 `std::bitset` 速查

> 主要面向 **GNU C++17**。`__builtin_*` 是 GCC/Clang 扩展，不是 `std` 函数；`std::bitset` 是标准库定长二进制容器；C++20 `<bit>` 单独列在第 6 节。

---

## 1. 一页速查

### 1.1 位运算与常用公式

| 操作 | 写法 | 备注 |
|---|---|---|
| 与、或、异或、取反 | `a & b`、`a \| b`、`a ^ b`、`~a` | `^` 是异或，不是乘方 |
| 左移、右移 | `x << k`、`x >> k` | 非负位移且小于类型位宽 |
| 第 `k` 位掩码（64 位） | `1ULL << k` | 不要写成可能溢出的 `1 << k` |
| 取第 `k` 位 | `(x >> k) & 1` | 返回 `0/1` |
| 第 `k` 位置 1 | `x \|= 1ULL << k` | |
| 第 `k` 位置 0 | `x &= ~(1ULL << k)` | |
| 翻转第 `k` 位 | `x ^= 1ULL << k` | |
| 判断第 `k` 位是否为 1 | `x & (1ULL << k)` | 结果是掩码，不一定是 `1` |
| 最低位的 1（lowbit） | `x & -x` | 建议用于无符号数；`x=0` 时结果为零 |
| 删除最低位的 1 | `x &= x - 1` | 要求 `x!=0` |
| 是否只有一个 1 | `x && !(x & (x - 1))` | 即非零的 2 的幂 |
| `a` 是否为 `b` 的子集掩码 | `(a & b) == a` | 等价于 `(a & ~b)==0` |
| 保留低 `k` 位 | `x & ((1ULL << k) - 1)` | `k=64` 时不能这样移位，需特判 |
| 无符号模 `2^k` | `x & ((1ULL << k) - 1)` | 仅适用于非负位掩码语义 |
| 交换（不推荐异或交换） | `swap(a,b)` | 清晰且安全 |

位编号约定：**第 0 位是最低位（最右边）**。例如 `13=(1101)₂`，第 `0,2,3` 位为 1。

### 1.2 GNU 内建函数速查

以下函数返回值通常都是 `int`。无后缀处理 `unsigned int`（典型 32 位），`ll` 后缀处理 `unsigned long long`（典型 64 位）。

| 功能 | 32 位 | 64 位 | `x=0` |
|---|---|---|---|
| 1 的个数 | `__builtin_popcount(x)` | `__builtin_popcountll(x)` | 返回 0 |
| 1 的个数奇偶性 | `__builtin_parity(x)` | `__builtin_parityll(x)` | 返回 0 |
| 前导零个数 | `__builtin_clz(x)` | `__builtin_clzll(x)` | **未定义** |
| 尾部零个数 | `__builtin_ctz(x)` | `__builtin_ctzll(x)` | **未定义** |
| 最低 1 的位置加一 | `__builtin_ffs(x)` | `__builtin_ffsll(x)` | 返回 0 |

### 1.3 `bitset` 速查

```cpp
bitset<8> a;            // 00000000
bitset<8> b(0b1011);    // 00001011
bitset<8> c("11001010");
b.set(6);               // 第 6 位置 1：01001011
b.reset(0);             // 第 0 位置 0
b.flip(1);              // 翻转第 1 位
cout << b << '\n';      // 从最高位打印到最低位
```

```cpp
a.count(); a.any(); a.none(); a.all(); a.size();
a & b; a | b; a ^ b; ~a; a << k; a >> k;
a &= b; a |= b; a ^= b; a <<= k; a >>= k;
```

---

## 2. 基础位运算与优先级

### 2.1 运算含义

```cpp
// a = 1100b, b = 1010b
a & b   // 1000b：对应位都为 1 才为 1
a | b   // 1110b：至少一个为 1 就为 1
a ^ b   // 0110b：对应位不同则为 1
~a      // 按整个类型位宽逐位取反，不只是低 4 位
```

异或的重要性质：

```cpp
a ^ a == 0;
a ^ 0 == a;
a ^ b ^ a == b; // 异或满足交换律、结合律，可逐项抵消
```

### 2.2 运算符优先级：位运算表达式尽量加括号

```cpp
// 推荐
if ((x & mask) == mask) { }
int bit = (x >> k) & 1;
if (a ^ b) { }

// 不建议：==、!= 的优先级高于 &，容易误写
if (x & mask == mask) { } // 实际类似 x & (mask == mask)
```

`<<`、`>>` 的优先级低于 `+`、`-`：

```cpp
1ULL << k + 1   // 相当于 1ULL << (k+1)，不一定是想要的结果
1ULL << (k + 1) // 明确推荐
```

### 2.3 布尔逻辑与位逻辑不要混淆

```cpp
&&  ||  !   // 布尔运算，&&/|| 有短路求值
&   |   ~   // 对整数的每一位进行运算
```

`~x` 不是 `!x`：`~0` 的全部位都是 1，而 `!0` 为 `true`。

---

## 3. GNU / Clang 内建位处理函数

以下内建函数使用 `<bits/stdc++.h>` 后即可在 GNU C++ 中调用，无需专门的 `std` 命名空间。

### 3.1 统计 1 的个数与奇偶性

```cpp
unsigned x = 0b101101;
__builtin_popcount(x);    // 4
__builtin_parity(x);      // 0，因为 4 是偶数

unsigned long long y = 1234567890123ULL;
__builtin_popcountll(y);  // y 中 1 的个数
__builtin_parityll(y);    // 等价于 1 的个数 % 2
```

`__builtin_parity` 返回 `0/1`，并非返回“是否有奇数个位”的字符串或其他编码。

### 3.2 前导零：`clz`

```cpp
unsigned x = 1; // 按 unsigned int 的 32 位理解
__builtin_clz(x);       // 31
__builtin_clz(16u);     // 27，因为 16=2^4
```

最高位下标（非零时）：

```cpp
int msb32(unsigned x) { return 31 - __builtin_clz(x); }
int msb64(unsigned long long x) { return 63 - __builtin_clzll(x); }
```

二进制有效位宽（`0` 时定义为 0）：

```cpp
int width32(unsigned x) { return x ? 32 - __builtin_clz(x) : 0; }
int width64(unsigned long long x) {
    return x ? 64 - __builtin_clzll(x) : 0;
}
```

A 题线性基中的：

```cpp
int k = 31 - __builtin_clz((unsigned)x);
```

就是求 `x` 的最高 1 位下标。该语句位于 `while(x)` 内，因此不会把零传给 `clz`。

### 3.3 尾部零：`ctz`

```cpp
__builtin_ctz(1u);   // 0
__builtin_ctz(8u);   // 3
__builtin_ctz(12u);  // 2，因为 12=(1100)₂
```

应用：求最低 1 的下标，或整数含因子 2 的次数。

```cpp
int lowestBitIndex(unsigned long long x) { // 必须 x!=0
    return __builtin_ctzll(x);
}
```

### 3.4 `ffs`：最低 1 的位置加一

```cpp
__builtin_ffs(0);   // 0，零输入有定义
__builtin_ffs(1);   // 1
__builtin_ffs(8);   // 4
__builtin_ffs(12);  // 3
```

因此：

```cpp
// 非零时，最低 1 的零基下标：
int k = __builtin_ffsll((long long)x) - 1;
```

`ffs` 可直接用于“找一个仍剩余的元素”：

```cpp
while (mask) {
    int k = __builtin_ffsll(mask) - 1;
    mask &= mask - 1;
    // 处理第 k 个元素
}
```

### 3.5 后缀不是性能参数，而是类型参数

| 后缀 | 对应类型 | 常见位宽 |
|---|---|---|
| 无 | `unsigned int` | 32 |
| `l` | `unsigned long` | 依平台，Windows 常为 32，Linux x86-64 常为 64 |
| `ll` | `unsigned long long` | 64 |

`ffs/clrsb` 对应有符号类型，其余上述函数对应无符号类型。**处理 `long long` 时通常明确使用 `ll` 版本，避免依赖 `long` 的平台宽度。**

```cpp
long long x;
__builtin_popcount(x);   // 可能先窄化为 unsigned int，丢失高位，错误
__builtin_popcountll(x); // 正确选择 64 位版本
```

### 3.6 较少用：冗余符号位与字节翻转

```cpp
__builtin_clrsb(x);    // 最高符号位之后，与其相同的连续位数
__builtin_clrsbl(x);
__builtin_clrsbll(x);

__builtin_bswap16(x);  // 翻转字节顺序，不是翻转每一位
__builtin_bswap32(x);
__builtin_bswap64(x);
```

---

## 4. C++17 推荐封装与常用模板

### 4.1 统一使用 64 位的安全小工具

```cpp
using u64 = unsigned long long;
using u32 = unsigned;

int popcnt(u64 x) { return __builtin_popcountll(x); }
int parity(u64 x) { return __builtin_parityll(x); }

int msb(u64 x) { // 最高 1 的下标，x 必须非零
    return 63 - __builtin_clzll(x);
}
int lsb(u64 x) { // 最低 1 的下标，x 必须非零
    return __builtin_ctzll(x);
}
int bitWidth(u64 x) { // x=0 时返回 0
    return x ? 64 - __builtin_clzll(x) : 0;
}
int floorLog2(u64 x) { // x 必须非零
    return msb(x);
}
u64 lowbit(u64 x) { return x & (u64(0) - x); }
```

**不要在通用模板里固定写 `31-clz` 或 `63-clzll`**，因为结果取决于实际传入类型和转换后的类型。

### 4.2 2 的幂、上取整与下取整

```cpp
bool isPowerOfTwo(u64 x) { return x && !(x & (x - 1)); }

u64 bitFloor(u64 x) { // 不超过 x 的最大 2 的幂；x=0 返回 0
    return x ? 1ULL << msb(x) : 0;
}

u64 bitCeil(u64 x) { // 不小于 x 的最小 2 的幂
    if (x <= 1) return 1;
    return 1ULL << (64 - __builtin_clzll(x - 1));
}
```

`bitCeil` 调用前应保证答案可表示；当 `x>2^63`（即 `x>(1ULL<<63)`）时，下一种 2 的幂已不能用 `u64` 表示，示例中的左移 64 位是未定义行为。

### 4.3 lowbit：树状数组与逐位删除

```cpp
// 树状数组：p += lowbit(p) / p -= lowbit(p)
for (int i = x; i <= n; i += i & -i) bit[i] += value;

// 逐个取出 mask 中的 1
for (u64 t = mask; t; t &= t - 1) {
    int k = __builtin_ctzll(t);
    // 处理 k
}
```

### 4.4 枚举固定大小的所有子集

```cpp
for (int mask = 0; mask < (1 << n); ++mask) {
    // 只有 n 较小、且 1<<n 不溢出时使用
}
```

64 位掩码枚举范围必须确保可表示，通常竞赛位掩码 DP 的 `n` 不超过约 20～25，而不是能存 64 位就真的枚举 `2^64` 个状态。

### 4.5 枚举一个掩码的全部非空子集

```cpp
for (u64 sub = mask; sub; sub = (sub - 1) & mask) {
    // sub 按递减数值访问 mask 的所有非空子集
}
// 如需包含空集，在循环外单独处理 sub=0。
```

枚举所有 `n` 位掩码及其所有子集的总次数为 `3^n`：每一位有三种状态——父掩码为 0、父掩码为 1 但子集不选、父掩码和子集都为 1。

### 4.6 枚举恰好含 k 个 1 的掩码（Gosper's hack）

```cpp
// 枚举 [0,2^n) 中所有含 k 个 1 的数，要求 0<k<n。
u64 x = (1ULL << k) - 1;
u64 limit = 1ULL << n;
while (x < limit) {
    // 处理 x
    u64 c = x & -x;
    u64 r = x + c;
    x = (((r ^ x) >> 2) / c) | r;
}
```

### 4.7 SOS DP：求每个掩码的所有子集和

```cpp
// f[mask] 初始只保存 mask 本身的值。
for (int b = 0; b < n; ++b)
    for (int mask = 0; mask < (1 << n); ++mask)
        if (mask & (1 << b))
            f[mask] += f[mask ^ (1 << b)];
// 最后 f[mask] = 所有 sub⊆mask 的初始 f[sub] 之和。
```

超集和可以把下标取反，或改为从未包含该位的状态转移；写之前先明确“子集和”还是“超集和”。

---

## 5. `std::bitset<N>` 完整速查

### 5.1 特点与声明

```cpp
#include <bitset>
using std::bitset;

const int N = 100005;
bitset<N> bs;
```

- 固定保存 `N` 位，`N` 必须是编译期常量；
- 默认所有位为 0；
- 下标从 0 到 `N-1`，第 0 位是最低位；
- 支持整体与、或、异或、取反和移位；
- 常见实现按机器字压缩存储，整体操作通常约 `O(N/64)`，不是严格 `O(1)`；
- 位数需要运行时变化时，可用 `vector<uint64_t>` 手写动态位集，或环境允许时使用 `boost::dynamic_bitset`。

### 5.2 构造与字符串方向

```cpp
bitset<8> a;                 // 00000000
bitset<8> b(0b1011);         // 00001011
bitset<8> c(string("1011")); // 00001011
bitset<8> d("1100");         // 00001100

cout << b.to_string() << '\n'; // 00001011
cout << b << '\n';             // 00001011
cout << b.to_ullong() << '\n'; // 11
```

字符串按普通二进制书写，**最左字符对应较高位**；`operator[]` 的下标则从最右边的第 0 位开始：

```cpp
bitset<8> x("10000001");
x[0] == 1;
x[7] == 1;
x[1] == 0;
```

### 5.3 单个位的读写与判断

| 写法 | 功能 | 越界行为 |
|---|---|---|
| `x.set()` | 全部置 1 | 不适用 |
| `x.set(i)` | 第 `i` 位置 1 | 抛 `out_of_range` |
| `x.set(i,v)` | 第 `i` 位设为 `v` | 抛 `out_of_range` |
| `x.reset()` | 全部置 0 | 不适用 |
| `x.reset(i)` | 第 `i` 位置 0 | 抛 `out_of_range` |
| `x.flip()` | 全部翻转 | 不适用 |
| `x.flip(i)` | 翻转第 `i` 位 | 抛 `out_of_range` |
| `x[i]` | 读取或修改第 `i` 位 | 不检查，调用者保证合法 |
| `x.test(i)` | 读取第 `i` 位 | 抛 `out_of_range` |

```cpp
x.set(3);         // 第 3 位 = 1
x.set(3, false);  // 第 3 位 = 0
bool v = x[3];    // 已知下标合法时最方便
```

非 const 的 `x[i]` 返回代理引用，不等同于普通 `bool&`，但日常读写可直接使用。

### 5.4 查询整体状态

| 写法 | 含义 |
|---|---|
| `x.count()` | 1 的个数，返回 `size_t` |
| `x.size()` | 总位数，即模板参数 `N` |
| `x.any()` | 是否至少有一个 1 |
| `x.none()` | 是否全为 0 |
| `x.all()` | 是否全为 1 |
| `x==y`、`x!=y` | 整体比较 |

### 5.5 集合运算与移位

```cpp
auto intersection = a & b;
auto Union        = a | b;
auto symmetric    = a ^ b;
auto complement   = ~a;

a &= b;
a |= b;
a ^= b;

auto left  = a << k; // 位下标增大，低位补 0
auto right = a >> k; // 位下标减小，高位补 0
a <<= k;
a >>= k;
```

`bitset<8>("00000001") << 3` 得到 `"00001000"`。移位超过位数时，所有位移出后得到零，不会出现整数移位那种“位移量等于位宽”的未定义行为。

`~a` 会翻转**全部 N 位**，包括未使用的高位；若只处理低 `n` 位，应使用有效位掩码：

```cpp
const int n = 10;
bitset<100> a, valid;
for (int i = 0; i < n; ++i) valid.set(i);
a = (~a) & valid; // 只对低 n 位取反
```

### 5.6 转换为整数或字符串

```cpp
string s = a.to_string(); // 长度恰为 N，高位到低位
unsigned long x = a.to_ulong();
unsigned long long y = a.to_ullong();
```

若存在无法放入目标整数的高位 1，转换会抛出 `std::overflow_error`。`N>64` 时不要假设 `to_ullong()` 可以无损转换。

### 5.7 输入输出

```cpp
bitset<8> a;
cin >> a;  // 读取 0/1 字符串，最左字符对应高位
cout << a; // 输出完整 N 位
```

### 5.8 标准 `bitset` 没有直接提供什么

C++17 的 `std::bitset` **没有**以下通用成员：

```cpp
a.find_first();       // 没有
a.find_next(pos);     // 没有
a._Find_first();      // 某些实现可能有内部扩展，不应作为可移植代码依赖
a._Find_next(pos);    // 同上
```

需要遍历 1 位时：

1. 位数小：直接 `for(int i=0;i<N;++i) if(a[i]) ...`；
2. 位数大且只遍历 1：用 `vector<uint64_t>` 分块，再用 `ctzll`；
3. 能使用 Boost：`boost::dynamic_bitset` 提供 `find_first/find_next`。

```cpp
// 标准 bitset 的简单遍历：O(N)
for (int i = 0; i < N; ++i)
    if (a[i]) { /* 处理 i */ }
```

---

## 6. C++20 `<bit>`：如果评测环境支持

头文件为 `<bit>`，函数位于 `std`。这些函数主要接受无符号整数类型；不是 `__builtin` 的简单换名，而是标准化接口。

| 函数 | 功能 | 特别说明 |
|---|---|---|
| `std::popcount(x)` | 1 的个数 | `x=0` 返回 0 |
| `std::countl_zero(x)` | 前导零个数 | `x=0` 返回类型位宽 |
| `std::countr_zero(x)` | 尾部零个数 | `x=0` 返回类型位宽 |
| `std::countl_one(x)` | 前导一个数 | 全 1 返回类型位宽 |
| `std::countr_one(x)` | 尾部一个数 | 全 1 返回类型位宽 |
| `std::has_single_bit(x)` | 是否恰有一个 1 | 等价于非零的 2 的幂 |
| `std::bit_floor(x)` | 不超过 `x` 的最大 2 的幂 | `x=0` 返回 0 |
| `std::bit_ceil(x)` | 不小于 `x` 的最小 2 的幂 | 结果不可表示时为未定义行为 |
| `std::bit_width(x)` | 表示 `x` 所需位数 | `x=0` 返回 0 |
| `std::rotl(x,s)` | 循环左移 | 移出的位从另一端回来 |
| `std::rotr(x,s)` | 循环右移 | 不是普通移位 |

```cpp
#include <bit>

unsigned x = 13;
int a = std::popcount(x);           // 3
int b = std::countr_zero(x);        // 0
int c = std::bit_width(x);          // 4
unsigned d = std::bit_floor(x);     // 8
unsigned e = std::bit_ceil(x);      // 16
int top = std::bit_width(x) - 1;    // x!=0 时最高位下标
```

C++20 计数接口对零输入有定义，这一点比 `clz/ctz` 安全，但仍需注意模板实参的实际类型位宽。

### 6.1 记忆命名：`l/r` 指方向，`zero/one` 指数的内容

- `countl_zero`：从左边（高位）数连续 0；
- `countr_zero`：从右边（低位）数连续 0；
- `countl_one`：从左边数连续 1；
- `countr_one`：从右边数连续 1。

标准 `clz/ctz` 风格名称不存在于 `std`；应写 `std::countl_zero`、`std::countr_zero`。另外，`std::popcount` 不是 `__builtin_popcount` 的同义宏，它需要 C++20 和 `<bit>`。

---

## 7. `bitset` 常见竞赛应用

### 7.1 位并行背包：可达性 DP

每种物品只有一个，重量为 `w`：

```cpp
const int MAXS = 100000;
bitset<MAXS + 1> dp;
dp[0] = 1;
for (int w : weights)
    dp |= dp << w;
// dp[s] 表示能否凑出总和 s
```

`dp<<w` 表示所有旧可达重量同时增加 `w`。`dp |= dp<<w` 的右侧基于修改前的旧值，所以恰好是 0/1 背包，不会在同一物品内重复使用。

无限背包则不是简单写成同一个式子，需要按余数等额外方法处理。

### 7.2 带删边的图连通性或批量转移

邻接矩阵可用 `bitset<N> adj[N]` 保存：

```cpp
adj[u][v] = adj[v][u] = 1;
auto commonNeighbors = adj[u] & adj[v];
int cnt = (int)commonNeighbors.count();
```

若状态由“某些集合的并、交、差”反复构成，使用位集通常比逐点遍历快很多，但仍要注意 `O(N²/64)` 的总复杂度与内存。

### 7.3 用位移表示状态传播

```cpp
bitset<MAXN> cur, nxt;
nxt = (cur << 1) | (cur >> 1); // 一步向相邻位置传播
nxt &= valid;                  // 清除非法位置
```

适合细胞自动机、棋盘行状态、简单图层的批量传播等。

---

## 8. 未定义行为与常见错误清单

### 8.1 零输入

```cpp
__builtin_clz(0);      // 未定义
__builtin_ctz(0);      // 未定义
__builtin_clzll(0);    // 未定义
__builtin_ctzll(0);    // 未定义
```

在调用前写 `if(x)`、`assert(x)`，或封装 `bitWidth` 等带零处理的接口。

### 8.2 位移量与溢出

```cpp
1 << 40       // int 通常只有 32 位，错误
1LL << 40     // 可表示时为 2^40
1ULL << 63    // 正确的 64 位无符号掩码

x << -1       // 未定义
x >> 64       // 对 64 位整数未定义，不会可靠地返回 0
```

对无符号数，左移相当于模 `2^N` 的位运算；对于有符号数，负值左移、结果不可表示的左移可能产生未定义行为。位掩码代码优先使用无符号类型。

### 8.3 有符号右移与 `~`

负数右移是否算术填充由实现定义；不要把负数当成纯位掩码。`~1` 通常等于 `-2`，不是“除了第 0 位外的小整数全集”。

### 8.4 把集合值当成布尔值

```cpp
if (x & (1ULL << k)) { }       // 正确：非零即该位为 1
if ((x & (1ULL << k)) == 1) { }// 仅 k=0 才成立，通常错误
```

### 8.5 `bitset` 常见坑

- `N` 必须是编译期常量，不能直接写 `bitset<n>`（`n` 为运行时变量）；
- `bitset<N>` 下标方向与输出字符串方向相反；
- `~x` 会翻转所有 `N` 位；
- `x<<k` 向左移时低位补零，向右移时高位补零，不是循环移位；
- `to_ullong` 遇到放不下的高位会抛异常；
- `bitset` 不能直接调用整数版 `__builtin_popcount`；
- 不要误以为整个 `bitset` 的操作永远是 `O(1)`。

---

## 9. 考场最小模板

```cpp
#include <bits/stdc++.h>
using namespace std;
using u64 = unsigned long long;

int popcnt(u64 x) { return __builtin_popcountll(x); }
int msb(u64 x) { return 63 - __builtin_clzll(x); } // x!=0
int lsb(u64 x) { return __builtin_ctzll(x); }      // x!=0
int width(u64 x) { return x ? 64 - __builtin_clzll(x) : 0; }

// 遍历所有置位
for (u64 t = mask; t; t &= t - 1) {
    int k = __builtin_ctzll(t);
}

// 遍历所有非空子集
for (u64 s = mask; s; s = (s - 1) & mask) {
}

// bitset 可达性 DP
const int N = 100000;
bitset<N + 1> dp;
dp[0] = 1;
// dp |= dp << w;
```

---

## 10. 参考资料

- [GCC：Bit Operation Builtins](https://gcc.gnu.org/onlinedocs/gcc/Bit-Operation-Builtins.html)
- [C++ Reference：`<bit>`](https://en.cppreference.com/w/cpp/header/bit.html)
- [C++ Reference：`std::bitset`](https://en.cppreference.com/w/cpp/utility/bitset.html)
