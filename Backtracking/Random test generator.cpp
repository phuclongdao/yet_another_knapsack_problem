#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <numeric>

using namespace std;

static mt19937 rng(
    (unsigned)chrono::steady_clock::now().time_since_epoch().count()
);

int rand_int(int l, int r) {
    uniform_int_distribution<int> dist(l, r);
    return dist(rng);
}

int main() {
    int N = 120;
    int K = 100;

    ofstream fout("test_random.txt");
    if (!fout.is_open()) {
        cerr << "Cannot open output file\n";
        return 1;
    }

    // =========================
    // 1. Sinh order
    // =========================
    vector<int> quantity(N), cost(N);
    for (int i = 0; i < N; ++i) {
        quantity[i] = rand_int(5, 30);
        cost[i] = rand_int(1, 100);
    }

    // =========================
    // 2. Chọn tập order được dùng
    // =========================
    vector<int> ids(N);
    iota(ids.begin(), ids.end(), 0);
    shuffle(ids.begin(), ids.end(), rng);

    int used_cnt = rand_int(K, N - 1);  // BẮT BUỘC >= K
    vector<int> used_orders(ids.begin(), ids.begin() + used_cnt);

    // =========================
    // 3. Phân phối order cho truck (KHÔNG nhân bản)
    // =========================
    vector<vector<int>> bucket(K);

    // 3.1 Mỗi truck nhận đúng 1 order trước
    for (int k = 0; k < K; ++k) {
        bucket[k].push_back(used_orders[k]);
    }

    // 3.2 Phần order còn lại phân ngẫu nhiên
    for (int i = K; i < used_cnt; ++i) {
        int t = rand_int(0, K - 1);
        bucket[t].push_back(used_orders[i]);
    }

    // =========================
    // 4. Sinh low / high dựa trên load thực
    // =========================
    vector<int> low(K), high(K);
    for (int k = 0; k < K; ++k) {
        int load = 0;
        for (int id : bucket[k]) load += quantity[id];

        int slack_low = rand_int(0, load / 4);
        int slack_high = rand_int(0, load / 2);

        low[k] = max(1, load - slack_low);
        high[k] = load + slack_high;
    }

    // =========================
    // 5. Shuffle order để che nghiệm
    // =========================
    vector<int> perm(N);
    iota(perm.begin(), perm.end(), 0);
    shuffle(perm.begin(), perm.end(), rng);

    // =========================
    // 6. Ghi file
    // =========================
    fout << N << " " << K << "\n";
    for (int i : perm) {
        fout << quantity[i] << " " << cost[i] << "\n";
    }
    for (int k = 0; k < K; ++k) {
        fout << low[k] << " " << high[k] << "\n";
    }

    fout.close();
    return 0;
}
