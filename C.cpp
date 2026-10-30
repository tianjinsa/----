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

// 按 DFS 序维护公共连通块的权值，支持单点修改和前缀查询。
// 每个公共块的权值只存放在其深度最小顶点的 tin 位置上。
struct Fenwick {
    int n;
    vector<int> bit;
    // 由普通数组 O(n) 构造树状数组：把当前位置的值贡献给父节点。
    explicit Fenwick(const vector<int> &values) : n((int)values.size() - 1), bit(values) {
        for (int i = 1; i <= n; ++i) if (i + (i & -i) <= n) bit[i + (i & -i)] += bit[i];
    }
    void add(int p, int x) { for (; p <= n; p += p & -p) bit[p] += x; }
    int prefix(int p) const { int s = 0; for (; p; p -= p & -p) s += bit[p]; return s; }
};

// 删除 [l,r] 中的边 f，求剩余森林的最大连通块权值；id 用于恢复输出顺序。
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
        // 将原树固定在顶点 1：child[e] 是边 e 靠近叶子的端点，
        // 因而删除 e 恰好分出 child[e] 的子树和子树外部两部分。
        // 迭代 DFS 避免链状树导致递归栈溢出。
        vector<int> parent(n + 1), tin(n + 1), tout(n + 1), child(n), order, stack{1};
        order.reserve(n);
        while (!stack.empty()) {
            int u = stack.back(); stack.pop_back();
            tin[u] = (int)order.size() + 1; order.push_back(u);
            for (auto [v, e] : graph[u]) if (v != parent[u]) {
                parent[v] = u; child[e] = v; stack.push_back(v);
            }
        }
        // 子树在 DFS 序中对应连续区间 [tin,tout]；subtree 保存原子树总权值。
        vector<int> subtree(weight), euler(n + 1), subtreeSize(n + 1, 1);
        for (int i = n - 1; i >= 0; --i) {
            int u = order[i];
            tout[u] = tin[u] + subtreeSize[u] - 1;
            euler[tin[u]] = weight[u]; // 初始每个顶点自成一个公共连通块。
            if (parent[u]) {
                subtree[parent[u]] += subtree[u];
                subtreeSize[parent[u]] += subtreeSize[u];
            }
        }
        int q = in.read();
        // 按左端点 l 分块。公共部分每次扩展的代价与 q*B 的补边代价折中，
        // 块长约取 2.2*n/sqrt(q)，并限制在 [1,m]。
        int block = max(1, min(m, (int)(2.2 * n / sqrt((double)q))));
        vector<vector<Query>> groups((m + block - 1) / block);
        for (int id = 0; id < q; ++id) {
            int l = in.read(), r = in.read(), f = in.read();
            groups[(l - 1) / block].push_back({l, r, f, id});
        }
        vector<int> answer(q);
        // 临时并查集只涉及补边所触及的公共连通块，不修改公共并查集。
        // side1/side0 分别记录临时块位于被删原子树内/外的权值。
        // stamp 配合 epoch 懒初始化，避免每个询问清空 O(n) 的临时数组。
        vector<int> tp(n + 1), ts(n + 1), side0(n + 1), side1(n + 1), stamp(n + 1);
        int epoch = 0;
        auto tempFind = [&](int x) {
            while (tp[x] != x) { tp[x] = tp[tp[x]]; x = tp[x]; }
            return x;
        };
        for (int g = 0; g < (int)groups.size(); ++g) {
            auto &queries = groups[g];
            if (queries.empty()) continue;
            int boundary = min(m, (g + 1) * block); // 当前左端点块的右边界 R。
            // r 升序处理后，公共森林只加入边 [R+1,r]，不会在询问间删边。
            sort(queries.begin(), queries.end(), [](const Query &a, const Query &b) { return a.r < b.r; });
            // 公共并查集：sum 为块权值，top 为块内原树深度最小的顶点。
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
            // 大根堆记录各公共连通块权值；合并后不删除旧记录，
            // 只在取堆顶时根据“仍是根且权值未变”惰性清除失效项。
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
                // 将公共森林扩展至当前 r。加入原树父子边时，下面的块接到上面的块，
                // 合并后的最高点仍是上面的 top；只需移动下面块在树状数组中的权值。
                while (right < query.r) {
                    int v = child[++right];
                    int a = find(parent[v]), b = find(v);
                    int upper = top[a];
                    bit.add(tin[upper], sum[b]);
                    bit.add(tin[top[b]], -sum[b]);
                    if (size[a] < size[b]) swap(a, b); // 按大小合并；top 已单独保存。
                    dsu[b] = a; size[a] += size[b]; sum[a] += sum[b]; top[a] = upper;
                    maximum.emplace(sum[a], a);
                }
                ++epoch;
                int special = -1, cutLeft = 0, cutRight = -1, insideWeight = 0;
                clean();
                int best = maximum.top().first; // 未被切开的公共连通块候选。
                if (query.f > boundary) {
                    // 被删边在公共森林中，只会切开 special 这一个公共连通块。
                    // 原子树 v 的权值减去完全位于其中的其它公共块权值，
                    // 就得到 special∩subtree(v) 的权值；树状数组负责这段区间求和。
                    int v = child[query.f];
                    special = find(v); cutLeft = tin[v]; cutRight = tout[v];
                    insideWeight = subtree[v] - (bit.prefix(cutRight) - bit.prefix(cutLeft - 1));
                    if (maximum.top().second == special) {
                        // 若堆顶正是被切开的块，暂时排除它，寻找其它公共块最大值。
                        auto keep = maximum.top(); maximum.pop(); clean();
                        best = maximum.empty() ? 0 : maximum.top().first;
                        maximum.push(keep);
                    }
                    best = max({best, insideWeight, sum[special] - insideWeight});
                }
                auto touch = [&](int a) {
                    if (stamp[a] == epoch) return; // 本次询问已经初始化过。
                    stamp[a] = epoch; tp[a] = a; ts[a] = 1;
                    // special 需要按公式分成两侧；其它公共块完整地属于某一侧，
                    // 可根据其最高点 top 是否落在原子树 DFS 区间内判断。
                    side1[a] = a == special ? insideWeight :
                        (cutLeft <= tin[top[a]] && tin[top[a]] <= cutRight ? sum[a] : 0);
                    side0[a] = sum[a] - side1[a];
                };
                // 补回 [l,min(R,r)] 中至多 B 条边；若 f 在这段，直接跳过即为删除。
                // r<=R 时公共森林为空，这里也会独立重建完整查询森林。
                for (int e = query.l; e <= min(boundary, query.r); ++e) if (e != query.f) {
                    int a = find(eu[e]), b = find(ev[e]);
                    touch(a); touch(b); a = tempFind(a); b = tempFind(b);
                    if (a == b) continue;
                    if (ts[a] < ts[b]) swap(a, b);
                    tp[b] = a; ts[a] += ts[b];
                    // 临时边不跨越被删边形成的割，两侧权值可分别相加。
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
