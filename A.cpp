#include <bits/stdc++.h>
using namespace std;

// 二进制异或线性基：把整数看作 F_2^m 上的向量。
// b[k] 保存最高位为 k 的基向量；插入返回“是否使线性空间的秩增加”。
struct Basis {
    array<int, 20> b{}; // m <= 20，每一位至多保存一个向量。
    int rank = 0;
    bool insert(int x) {
        while (x) {
            // 取 x 的最高二进制位，用已有的同位基向量继续消元。
            int k = 31 - __builtin_clz((unsigned)x);
            if (b[k]) x ^= b[k];
            else { b[k] = x; ++rank; return true; }
        }
        return false; // x 可由已有基向量异或得到，不是独立向量。
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, m;
    cin >> n >> m;
    vector<vector<int>> sets(n); // 每个集合的原始元素。
    vector<vector<int>> span(n); // 显式保存尚未被吸收的局部线性空间。
    vector<Basis> local(n);      // local[i] 张成 span(sets[i] ∪ V)。
    vector<int> frequency(1 << m); // 每个数值在多少个原始集合中出现。
    vector<int> mark(1 << m, -1);  // 时间戳式标记：数值是否属于当前集合。
    for (int i = 0; i < n; ++i) {
        int c; cin >> c;
        sets[i].resize(c);
        for (int &x : sets[i]) {
            cin >> x;
            ++frequency[x];
            local[i].insert(x);
        }
    }
    Basis common; // V：已经证明必须包含在每个最终线性空间中的公共部分。
    vector<int> events; // 待传播到各局部空间的独立公共向量。
    vector<int> active; // 仍需显式维护元素列表的集合下标。
    auto force = [&](int x) {
        // 只有秩增加才需要传播；否则 x 已可由 V 中的向量异或得到。
        if (common.insert(x)) events.push_back(x);
    };
    auto absorb = [&](int i) {
        // 将整个局部空间的基并入公共空间，原集合以后不再产生新限制。
        for (int x : local[i].b) if (x) force(x);
        vector<int>().swap(span[i]); // 释放不再需要的显式空间。
    };
    for (int i = 0; i < n && common.rank < m; ++i) {
        // 若 |U_i| > 2|S_i|，缺失元素超过一半，必能张成整个 U_i；
        // 因而 U_i 的全部基向量都是强制元素，可直接吸收该集合。
        if ((1 << local[i].rank) > 2 * (int)sets[i].size()) {
            absorb(i);
            continue;
        }
        for (int x : sets[i]) mark[x] = i;
        auto &v = span[i];
        // 从小空间开始逐次加入基向量：若已有 W，加入 x 后得到 W ∪ (W⊕x)。
        // 初始为 {0}，也保证零元素不会因空集合或全被吸收而被遗漏。
        v.reserve(1 << local[i].rank);
        v.push_back(0);
        for (int x : local[i].b) if (x) {
            int s = (int)v.size();
            for (int j = 0; j < s; ++j) v.push_back(v[j] ^ x);
        }
        // 最终 S_i∪X 必须异或封闭，所以 U_i\S_i 中的值都必须插入。
        for (int x : v) if (mark[x] != i) force(x);
        active.push_back(i);
    }
    // 新公共向量可能使局部空间产生新的强制元素，而这些元素又可能扩张 V。
    // events 在处理中会增长，因此此循环实际上执行到不动点为止。
    for (size_t e = 0; e < events.size() && common.rank < m; ++e) {
        int x = events[e];
        for (size_t j = 0; j < active.size() && common.rank < m;) {
            int i = active[j];
            if (!local[i].insert(x)) { ++j; continue; }
            if ((1 << local[i].rank) > 2 * (int)sets[i].size()) {
                absorb(i);
                active[j] = active.back();
                active.pop_back();
                continue;
            }
            for (int y : sets[i]) mark[y] = i;
            auto &v = span[i];
            int s = (int)v.size();
            v.reserve(s * 2);
            // 新增独立向量 x 时，只需追加陪集 U_i⊕x；原空间中的元素不变。
            for (int k = 0; k < s; ++k) {
                int y = v[k] ^ x;
                v.push_back(y);
                if (mark[y] != i) force(y);
            }
            ++j;
        }
    }
    // 展开公共线性基，枚举 V 的全部元素，包括零向量。
    vector<int> v{0};
    v.reserve(1 << common.rank);
    for (int x : common.b) if (x) {
        int s = (int)v.size();
        for (int j = 0; j < s; ++j) v.push_back(v[j] ^ x);
    }
    // x 若未出现在所有原集合中，就必须补入至少一个缺少它的集合；
    // 若本来就在所有集合中，则不必重复插入，故不输出。
    vector<int> answer;
    for (int x : v) if (frequency[x] != n) answer.push_back(x);
    cout << answer.size() << '\n';
    for (int x : answer) cout << x << ' ';
    cout << '\n';
}
