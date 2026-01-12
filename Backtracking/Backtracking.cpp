#include <chrono>
#include <vector>
using namespace std;

int n, k, ans = 0, temp_ans = 0;
double TIME_LIMIT;
vector<pair<int, int>> orders; // quantity, cost
vector<pair<int, int>> trucks; // bounds c1 <= c2
vector<int> assignment;
vector<int> temp_assignment;
vector<int> temp_weight;

void Try(int order) {
    if (clock() >= TIME_LIMIT) return;
    if (order == n) {
        if (temp_ans > ans) {
            bool check = true;
            for (int i = 0; i < k; ++i) {
                if (temp_weight[i] && temp_weight[i] < trucks[i].first) {
                    check = false;
                    break;
                }
            }
            if (check) {
                assignment = temp_assignment;
                ans = temp_ans;
            }
        }
        return;
    }
    for (int truck = 0; truck <= k; ++truck) { // k: không gán order cho xe nào
        if (truck < k && temp_weight[truck] + orders[order].first > trucks[truck].second) continue;
        temp_assignment[order] = truck;
        if (truck < k) {
            temp_weight[truck] += orders[order].first;
            temp_ans += orders[order].second;
        }
        Try(order + 1);
        temp_assignment[order] = -2;
        if (truck < k) {
            temp_weight[truck] -= orders[order].first;
            temp_ans -= orders[order].second;
        }
    }
}

int main() {
    //TIME_LIMIT = 0.9 * CLOCKS_PER_SEC + clock();
    scanf("%d %d", &n, &k);
    orders.resize(n);
    trucks.resize(k);
    assignment.resize(n, k);
    temp_assignment.resize(n, -2);
    temp_weight.resize(k, 0);
    for (int i = 0; i < n; ++i) scanf("%d %d", &orders[i].first, &orders[i].second);
    for (int i = 0; i < k; ++i) scanf("%d %d", &trucks[i].first, &trucks[i].second);
    TIME_LIMIT = 0.9 * CLOCKS_PER_SEC + clock();
    Try(0);
    int m = 0;
    for (int u : assignment) if (u < k) ++m;
    printf("%d\n", m);
    for (int i = 0; i < n; ++i) if (assignment[i] < k) printf("%d %d\n", i + 1, assignment[i] + 1);
    printf("%d", ans);
}