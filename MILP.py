import sys
from ortools.sat.python import cp_model

def milp():
    input_data = sys.stdin.read().split()
    
    idx = 0
    N = int(input_data[idx])
    idx += 1
    K = int(input_data[idx])
    idx += 1
    
    d = [] # quantity
    c = [] # cost
    for _ in range(N):
        d.append(int(input_data[idx]))
        c.append(int(input_data[idx+1]))
        idx += 2
        
    c1 = [] # lower bound
    c2 = [] # upper bound
    for _ in range(K):
        c1.append(int(input_data[idx]))
        c2.append(int(input_data[idx + 1]))
        idx += 2
    model = cp_model.CpModel()

    # parameter
    # x[i, k] = 1 if vehicle k serve order i
    x = {}
    for i in range(N):
        for k in range(K):
            x[i, k] = model.NewBoolVar(f'x_{i}_{k}')

    # constraint
    # Each order is served by at most one vehicle
    for i in range(N):
        model.Add(sum(x[i, k] for k in range(K)) <= 1)

    # Sum of quantity of orders served in a vehicle must be between the low-capacity and up-capacity of that vehicle
    for k in range(K):
        model.Add(sum(x[i, k] * d[i] for i in range(N)) >= c1[k])
        model.Add(sum(x[i, k] * d[i] for i in range(N)) <= c2[k])

    # Objective
    cost = sum(x[i, k] * c[i] for i in range(N) for k in range(K))
    model.Maximize(cost)

    solver = cp_model.CpSolver()
    status = solver.Solve(model)

    # Solution
    if status == cp_model.OPTIMAL or status == cp_model.FEASIBLE:
        print(f"total cost: {int(solver.ObjectiveValue())}")
        assigned_orders = []
        for i in range(N):
            for k in range(K):
                if solver.Value(x[i, k]) == 1:
                    assigned_orders.append((i + 1, k + 1))
        
        print(len(assigned_orders))
        
        for order_id, vehicle_id in assigned_orders:
            print(f"{order_id} {vehicle_id}")

if __name__ == '__main__':
    milp()
