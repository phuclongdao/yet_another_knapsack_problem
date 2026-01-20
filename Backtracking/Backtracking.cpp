#define _CRT_SECURE_NO_WARNINGS
#include <chrono>
#include <vector>
#include <cstdio>
#include <algorithm>
#include <climits>
using namespace std;

struct Order {
    int quantity;
    int cost;
    int id;
    Order(int quantity_, int cost_, int id_) : quantity(quantity_), cost(cost_), id(id_) {}
};

struct Truck {
    int low;
    int high;
    int id;
    Truck(int low_, int high_, int id_) : low(low_), high(high_), id(id_) {}
};

int n, k;
double TIME_LIMIT;
vector<Order> orders; // quantity, cost, id
vector<Truck> trucks; // bounds c1 <= c2, id

//Result
int ans = 0, temp_ans = 0;
vector<int> assignment, temp_assignment, temp_weight;

//Bounds
int sum_truck_deficit = 0, num_truck_deficit;
vector<int> suffix_sum_quantity, suffix_sum_cost;

void Try(int order) {
    if (clock() >= TIME_LIMIT) return;
    if (suffix_sum_quantity[order] < sum_truck_deficit) return;
    if (temp_ans + suffix_sum_cost[order] <= ans) return;
    if (n - order < num_truck_deficit) return;
    if (order == n) {
        if (sum_truck_deficit == 0) {
            assignment = temp_assignment;
            ans = temp_ans;
        }
        return;
    }
    const auto& [curr_quantity, curr_cost, curr_id] = orders[order];
    for (auto& truck : trucks) {
        int& curr_truck_weight = temp_weight[truck.id];
        if (curr_truck_weight + curr_quantity > truck.high) continue;
        temp_assignment[curr_id] = truck.id;
        int deficit_change = min(max(truck.low - curr_truck_weight, 0), curr_quantity);
        bool change_num_deficit = (curr_truck_weight < truck.low) && (curr_truck_weight + curr_quantity >= truck.low);
        curr_truck_weight += curr_quantity;
        temp_ans += curr_cost;
        sum_truck_deficit -= deficit_change;
        if (change_num_deficit) --num_truck_deficit;
        Try(order + 1);
        curr_truck_weight -= curr_quantity;
        temp_ans -= curr_cost;
        sum_truck_deficit += deficit_change;
        if (change_num_deficit) ++num_truck_deficit;
    }
    temp_assignment[curr_id] = INT_MAX; // INT_MAX: không gán order cho xe nào
    Try(order + 1);
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        freopen(argv[1], "r", stdin);
    }
    TIME_LIMIT = 0.95 * CLOCKS_PER_SEC + clock();
    scanf("%d %d", &n, &k);
    orders.reserve(n);
    trucks.reserve(k);
    assignment.resize(n, INT_MAX);
    temp_assignment.resize(n, INT_MAX);
    temp_weight.resize(k, 0);
    suffix_sum_quantity.resize(n + 1, 0);
    suffix_sum_cost.resize(n + 1, 0);
    int a, b, id;
    for (id = 0; id < n; ++id) {
        scanf("%d %d", &a, &b);
        orders.emplace_back(a, b, id);
    }
    for (id = 0; id < k; ++id) {
        scanf("%d %d", &a, &b);
        trucks.emplace_back(a, b, id);
        sum_truck_deficit += a;
    }
    num_truck_deficit = k;
    //Sorting begin
    sort(orders.begin(), orders.end(),
        [](auto& a, auto& b) {
            return a.quantity > b.quantity;
        });
    //Sorting end
    for (id = n - 1; id >= 0; --id) {
        suffix_sum_quantity[id] = suffix_sum_quantity[id + 1] + orders[id].quantity;
        suffix_sum_cost[id] = suffix_sum_cost[id + 1] + orders[id].cost;
    }
    Try(0);
    int num_orders = 0;
    for (int u : assignment) if (u < INT_MAX) ++num_orders;
    printf("%d\n", num_orders);
    for (id = 0; id < n; ++id) if (assignment[id] < INT_MAX) {
        printf("%d %d\n", id + 1, assignment[id] + 1);
    }
    printf("%d\n", ans);
}