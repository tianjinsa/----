#include <bits/stdc++.h>
using namespace std;
using int64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    vector<int> primes;
    vector<bool> composite(31624);
    for (int i = 2; i <= 31623; ++i) {
        if (!composite[i]) {
            primes.push_back(i);
            if (i * i <= 31623)
                for (int j = i * i; j <= 31623; j += i) composite[j] = true;
        }
    }
    auto factor = [&](int x) {
        vector<pair<int, int>> result;
        for (int p : primes) {
            if (1LL * p * p > x) break;
            if (x % p == 0) {
                int e = 0;
                do { x /= p; ++e; } while (x % p == 0);
                result.emplace_back(p, e);
            }
        }
        if (x > 1) result.emplace_back(x, 1);
        return result;
    };
    int T; cin >> T;
    while (T--) {
        int n; cin >> n;
        // The unit group is a direct product of cyclic groups of these sizes.
        vector<int> cyclic;
        for (auto [p, e] : factor(n)) {
            if (p == 2) {
                if (e == 2) cyclic.push_back(2);
                if (e >= 3) {
                    cyclic.push_back(2);
                    cyclic.push_back(1 << (e - 2));
                }
            } else {
                int s = p - 1;
                for (int i = 1; i < e; ++i) s *= p;
                cyclic.push_back(s);
            }
        }
        map<int, vector<int>> primary;
        for (int s : cyclic)
            for (auto [p, e] : factor(s)) primary[p].push_back(e);
        int64 answer = 1;
        for (auto &[p, exponents] : primary) {
            int maximum = *max_element(exponents.begin(), exponents.end());
            int64 previous = 1, killed = 1, order = 1, sum = 1;
            for (int k = 1; k <= maximum; ++k) {
                order *= p;
                for (int e : exponents) if (e >= k) killed *= p;
                sum += order * (killed - previous);
                previous = killed;
            }
            answer *= sum;
        }
        cout << answer << '\n';
    }
}
