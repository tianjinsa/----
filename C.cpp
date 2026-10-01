#include <bits/stdc++.h>
using namespace std;

struct FastInput {
    static constexpr int S = 1 << 16;
    char buf[S]; int at = 0, len = 0;
    int get() {
        if (at == len) { len = (int)fread(buf, 1, S, stdin); at = 0; }
        return len ? buf[at++] : EOF;
    }
    int read() {
        int c, x = 0;
        do { c = get(); } while (c <= ' ' && c != EOF);
        while (c >= '0' && c <= '9') { x = x * 10 + c - '0'; c = get(); }
        return x;
    }
};

struct Fenwick {
    int n;
    vector<int> bit;
    explicit Fenwick(const vector<int> &values) : n((int)values.size() - 1), bit(values) {
        for (int i = 1; i <= n; ++i) if (i + (i & -i) <= n) bit[i + (i & -i)] += bit[i];
    }
    void add(int p, int x) { for (; p <= n; p += p & -p) bit[p] += x; }
    int prefix(int p) const { int s = 0; for (; p; p -= p & -p) s += bit[p]; return s; }
};

struct Query { int l, r, f, id; };

int main() {
    FastInput in;
    int T = in.read();
    string output;
    while (T--) {
        int n = in.read(), m = n - 1;
        vector<int> eu(n), ev(n);
        vector<vector<pair<int, int>>> graph(n + 1);
        for (int i = 1; i <= m; ++i) {
            eu[i] = in.read(); ev[i] = in.read();
            graph[eu[i]].emplace_back(ev[i], i);
            graph[ev[i]].emplace_back(eu[i], i);
        }
        vector<int> weight(n + 1);
        int vertexMax = 0;
        for (int i = 1; i <= n; ++i) vertexMax = max(vertexMax, weight[i] = in.read());
        vector<int> parent(n + 1), tin(n + 1), tout(n + 1), child(n), order, stack{1};
        order.reserve(n);
        while (!stack.empty()) {
            int u = stack.back(); stack.pop_back();
            tin[u] = (int)order.size() + 1; order.push_back(u);
            for (auto [v, e] : graph[u]) if (v != parent[u]) {
                parent[v] = u; child[e] = v; stack.push_back(v);
            }
        }
        vector<int> subtree(weight), euler(n + 1), subtreeSize(n + 1, 1);
        for (int i = n - 1; i >= 0; --i) {
            int u = order[i];
            tout[u] = tin[u] + subtreeSize[u] - 1;
            euler[tin[u]] = weight[u];
            if (parent[u]) {
                subtree[parent[u]] += subtree[u];
                subtreeSize[parent[u]] += subtreeSize[u];
            }
        }
        int q = in.read();
        int block = max(1, min(m, (int)(2.2 * n / sqrt((double)q))));
        vector<vector<Query>> groups((m + block - 1) / block);
        for (int id = 0; id < q; ++id) {
            int l = in.read(), r = in.read(), f = in.read();
            groups[(l - 1) / block].push_back({l, r, f, id});
        }
        vector<int> answer(q), tp(n + 1), ts(n + 1), side0(n + 1), side1(n + 1), stamp(n + 1);
        int epoch = 0;
        auto tempFind = [&](int x) {
            while (tp[x] != x) { tp[x] = tp[tp[x]]; x = tp[x]; }
            return x;
        };
        for (int g = 0; g < (int)groups.size(); ++g) {
            auto &queries = groups[g];
            if (queries.empty()) continue;
            int boundary = min(m, (g + 1) * block);
            sort(queries.begin(), queries.end(), [](const Query &a, const Query &b) { return a.r < b.r; });
            vector<int> dsu(n + 1), size(n + 1, 1), sum(weight), top(n + 1);
            iota(dsu.begin(), dsu.end(), 0); iota(top.begin(), top.end(), 0);
            auto find = [&](int x) {
                while (dsu[x] != x) { dsu[x] = dsu[dsu[x]]; x = dsu[x]; }
                return x;
            };
            Fenwick bit(euler);
            vector<pair<int, int>> initial;
            initial.reserve(2 * n);
            for (int i = 1; i <= n; ++i) initial.emplace_back(weight[i], i);
            priority_queue<pair<int, int>> maximum(less<pair<int, int>>(), move(initial));
            auto clean = [&]() {
                while (!maximum.empty()) {
                    auto [w, u] = maximum.top();
                    if (dsu[u] == u && sum[u] == w) break;
                    maximum.pop();
                }
            };
            int right = boundary;
            for (const Query &query : queries) {
                while (right < query.r) {
                    int v = child[++right];
                    int a = find(parent[v]), b = find(v);
                    int upper = top[a];
                    bit.add(tin[upper], sum[b]);
                    bit.add(tin[top[b]], -sum[b]);
                    if (size[a] < size[b]) swap(a, b);
                    dsu[b] = a; size[a] += size[b]; sum[a] += sum[b]; top[a] = upper;
                    maximum.emplace(sum[a], a);
                }
                ++epoch;
                int special = -1, cutLeft = 0, cutRight = -1, insideWeight = 0;
                clean();
                int best = maximum.top().first;
                if (query.f > boundary) {
                    int v = child[query.f];
                    special = find(v); cutLeft = tin[v]; cutRight = tout[v];
                    insideWeight = subtree[v] - (bit.prefix(cutRight) - bit.prefix(cutLeft - 1));
                    if (maximum.top().second == special) {
                        auto keep = maximum.top(); maximum.pop(); clean();
                        best = maximum.empty() ? 0 : maximum.top().first;
                        maximum.push(keep);
                    }
                    best = max({best, insideWeight, sum[special] - insideWeight});
                }
                auto touch = [&](int a) {
                    if (stamp[a] == epoch) return;
                    stamp[a] = epoch; tp[a] = a; ts[a] = 1;
                    side1[a] = a == special ? insideWeight :
                        (cutLeft <= tin[top[a]] && tin[top[a]] <= cutRight ? sum[a] : 0);
                    side0[a] = sum[a] - side1[a];
                };
                for (int e = query.l; e <= min(boundary, query.r); ++e) if (e != query.f) {
                    int a = find(eu[e]), b = find(ev[e]);
                    touch(a); touch(b); a = tempFind(a); b = tempFind(b);
                    if (a == b) continue;
                    if (ts[a] < ts[b]) swap(a, b);
                    tp[b] = a; ts[a] += ts[b];
                    side0[a] += side0[b]; side1[a] += side1[b];
                    best = max({best, side0[a], side1[a]});
                }
                answer[query.id] = max(best, vertexMax);
            }
        }
        for (int value : answer) { output += to_string(value); output += '\n'; }
    }
    fwrite(output.data(), 1, output.size(), stdout);
}
