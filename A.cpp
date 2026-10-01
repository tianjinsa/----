#include <bits/stdc++.h>
using namespace std;

struct Basis {
    array<int, 20> b{};
    int rank = 0;
    bool insert(int x) {
        while (x) {
            int k = 31 - __builtin_clz((unsigned)x);
            if (b[k]) x ^= b[k];
            else { b[k] = x; ++rank; return true; }
        }
        return false;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, m;
    cin >> n >> m;
    vector<vector<int>> sets(n), span(n);
    vector<Basis> local(n);
    vector<int> frequency(1 << m), mark(1 << m, -1);
    for (int i = 0; i < n; ++i) {
        int c; cin >> c;
        sets[i].resize(c);
        for (int &x : sets[i]) {
            cin >> x;
            ++frequency[x];
            local[i].insert(x);
        }
    }
    Basis common;
    vector<int> events, active;
    auto force = [&](int x) {
        if (common.insert(x)) events.push_back(x);
    };
    auto absorb = [&](int i) {
        for (int x : local[i].b) if (x) force(x);
        vector<int>().swap(span[i]);
    };
    for (int i = 0; i < n && common.rank < m; ++i) {
        // More than half the space is missing, so the missing vectors span it.
        if ((1 << local[i].rank) > 2 * (int)sets[i].size()) {
            absorb(i);
            continue;
        }
        for (int x : sets[i]) mark[x] = i;
        auto &v = span[i];
        v.reserve(1 << local[i].rank);
        v.push_back(0);
        for (int x : local[i].b) if (x) {
            int s = (int)v.size();
            for (int j = 0; j < s; ++j) v.push_back(v[j] ^ x);
        }
        for (int x : v) if (mark[x] != i) force(x);
        active.push_back(i);
    }
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
            for (int k = 0; k < s; ++k) {
                int y = v[k] ^ x;
                v.push_back(y);
                if (mark[y] != i) force(y);
            }
            ++j;
        }
    }
    vector<int> v{0};
    v.reserve(1 << common.rank);
    for (int x : common.b) if (x) {
        int s = (int)v.size();
        for (int j = 0; j < s; ++j) v.push_back(v[j] ^ x);
    }
    vector<int> answer;
    for (int x : v) if (frequency[x] != n) answer.push_back(x);
    cout << answer.size() << '\n';
    for (int x : answer) cout << x << ' ';
    cout << '\n';
}
