
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <climits>
#include <set>
#include <map>
#include <random>
#include <chrono>
#include <cstdio>
using namespace std;

// ==================== GLOBAL DATA ====================
int N, K;
vector<int> qty;  // qty[i] for order i
vector<int> cst;  // cst[i] for order i (cost)
vector<int> minC; // minC[k] for vehicle k
vector<int> maxC; // maxC[k] for vehicle k

mt19937 rng;

// ==================== SOLUTION ====================
struct Solution
{
    vector<int> assign; // assign[i] = vehicle (-1 if unassigned)
    vector<int> load;   // load[k] = total quantity in vehicle k
    vector<int> vcost;  // vcost[k] = sum of costs of orders in vehicle k
    int totalCost;

    void init()
    {
        assign.assign(N, -1);
        load.assign(K, 0);
        vcost.assign(K, 0);
        totalCost = 0;
    }

    bool valid(int k) const
    {
        return load[k] == 0 || (load[k] >= minC[k] && load[k] <= maxC[k]);
    }

    // Recalculate everything from scratch
    void recalc()
    {
        load.assign(K, 0);
        vcost.assign(K, 0);
        for (int i = 0; i < N; i++)
        {
            if (assign[i] >= 0)
            {
                int k = assign[i];
                load[k] += qty[i];
                vcost[k] += cst[i];
            }
        }
        totalCost = 0;
        for (int k = 0; k < K; k++)
        {
            if (valid(k))
            {
                totalCost += vcost[k];
            }
        }
    }

    // O(1) delta calculation for moving order i from fromK to toK
    int calcDelta(int i, int fromK, int toK) const
    {
        if (fromK == toK)
            return 0;

        // Check capacity constraint
        if (toK >= 0 && load[toK] + qty[i] > maxC[toK])
        {
            return INT_MIN;
        }

        int delta = 0;

        // Effect on source vehicle (fromK)
        if (fromK >= 0)
        {
            bool wasValid = valid(fromK);
            int newLoad = load[fromK] - qty[i];
            bool willBeValid = (newLoad == 0) || (newLoad >= minC[fromK] && newLoad <= maxC[fromK]);

            if (wasValid && !willBeValid)
            {
                delta -= vcost[fromK];
            }
            else if (wasValid && willBeValid)
            {
                delta -= cst[i];
            }
            else if (!wasValid && willBeValid && newLoad > 0)
            {
                delta += vcost[fromK] - cst[i];
            }
        }

        // Effect on destination vehicle (toK)
        if (toK >= 0)
        {
            bool wasValid = valid(toK);
            int newLoad = load[toK] + qty[i];
            bool willBeValid = (newLoad >= minC[toK] && newLoad <= maxC[toK]);

            if (!wasValid && willBeValid)
            {
                delta += vcost[toK] + cst[i];
            }
            else if (wasValid && willBeValid)
            {
                delta += cst[i];
            }
            else if (wasValid && !willBeValid)
            {
                delta -= vcost[toK];
            }
        }

        return delta;
    }

    // Apply move
    void move(int i, int toK)
    {
        int fromK = assign[i];

        if (fromK >= 0)
        {
            load[fromK] -= qty[i];
            vcost[fromK] -= cst[i];
        }
        if (toK >= 0)
        {
            load[toK] += qty[i];
            vcost[toK] += cst[i];
        }
        assign[i] = toK;
    }

    // Recalculate totalCost only
    void recalcCost()
    {
        totalCost = 0;
        for (int k = 0; k < K; k++)
        {
            if (valid(k))
            {
                totalCost += vcost[k];
            }
        }
    }
};

// ==================== GREEDY INITIAL ====================
Solution greedyInit()
{
    Solution sol;
    sol.init();

    // Sort by ratio descending (with division by zero protection)
    vector<int> perm(N);
    for (int i = 0; i < N; i++)
        perm[i] = i;
    sort(perm.begin(), perm.end(), [](int a, int b)
         { 
             double ra = (qty[a] > 0) ? (double)cst[a] / qty[a] : 0;
             double rb = (qty[b] > 0) ? (double)cst[b] / qty[b] : 0;
             return ra > rb; });

    for (int i : perm)
    {
        int bestK = -1;
        int bestScore = INT_MIN;

        for (int k = 0; k < K; k++)
        {
            int newLoad = sol.load[k] + qty[i];
            if (newLoad > maxC[k])
                continue;

            int score = 0;
            if (newLoad >= minC[k])
                score += 1000000;
            score += sol.load[k];

            if (score > bestScore)
            {
                bestScore = score;
                bestK = k;
            }
        }

        if (bestK >= 0)
        {
            sol.move(i, bestK);
        }
    }

    sol.recalcCost();
    return sol;
}

// ==================== RANDOMIZED GREEDY ====================
Solution randGreedy()
{
    Solution sol;
    sol.init();

    vector<int> perm(N);
    for (int i = 0; i < N; i++)
        perm[i] = i;
    shuffle(perm.begin(), perm.end(), rng);

    // Pre-generate random noise values to avoid issues in lambda
    vector<double> noise(N);
    uniform_real_distribution<double> noise_dist(0.7, 1.3);
    for (int i = 0; i < N; i++)
        noise[i] = noise_dist(rng);

    // Sort with noise (with division by zero protection)
    stable_sort(perm.begin(), perm.end(), [&noise](int a, int b)
                {
        double ra = (qty[a] > 0) ? (double)cst[a] / qty[a] : 0;
        double rb = (qty[b] > 0) ? (double)cst[b] / qty[b] : 0;
        ra *= noise[a];
        rb *= noise[b];
        return ra > rb; });

    for (int i : perm)
    {
        vector<int> cands;
        for (int k = 0; k < K; k++)
        {
            if (sol.load[k] + qty[i] <= maxC[k])
            {
                cands.push_back(k);
            }
        }
        if (cands.empty())
            continue;

        sort(cands.begin(), cands.end(), [&](int a, int b)
             {
            bool av = sol.load[a] + qty[i] >= minC[a];
            bool bv = sol.load[b] + qty[i] >= minC[b];
            if (av != bv) return av > bv;
            return sol.load[a] > sol.load[b]; });

        int pick = 0;
        if (cands.size() > 1)
        {
            uniform_int_distribution<int> percent_dist(0, 99);
            if (percent_dist(rng) < 25)
            {
                uniform_int_distribution<int> pick_dist(0, min((int)cands.size() - 1, 2));
                pick = pick_dist(rng);
            }
        }
        sol.move(i, cands[pick]);
    }

    sol.recalcCost();
    return sol;
}

// ==================== LOCAL SEARCH ====================
void localSearch(Solution &sol)
{
    bool improved = true;
    while (improved)
    {
        improved = false;
        for (int i = 0; i < N && !improved; i++)
        {
            int fromK = sol.assign[i];
            for (int toK = -1; toK < K; toK++)
            {
                if (toK == fromK)
                    continue;
                int delta = sol.calcDelta(i, fromK, toK);
                if (delta > 0)
                {
                    sol.move(i, toK);
                    sol.recalcCost();
                    improved = true;
                    break;
                }
            }
        }
    }
}

// ==================== TABU SEARCH ====================
Solution tabuSearch(Solution init, int maxIter, int maxNoImprove, int tenure)
{
    Solution cur = init;
    Solution best = cur;

    map<pair<int, int>, int> tabu; // (order, vehicle) -> iteration when becomes non-tabu

    int iter = 0, noImprove = 0;

    while (iter < maxIter && noImprove < maxNoImprove)
    {
        iter++;

        // Periodic cleanup to prevent memory buildup
        if (iter % 100 == 0 && tabu.size() > 1000)
        {
            // Remove old entries
            for (auto it = tabu.begin(); it != tabu.end();)
            {
                if (it->second <= iter)
                    it = tabu.erase(it);
                else
                    ++it;
            }
        }

        int bestDelta = INT_MIN;
        int bestI = -1, bestK = -1;

        // Find best move
        for (int i = 0; i < N; i++)
        {
            int fromK = cur.assign[i];
            for (int toK = -1; toK < K; toK++)
            {
                if (toK == fromK)
                    continue;

                int delta = cur.calcDelta(i, fromK, toK);
                if (delta == INT_MIN)
                    continue;

                bool isTabu = tabu.count({i, toK}) && tabu[{i, toK}] > iter;

                // Aspiration: accept if improves best known
                if (isTabu && cur.totalCost + delta <= best.totalCost)
                    continue;

                if (delta > bestDelta)
                {
                    bestDelta = delta;
                    bestI = i;
                    bestK = toK;
                }
            }
        }

        if (bestI < 0)
        {
            noImprove++;
            continue;
        }

        // Apply move
        int fromK = cur.assign[bestI];
        cur.move(bestI, bestK);
        cur.recalcCost();

        // Update tabu
        if (fromK >= 0)
        {
            tabu[{bestI, fromK}] = iter + tenure;
        }

        if (cur.totalCost > best.totalCost)
        {
            best = cur;
            noImprove = 0;
        }
        else
        {
            noImprove++;
        }
    }

    return best;
}

// ==================== MAIN ====================
int main(int argc, char *argv[])
{
    try
    {
        rng.seed(chrono::high_resolution_clock::now().time_since_epoch().count());

        FILE *f = (argc > 1) ? fopen(argv[1], "r") : stdin;
        if (!f)
        {
            return 1;
        }

        int readResult = fscanf(f, "%d %d", &N, &K);
        if (readResult != 2 || N <= 0 || K <= 0 || N > 10000 || K > 10000)
        {
            if (argc > 1)
                fclose(f);
            return 1;
        }

        qty.resize(N);
        cst.resize(N);
        for (int i = 0; i < N; i++)
        {
            if (fscanf(f, "%d %d", &qty[i], &cst[i]) != 2)
                return 1;
        }
        minC.resize(K);
        maxC.resize(K);
        for (int k = 0; k < K; k++)
        {
            if (fscanf(f, "%d %d", &minC[k], &maxC[k]) != 2)
                return 1;
        }
        if (argc > 1)
            fclose(f);

        // Parameters - tuned for speed
        int tenure = max(7, min(20, N / 30));
        int maxIter = max(200, N / 2);
        int maxNoImprove = max(50, N / 8);
        int restartsPerReset = max(1, min(3, 100 / N)); // Restarts between resets
        int maxResets = 1;                              // Maximum number of resets
        int noImproveThreshold = restartsPerReset;      // Reset if no improvement after this many restarts

        // Multi-start Tabu Search with Reset mechanism
        Solution best;
        best.init();
        best.totalCost = INT_MIN;

        for (int reset = 0; reset < maxResets; reset++)
        {
            // Start with greedy solution
            Solution current = greedyInit();
            localSearch(current);
            current = tabuSearch(current, maxIter, maxNoImprove, tenure);

            if (current.totalCost > best.totalCost)
                best = current;

            int noImproveCount = 0;

            // Multiple restarts before next reset
            for (int r = 0; r < restartsPerReset; r++)
            {
                Solution s = randGreedy();
                localSearch(s);
                s = tabuSearch(s, maxIter, maxNoImprove, tenure);

                if (s.totalCost > best.totalCost)
                {
                    best = s;
                    noImproveCount = 0; // Reset counter on improvement
                }
                else
                {
                    noImproveCount++;
                }

                // Early exit if stuck in local optimum
                if (noImproveCount >= noImproveThreshold)
                    break;
            }

            // If we're improving, continue without full reset
            if (noImproveCount < noImproveThreshold && reset < maxResets - 1)
                continue;
        }

        localSearch(best);

        // Output
        vector<pair<int, int>> res;
        for (int k = 0; k < K; k++)
        {
            if (best.valid(k) && best.load[k] > 0)
            {
                for (int i = 0; i < N; i++)
                {
                    if (best.assign[i] == k)
                    {
                        res.push_back({i + 1, k + 1});
                    }
                }
            }
        }

        cout << res.size() << "\n";
        for (auto &p : res)
            cout << p.first << " " << p.second << "\n";
        cout.flush();

        return 0;
    }
    catch (...)
    {
        return 1;
    }
}
