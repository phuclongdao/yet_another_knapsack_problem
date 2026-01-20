import sys
import random
import time

POP_SIZE = 100
GENERATIONS = 2000
MUTATION_RATE = 0.1
PENALTY_WEIGHT = 500
TIME_LIMIT = 0.9

def calculate_fitness(individual, n, k, d, costs, c1, c2):
    loads = [0] * k
    total_cost = 0
    for i in range(n):
        v = individual[i]
        if v > 0:
            loads[v - 1] += d[i]
            total_cost += costs[i]
    penalty = 0
    for i in range(k):
        if loads[i] < c1[i]:
            penalty += (c1[i] - loads[i]) * PENALTY_WEIGHT
        elif loads[i] > c2[i]:
            penalty += (loads[i] - c2[i]) * PENALTY_WEIGHT
    return total_cost - penalty

def initialize_population(pop_size, n, k, d, c1, c2):
    population = []
    for _ in range(pop_size):
        ind = [0] * n
        order_indices = list(range(n))
        random.shuffle(order_indices)
        loads = [0] * k
        for v_idx in range(k):
            for i in range(len(order_indices) - 1, -1, -1):
                o_idx = order_indices[i]
                if loads[v_idx] < c1[v_idx]:
                    if loads[v_idx] + d[o_idx] <= c2[v_idx]:
                        ind[o_idx] = v_idx + 1
                        loads[v_idx] += d[o_idx]
                        order_indices.pop(i)
                else:
                    break
        for o_idx in order_indices:
            if random.random() < 0.2:
                v_rand = random.randint(0, k)
                if v_rand > 0:
                    if loads[v_rand - 1] + d[o_idx] <= c2[v_rand - 1]:
                        ind[o_idx] = v_rand
                        loads[v_rand - 1] += d[o_idx]
        population.append(ind)
    return population

def selection(population, scores):
    i1, i2 = random.sample(range(len(population)), 2)
    return list(population[i1] if scores[i1] > scores[i2] else population[i2])

def crossover(p1, p2):
    point = random.randint(1, len(p1) - 1)
    return p1[:point] + p2[point:], p2[:point] + p1[point:]

def mutate(individual, rate, k):
    for i in range(len(individual)):
        if random.random() < rate:
            individual[i] = random.randint(0, k)
    return individual

def solve():
    start_time = time.perf_counter()
    input_data = sys.stdin.read().split()
    if not input_data: return
    
    ptr = 0
    N = int(input_data[ptr]); ptr += 1
    K = int(input_data[ptr]); ptr += 1
    d, costs = [], []
    for _ in range(N):
        d.append(int(input_data[ptr])); ptr += 1
        costs.append(int(input_data[ptr])); ptr += 1
    c1, c2 = [], []
    for _ in range(K):
        c1.append(int(input_data[ptr])); ptr += 1
        c2.append(int(input_data[ptr])); ptr += 1

    population = initialize_population(POP_SIZE, N, K, d, c1, c2)
    scores = [calculate_fitness(ind, N, K, d, costs, c1, c2) for ind in population]
    
    best_ind = None
    best_fit = -float('inf')

    for gen in range(GENERATIONS):
        if time.perf_counter() - start_time > TIME_LIMIT:
            break
            
        new_children = []
        while len(new_children) < POP_SIZE:
            p1 = selection(population, scores)
            p2 = selection(population, scores)
            ch1, ch2 = crossover(p1, p2)
            new_children.append(mutate(ch1, MUTATION_RATE, K))
            new_children.append(mutate(ch2, MUTATION_RATE, K))
            
        child_scores = [calculate_fitness(ind, N, K, d, costs, c1, c2) for ind in new_children]
        combined = list(zip(population + new_children, scores + child_scores))
        combined.sort(key=lambda x: x[1], reverse=True)
        
        population = [x[0] for x in combined[:POP_SIZE]]
        scores = [x[1] for x in combined[:POP_SIZE]]
        
        if scores[0] > best_fit:
            best_fit = scores[0]
            best_ind = list(population[0])

    if best_ind:
        # print(f"total cost: {best_fit}")
        v_loads = [0] * K
        results = []
        for i in range(N):
            v = best_ind[i]
            if v > 0:
                v_loads[v - 1] += d[i]
                results.append((i + 1, v))
        
        valid = True
        for i in range(K):
            if v_loads[i] < c1[i] or v_loads[i] > c2[i]:
                valid = False
                break
        
        if valid:
            print(len(results))
            for oid, vid in results:
                print(f"{oid} {vid}")
        else:
            print(0)
    else:
        print(0)

if __name__ == "__main__":
    solve()