#include <bits/stdc++.h>
using namespace std;

// 将可能为负的差值规范到 {0, 1, 2}。
int mod3(int x) { x %= 3; return x < 0 ? x + 3 : x; }

// 查询 mask 对应的顶点集合，翻转 vertex 后割边数模 3 的值。
// vertex == -1 表示不翻转顶点，用来获得当前 mask 下的基准值。
// endl 同时换行并刷新缓冲区，避免交互器等待输出而死锁。
int ask(int mask, int vertex) {
    cout << "? " << mask << ' ' << vertex << endl;
    int r;
    if (!(cin >> r) || r == -1) exit(0); // 交互失败或收到终止信号时立即停止。
    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T; cin >> T;
    while (T--) {
        int n; cin >> n;
        if (n == 1) { cout << "! 0" << endl; continue; }
        // 顶点编号需要 k = ceil(log2(n)) 个二进制位。
        int k = 0;
        while ((1 << k) < n) ++k;
        // delta[v][b] 是翻转 v 后，第 b 个单比特割的割边数变化量模 3。
        vector<vector<int>> delta(n, vector<int>(k));
        vector<int> degree(n, 1); // 路径上的度数只可能是 1 或 2。
        for (int bit = 0; bit < k; ++bit) {
            int base = ask(1 << bit, -1), sum = 0;
            for (int v = 0; v + 1 < n; ++v) {
                delta[v][bit] = mod3(ask(1 << bit, v) - base);
                sum += delta[v][bit];
            }
            // 每条跨割边在两个端点处各被计数一次，所以
            // sum_v delta(v) = 2(n-1)-4c ≡ 2(n-1)-c (mod 3)。
            // 用前 n-1 个顶点的结果推回最后一个顶点，每位节省一次查询。
            delta[n - 1][bit] = mod3(2 * (n - 1) - base - sum);
        }
        // 内部顶点的两个邻居编号不同，至少一位不同；该位恰有一条关联边跨割，
        // 因而存在 delta = 2-2 = 0。端点只有一条边，变化量不可能为零。
        for (int v = 0; v < n; ++v)
            for (int d : delta[v]) if (d == 0) degree[v] = 2;

        // 从编号较小的端点开始，保证最终输出满足首端编号小于末端编号。
        int start = 0;
        while (degree[start] != 1) ++start;
        vector<int> answer;
        int previous = -1, current = start;
        for (int step = 0; step < n; ++step) {
            answer.push_back(current);
            if (step + 1 == n) break;
            int next = 0;
            // 设 s 是 current 的邻居中该位为 1 的数量，d 为对应变化量。
            // current 该位为 0 时：s ≡ d-degree；为 1 时：s ≡ -d-degree (mod 3)。
            // 度数至多 2，因此模 3 的 s 就是精确数量；减去 previous 的贡献后，
            // 剩下的 0/1 恰是后继 next 的这一位。逐位即可恢复后继编号。
            for (int bit = 0; bit < k; ++bit) {
                int d = delta[current][bit];
                int count = mod3(((current >> bit) & 1) ? -d - degree[current]
                                                                       : d - degree[current]);
                if (previous != -1) count -= (previous >> bit) & 1;
                if (count) next |= 1 << bit;
            }
            previous = current;
            current = next;
        }
        cout << '!';
        for (int v : answer) cout << ' ' << v;
        cout << endl;
    }
}
