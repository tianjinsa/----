#include <bits/stdc++.h>
using namespace std;

// Each vertex is one heap item in its parent's heap of children.
// head[v] is a separate heap containing v's own children.
struct LeftistHeap {
    vector<int> left, right, dist, height, head;
    explicit LeftistHeap(int n) : left(n + 1), right(n + 1), dist(n + 1, 1),
                                  height(n + 1), head(n + 1) { dist[0] = 0; }
    int merge(int a, int b) {
        if (!a || !b) return a | b;
        if (height[a] < height[b]) swap(a, b);
        right[a] = merge(right[a], b);
        if (dist[left[a]] < dist[right[a]]) swap(left[a], right[a]);
        dist[a] = dist[right[a]] + 1;
        return a;
    }
    int pop_child(int u) {
        int v = head[u];
        head[u] = merge(left[v], right[v]);
        left[v] = right[v] = 0;
        dist[v] = 1;
        return v;
    }
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
        vector<int> parent(n + 1, -1), order{1};
        parent[1] = 0;
        for (size_t i = 0; i < order.size(); ++i)
            for (int v : graph[order[i]]) if (v != parent[order[i]]) {
                parent[v] = order[i]; order.push_back(v);
            }
        int root = order.back(); // A farthest vertex from 1 is a diameter endpoint.
        order.assign(1, root); parent[root] = 0;
        for (size_t i = 0; i < order.size(); ++i)
            for (int v : graph[order[i]]) if (v != parent[order[i]]) {
                parent[v] = order[i]; order.push_back(v);
            }
        LeftistHeap heap(n);
        for (int i = n - 1; i >= 0; --i) {
            int u = order[i];
            heap.update(u);
            if (parent[u]) heap.head[parent[u]] = heap.merge(heap.head[parent[u]], u);
        }
        int answer = 0;
        vector<int> path; path.reserve(n);
        while (heap.head[root]) {
            path.clear();
            int u = root;
            path.push_back(u);
            // Remove all diameter edges from the child heaps.
            while (heap.head[u]) {
                u = heap.pop_child(u);
                path.push_back(u);
            }
            int D = (int)path.size() - 1, middle = D / 2;
            for (int i = 0; i <= middle; ++i) {
                int a = path[i], b = path[D - i];
                if (a != b) {
                    heap.head[a] = heap.merge(heap.head[a], heap.head[b]);
                    heap.head[b] = 0;
                }
            }
            // Rebuild the retained half of the diameter, bottom up.
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
