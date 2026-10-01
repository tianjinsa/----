#include <bits/stdc++.h>
using namespace std;
constexpr int MOD = 998244353, INV2 = 499122177, INV4 = 748683265;
constexpr int part[5][3] = {{0,0,0}, {0,0,1}, {0,1,0}, {0,1,1}, {0,1,2}};

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
    void load(int state, int offset) {
        for (int a = 0; a < 3; ++a)
            for (int b = 0; b < a; ++b)
                if (part[state][a] == part[state][b]) join(offset + a, offset + b);
    }
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
    int first[5][2], ways[5][5][5]{}, reductions[5][5]{};
    for (int b = 0; b < 5; ++b)
        for (int edge = 0; edge < 2; ++edge) {
            DSU d; d.load(b, 3);
            if (edge) d.join(0, 3);
            first[b][edge] = d.state(0, 4, 5);
        }
    for (int a = 0; a < 5; ++a)
        for (int b = 0; b < 5; ++b)
            for (int mask = 0; mask < 4; ++mask) {
                DSU d; d.load(a, 0); d.load(b, 3);
                int removed = 0;
                if (mask & 1) removed += d.join(0, 3); // Parent-child edge.
                if (mask & 2) removed += d.join(2, 4); // Consecutive leaves.
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
        vector<int> order{0};
        for (size_t i = 0; i < order.size(); ++i)
            for (int v : children[order[i]]) order.push_back(v);
        vector<array<int, 5>> dp(n);
        vector<int> expectation(n);
        for (int ii = n - 1; ii >= 0; --ii) {
            int u = order[ii];
            if (children[u].empty()) {
                dp[u][0] = 1;
                expectation[u] = 1;
                continue;
            }
            int v = children[u][0];
            for (int b = 0; b < 5; ++b) {
                int value = 1LL * dp[v][b] * INV2 % MOD;
                for (int edge = 0; edge < 2; ++edge) {
                    int &target = dp[u][first[b][edge]];
                    target += value;
                    if (target >= MOD) target -= MOD;
                }
            }
            expectation[u] = (expectation[v] + 1LL + MOD - INV2) % MOD;
            for (size_t j = 1; j < children[u].size(); ++j) {
                v = children[u][j];
                array<int, 5> next{};
                long long reduction = 0;
                for (int a = 0; a < 5; ++a) if (dp[u][a])
                    for (int b = 0; b < 5; ++b) if (dp[v][b]) {
                        long long prob = 1LL * dp[u][a] * dp[v][b] % MOD * INV4 % MOD;
                        reduction = (reduction + prob * reductions[a][b]) % MOD;
                        for (int s = 0; s < 5; ++s) if (ways[a][b][s])
                            next[s] = (next[s] + prob * ways[a][b][s]) % MOD;
                    }
                expectation[u] = (expectation[u] + 1LL * expectation[v] - reduction + MOD) % MOD;
                dp[u] = next;
            }
        }
        cout << expectation[0] << '\n';
    }
}
