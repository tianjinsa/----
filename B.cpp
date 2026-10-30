#include <bits/stdc++.h>
using namespace std;
using int64 = long long;
using Real = long double;

struct Point { int64 x, y; };
int64 distance2(Point a, Point b) {
    int64 dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy; // 用整数距离平方比较最远点，避免浮点误差。
}
// 稳定扫过区域的上边界可转化为上半圆的上包络。
// center 是支点的横坐标，radius2 是支点到最远顶点距离的平方，
// start 是该圆开始在包络上占优的横坐标（求包络时填写）。
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

        // 分别求“只看后缀 j>i”和“只看前缀 j<i”时的最远点。
        // 在凸多边形上，这两个限制下的最优下标具有决策单调性；
        // 不限制前缀/后缀时不能直接假设最优下标循环单调。
        // [l,r] 是待求解的行，[lo,hi] 是这些行的候选决策区间。
        auto solve = [&](auto &&self, int l, int r, int lo, int hi, bool suffix) -> void {
            if (l > r) return;
            int mid = (l + r) / 2, best = lo;
            int64 value = -1;
            int begin = suffix ? max(lo, mid + 1) : lo;
            int end = suffix ? hi : min(hi, mid - 1);
            // 扫描中间行的所有合法候选，找到它的最优决策。
            for (int j = begin; j <= end; ++j) {
                int64 d = distance2(p[mid], p[j]);
                if (d > value) { value = d; best = j; }
            }
            radius2[mid] = max(radius2[mid], value);
            // 左侧行的最优决策不超过 best，右侧行则不少于 best。
            self(self, l, mid - 1, lo, best, suffix);
            self(self, mid + 1, r, best, hi, suffix);
        };
        solve(solve, 0, n - 2, 1, n - 1, true);
        solve(solve, 1, n - 1, 0, n - 2, false);

        // 绕多边形滚动一整周长 P 后，图形姿态复原并平移 P，扫过区域呈周期性。
        // 第 i 个顶点成为支点时，其横坐标是此前经过的边长之和。
        vector<Real> center(n);
        Real perimeter = 0;
        for (int i = 0; i < n; ++i) {
            perimeter += sqrtl((Real)distance2(p[(i + n - 1) % n], p[i]));
            center[i] = perimeter;
        }
        vector<Circle> hull; hull.reserve(3 * n); // 半圆上包络的单调栈。
        // y_i^2=-x^2+2*c_i*x+(r_i^2-c_i^2)，公共项 -x^2 不影响比较。
        // 因而两圆上包络的切换点就是两条对应直线的交点。
        // 先整数相减半径平方再转为浮点数，可避免巨大圆心平方带来的精度损失。
        auto intersection = [](const Circle &a, const Circle &b) {
            return (a.center + b.center) / 2 +
                (Real)(a.radius2 - b.radius2) / (2 * (b.center - a.center));
        };
        // 半径不超过 P/2，所以计算 [0,P] 的包络只需原周期及左右各一个周期。
        // 圆心按横坐标递增，等价于按斜率递增地维护直线上包络。
        for (int shift = -1; shift <= 1; ++shift)
            for (int i = 0; i < n; ++i) {
                Circle current{center[i] + shift * perimeter, radius2[i], 0};
                while (!hull.empty()) {
                    current.start = intersection(hull.back(), current);
                    // 新圆在栈顶圆开始生效之前或同一位置就超过它，栈顶圆无贡献。
                    if (current.start > hull.back().start) break;
                    hull.pop_back();
                }
                if (hull.empty()) current.start = -numeric_limits<Real>::infinity();
                hull.push_back(current);
            }
        // 圆的上半圆原函数：∫sqrt(r^2-x^2)dx
        // = (x*sqrt(r^2-x^2)+r^2*asin(x/r))/2。
        auto primitive = [](Real x, Real r, Real r2) {
            // 截断根号参数和 asin 参数的微小越界，消除浮点舍入影响。
            x = max(-r, min(r, x));
            return (x * sqrtl(max((Real)0, r2 - x * x)) + r2 * asinl(x / r)) / 2;
        };
        Real area = 0;
        // 栈中相邻圆的 start 划出了各自负责的最优区间，与 [0,P] 取交后积分。
        for (size_t i = 0; i < hull.size(); ++i) {
            const Circle &c = hull[i];
            Real left = max((Real)0, c.start);
            Real right = min(perimeter, i + 1 < hull.size() ? hull[i + 1].start : perimeter);
            if (left >= right) continue;
            Real r = sqrtl((Real)c.radius2);
            area += primitive(right - c.center, r, c.radius2) - primitive(left - c.center, r, c.radius2);
        }
        // 一个周期的面积除以周长，就是无限滚动时扫过面积与滚动距离之比的极限。
        cout << area / perimeter << '\n';
    }
}
