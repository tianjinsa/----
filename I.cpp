#include <bits/stdc++.h>
using namespace std;
constexpr int MOD = 998244353, INV2 = 499122177, INV4 = 748683265;
// 三个边界顶点的划分只有五种：000、001、010、011、012。
// 相同数字表示连通，例如 010 表示第 1、3 个顶点连通，第 2 个独立。
// 边界顶点可以重合，因此“连通”只指当前已保留道路形成的连通关系。
constexpr int part[5][3] = {{0,0,0}, {0,0,1}, {0,1,0}, {0,1,1}, {0,1,2}};

// 预计算转移时使用的小并查集：左右两个图各有三个边界点，共六个。
struct DSU {
    int p[6];
    DSU() { iota(p, p + 6, 0); }
    int find(int x) { return p[x] == x ? x : p[x] = find(p[x]); }
    bool join(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        p[a] = b;
        return true;
    }
    // 将某个三边界点状态装入并查集；offset 区分左右两个图。
    void load(int state, int offset) {
        for (int a = 0; a < 3; ++a)
            for (int b = 0; b < a; ++b)
                if (part[state][a] == part[state][b]) join(offset + a, offset + b);
    }
    // 将实际并查集连通关系转换为五种规范状态编号之一。
    int state(int a, int b, int c) {
        int v[3] = {find(a), find(b), find(c)};
        for (int s = 0; s < 5; ++s) {
            bool ok = true;
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < i; ++j)
                    if ((v[i] == v[j]) != (part[s][i] == part[s][j])) ok = false;
            if (ok) return s;
        }
        abort();
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    // 三边界点统一记为 (u,L,R)：子树根、DFS 第一片叶子、最后一片叶子。
    // 后续合并时，左图边界为 (u,L_old,R_old)，右图为 (u_child,L_child,R_child)。
    // first[b][edge]：加入第一个孩子后的新状态，edge 表示父子边是否保留。
    // ways[a][b][s]：旧状态 a、新孩子状态 b 经四条边方案得到状态 s 的方案数。
    // reductions[a][b]：四条方案中，新增边成功合并连通块的次数总和。
    int first[5][2], ways[5][5][5]{}, reductions[5][5]{};
    for (int b = 0; b < 5; ++b)
        for (int edge = 0; edge < 2; ++edge) {
            DSU d; d.load(b, 3); // 孩子边界放在下标 3、4、5。
            if (edge) d.join(0, 3); // 0 是新根 u，3 是孩子根。
            // 新边界是 (u,L_child,R_child)，对应并查集下标 (0,4,5)。
            first[b][edge] = d.state(0, 4, 5);
        }
    // 预计算合并后续孩子时的全部 5×5×4 种转移。
    for (int a = 0; a < 5; ++a)
        for (int b = 0; b < 5; ++b)
            for (int mask = 0; mask < 4; ++mask) {
                DSU d; d.load(a, 0); d.load(b, 3);
                int removed = 0;
                // 跨左右图的两条随机边：父子树边、前一棵子树末叶到后一棵子树首叶的链边。
                // 只有连接原本不连通的两点时才使连通块总数减一；成环时不减少。
                if (mask & 1) removed += d.join(0, 3); // 父子边。
                if (mask & 2) removed += d.join(2, 4); // 相邻叶子边。
                // 新边界为 (u,L_old,R_child)，对应下标 (0,1,5)。
                ++ways[a][b][d.state(0, 1, 5)];
                reductions[a][b] += removed;
            }
    int T; cin >> T;
    while (T--) {
        int n; cin >> n;
        vector<vector<int>> children(n);
        for (auto &v : children) {
            int c; cin >> c; v.resize(c);
            for (int &x : v) { cin >> x; --x; }
        }
        // 先得到父亲先于孩子的顺序，再逆序计算 DP，避免链状树递归爆栈。
        vector<int> order{0};
        for (size_t i = 0; i < order.size(); ++i)
            for (int v : children[order[i]]) order.push_back(v);
        // dp[u][s]：u 子图中，三边界点连通关系为 s 的概率（模意义）。
        // expectation[u]：u 子图连通块数的无条件期望，不需要按状态分别保存。
        vector<array<int, 5>> dp(n);
        vector<int> expectation(n);
        for (int ii = n - 1; ii >= 0; --ii) {
            int u = order[ii];
            if (children[u].empty()) {
                dp[u][0] = 1; // 单个顶点的三个边界点重合，状态 000 概率为 1。
                expectation[u] = 1; // 一个孤立顶点恰有一个连通块。
                continue;
            }
            // 第一棵孩子子树之外新增独立根 u，连通块数先加 1；
            // 父子边以 1/2 概率保留，期望再减 1/2。
            int v = children[u][0];
            for (int b = 0; b < 5; ++b) {
                int value = 1LL * dp[v][b] * INV2 % MOD; // 每种边方案的概率均为 1/2。
                for (int edge = 0; edge < 2; ++edge) {
                    int &target = dp[u][first[b][edge]];
                    target += value;
                    if (target >= MOD) target -= MOD;
                }
            }
            expectation[u] = (expectation[v] + 1LL + MOD - INV2) % MOD;
            // 依次并入后续孩子。合并前左右子图独立且不相交，所以概率可相乘，
            // 连通块数相加后，只需减去两条新边实际成功合并的次数。
            for (size_t j = 1; j < children[u].size(); ++j) {
                v = children[u][j];
                array<int, 5> next{};
                long long reduction = 0; // 新边减少连通块数的期望。
                for (int a = 0; a < 5; ++a) if (dp[u][a])
                    for (int b = 0; b < 5; ++b) if (dp[v][b]) {
                        // 两条边各以 1/2 概率保留，每个固定 mask 的概率为 1/4。
                        long long prob = 1LL * dp[u][a] * dp[v][b] % MOD * INV4 % MOD;
                        reduction = (reduction + prob * reductions[a][b]) % MOD;
                        // ways 已汇总产生同一新状态的边方案数。
                        for (int s = 0; s < 5; ++s) if (ways[a][b][s])
                            next[s] = (next[s] + prob * ways[a][b][s]) % MOD;
                    }
                // 线性期望直接合并即可，无需各状态下的条件期望。
                expectation[u] = (expectation[u] + 1LL * expectation[v] - reduction + MOD) % MOD;
                dp[u] = next;
            }
        }
        cout << expectation[0] << '\n';
    }
}
