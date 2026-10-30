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
    // 返回 x 的质因数分解，pair 表示（质数，指数）。
    // 试除过的质数已经被除尽，最后剩下的 x > 1 必为质数。
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
        // 模 n 的单位群可以拆成若干循环群的直积，cyclic 记录各循环群的阶。
        // 只有与 n 互质的 a 才有非零乘法阶；其它元素的贡献为零。
        // 奇素数幂 p^e 对应 C_{p^(e-1)(p-1)}；2 的幂需要单独处理。
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
        // 按同一个素数 p 归类循环群，得到 p 部分
        // G_p = C_{p^a1} × ... × C_{p^at}，exponents 保存所有 a_j。
        map<int, vector<int>> primary;
        for (int s : cyclic)
            for (auto [p, e] : factor(s)) primary[p].push_back(e);
        int64 answer = 1;
        for (auto &[p, exponents] : primary) {
            int maximum = *max_element(exponents.begin(), exponents.end());
            // previous = F(k-1)，killed = F(k)，其中 F(k) 是阶整除 p^k 的元素数。
            // sum 初始包含阶为 1 的那个元素对“阶之和”的贡献 1。
            int64 previous = 1, killed = 1, order = 1, sum = 1;
            for (int k = 1; k <= maximum; ++k) {
                order *= p; // 当前考察的精确阶 p^k。
                // 每个循环因子的贡献从 p^min(a_j,k-1) 增为 p^min(a_j,k)，
                // 只有 a_j >= k 时乘以 p，所以 F(k) 可由 F(k-1) 增量得到。
                for (int e : exponents) if (e >= k) killed *= p;
                // 阶恰为 p^k 的元素数是 F(k)-F(k-1)。
                sum += order * (killed - previous);
                previous = killed;
            }
            // 不同素数部分的阶互质，直积元素的阶为乘积，求和后可分别相乘。
            answer *= sum;
        }
        cout << answer << '\n';
    }
}
