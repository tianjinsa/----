#include <bits/stdc++.h>
using namespace std;

// 用左偏树维护每个顶点的孩子集合，堆键为孩子子树的高度。
// 每个原顶点本身就是一枚位于“父亲的孩子堆”中的结点；
// head[v] 则是另一棵堆的根，里面装着 v 的孩子，两者不能混淆。
struct LeftistHeap {
    vector<int> left, right, dist; // 左偏树的左右孩子及空路径长度。
    vector<int> height, head;      // 子树高度、每个顶点的孩子堆堆顶。
    explicit LeftistHeap(int n) : left(n + 1), right(n + 1), dist(n + 1, 1),
                                  height(n + 1), head(n + 1) { dist[0] = 0; }
    // 大根左偏树合并：优先沿右链递归，使深度保持对数级。
    int merge(int a, int b) {
        if (!a || !b) return a | b;
        if (height[a] < height[b]) swap(a, b);
        right[a] = merge(right[a], b);
        if (dist[left[a]] < dist[right[a]]) swap(left[a], right[a]);
        dist[a] = dist[right[a]] + 1;
        return a;
    }
    // 取出 u 的高度最大的孩子，并将这个孩子从 u 的堆中删除。
    // 清空该孩子自身的堆指针，便于之后作为新结点重新合并。
    int pop_child(int u) {
        int v = head[u];
        head[u] = merge(left[v], right[v]);
        left[v] = right[v] = 0;
        dist[v] = 1;
        return v;
    }
    // 有孩子时，最高孩子决定子树高度；否则 u 是叶子，高度为零。
    void update(int u) { height[u] = head[u] ? height[head[u]] + 1 : 0; }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T; cin >> T;
    while (T--) {
        int n; cin >> n;
        vector<vector<int>> graph(n + 1);
        for (int i = 1; i < n; ++i) {
            int u, v; cin >> u >> v;
            graph[u].push_back(v); graph[v].push_back(u);
        }
        // 先从顶点 1 遍历一次。树中离 1 最远的顶点必是某条直径的端点。
        // 记录父节点以避免沿树边回头，order 同时保存遍历顺序。
        vector<int> parent(n + 1, -1), order{1};
        parent[1] = 0;
        for (size_t i = 0; i < order.size(); ++i)
            for (int v : graph[order[i]]) if (v != parent[order[i]]) {
                parent[v] = order[i]; order.push_back(v);
            }
        int root = order.back();
        // 固定这个直径端点为根；每次沿从它出发的直径向它折叠。
        // 该策略下 root 始终保持为当前树的直径端点，操作次数就是最大值。
        order.assign(1, root); parent[root] = 0;
        for (size_t i = 0; i < order.size(); ++i)
            for (int v : graph[order[i]]) if (v != parent[order[i]]) {
                parent[v] = order[i]; order.push_back(v);
            }
        LeftistHeap heap(n);
        // 按孩子先于父亲的顺序，先计算高度，再将顶点加入父亲的孩子堆。
        for (int i = n - 1; i >= 0; --i) {
            int u = order[i];
            heap.update(u);
            if (parent[u]) heap.head[parent[u]] = heap.merge(heap.head[parent[u]], u);
        }
        int answer = 0;
        vector<int> path; path.reserve(n);
        while (heap.head[root]) { // root 已无孩子时，整棵树只剩一个顶点。
            path.clear();
            int u = root;
            path.push_back(u);
            // 每次选择高度最大的孩子，延伸出从 root 出发的一条当前直径。
            // 沿途把路径点弹出父亲堆，使路径点剩下的堆只包含直径外的旁支。
            while (heap.head[u]) {
                u = heap.pop_child(u);
                path.push_back(u);
            }
            int D = (int)path.size() - 1, middle = D / 2;
            // 将直径上对称位置的 path[i] 与 path[D-i] 合并。
            // 保留靠近 root 的 path[i]，并把对称点的所有旁支挂到它下面。
            // 路径边已从堆中弹出，所以这里只合并旁支，不会把删去的半边接回来。
            for (int i = 0; i <= middle; ++i) {
                int a = path[i], b = path[D - i];
                if (a != b) {
                    heap.head[a] = heap.merge(heap.head[a], heap.head[b]);
                    heap.head[b] = 0;
                }
            }
            // 自底向上重新连接保留的半条直径，并在重连前更新子树高度。
            // 注意 u 更新时尚未加入父亲堆，因此修改堆键不会破坏父亲的堆序。
            for (int i = middle; i >= 0; --i) {
                u = path[i];
                heap.update(u);
                if (i) heap.head[path[i - 1]] = heap.merge(heap.head[path[i - 1]], u);
            }
            ++answer;
        }
        cout << answer << '\n';
    }
}
