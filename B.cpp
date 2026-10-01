#include <bits/stdc++.h>
using namespace std;
using int64 = long long;
using Real = long double;

struct Point { int64 x, y; };
int64 distance2(Point a, Point b) {
    int64 dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
}
struct Circle { Real center; int64 radius2; Real start; };

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout << fixed << setprecision(15);
    int T; cin >> T;
    while (T--) {
        int n; cin >> n;
        vector<Point> p(n);
        for (auto &a : p) cin >> a.x >> a.y;
        vector<int64> radius2(n);

        // Prefix-only and suffix-only farthest-point decisions are monotone.
        // Unrestricted decisions are NOT necessarily cyclically monotone.
        auto solve = [&](auto &&self, int l, int r, int lo, int hi, bool suffix) -> void {
            if (l > r) return;
            int mid = (l + r) / 2, best = lo;
            int64 value = -1;
            int begin = suffix ? max(lo, mid + 1) : lo;
            int end = suffix ? hi : min(hi, mid - 1);
            for (int j = begin; j <= end; ++j) {
                int64 d = distance2(p[mid], p[j]);
                if (d > value) { value = d; best = j; }
            }
            radius2[mid] = max(radius2[mid], value);
            self(self, l, mid - 1, lo, best, suffix);
            self(self, mid + 1, r, best, hi, suffix);
        };
        solve(solve, 0, n - 2, 1, n - 1, true);
        solve(solve, 1, n - 1, 0, n - 2, false);

        vector<Real> center(n);
        Real perimeter = 0;
        for (int i = 0; i < n; ++i) {
            perimeter += sqrtl((Real)distance2(p[(i + n - 1) % n], p[i]));
            center[i] = perimeter;
        }
        vector<Circle> hull; hull.reserve(3 * n);
        auto intersection = [](const Circle &a, const Circle &b) {
            return (a.center + b.center) / 2 +
                (Real)(a.radius2 - b.radius2) / (2 * (b.center - a.center));
        };
        for (int shift = -1; shift <= 1; ++shift)
            for (int i = 0; i < n; ++i) {
                Circle current{center[i] + shift * perimeter, radius2[i], 0};
                while (!hull.empty()) {
                    current.start = intersection(hull.back(), current);
                    if (current.start > hull.back().start) break;
                    hull.pop_back();
                }
                if (hull.empty()) current.start = -numeric_limits<Real>::infinity();
                hull.push_back(current);
            }
        auto primitive = [](Real x, Real r, Real r2) {
            x = max(-r, min(r, x));
            return (x * sqrtl(max((Real)0, r2 - x * x)) + r2 * asinl(x / r)) / 2;
        };
        Real area = 0;
        for (size_t i = 0; i < hull.size(); ++i) {
            const Circle &c = hull[i];
            Real left = max((Real)0, c.start);
            Real right = min(perimeter, i + 1 < hull.size() ? hull[i + 1].start : perimeter);
            if (left >= right) continue;
            Real r = sqrtl((Real)c.radius2);
            area += primitive(right - c.center, r, c.radius2) - primitive(left - c.center, r, c.radius2);
        }
        cout << area / perimeter << '\n';
    }
}
