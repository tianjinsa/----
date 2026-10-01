#include <bits/stdc++.h>
using namespace std;

int mod3(int x) { x %= 3; return x < 0 ? x + 3 : x; }

int ask(int mask, int vertex) {
    cout << "? " << mask << ' ' << vertex << endl;
    int r;
    if (!(cin >> r) || r == -1) exit(0);
    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T; cin >> T;
    while (T--) {
        int n; cin >> n;
        if (n == 1) { cout << "! 0" << endl; continue; }
        int k = 0;
        while ((1 << k) < n) ++k;
        vector<vector<int>> delta(n, vector<int>(k));
        vector<int> degree(n, 1);
        for (int bit = 0; bit < k; ++bit) {
            int base = ask(1 << bit, -1), sum = 0;
            for (int v = 0; v + 1 < n; ++v) {
                delta[v][bit] = mod3(ask(1 << bit, v) - base);
                sum += delta[v][bit];
            }
            // Sum_v delta(v) = 2(n-1) - 4*c, also valid modulo 3.
            delta[n - 1][bit] = mod3(2 * (n - 1) - base - sum);
        }
        for (int v = 0; v < n; ++v)
            for (int d : delta[v]) if (d == 0) degree[v] = 2;

        int start = 0;
        while (degree[start] != 1) ++start;
        vector<int> answer;
        int previous = -1, current = start;
        for (int step = 0; step < n; ++step) {
            answer.push_back(current);
            if (step + 1 == n) break;
            int next = 0;
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
