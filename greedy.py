import sys

def greedy():
    input_data = sys.stdin.read().split()
    
    idx = 0
    N = int(input_data[idx])
    idx += 1
    K = int(input_data[idx])
    idx += 1
    
    d = []
    c = []
    orders = [] 
    for i in range(N):
        quantity = int(input_data[idx])
        cost = int(input_data[idx+1])
        d.append(quantity)
        c.append(cost)
        orders.append((cost, quantity, i))
        idx += 2
        
    c1 = []
    c2 = []
    for _ in range(K):
        c1.append(int(input_data[idx]))
        c2.append(int(input_data[idx + 1]))
        idx += 2
    
    orders.sort(key=lambda x: x[0], reverse=True)
    
    assignment = {}
    vehicle_load = [0] * K
    for cost, quantity, order_idx in orders:
        best_vehicle = -1
        best_score = (float('inf'), float('inf'))
        
        for k in range(K):
            new_load = vehicle_load[k] + quantity
            
            if new_load <= c2[k]:
                remaining_for_lower = max(0, c1[k] - new_load)
                utilization = new_load / c2[k] if c2[k] > 0 else 0
                
                if remaining_for_lower == 0:
                    score = (0, -utilization)
                else:
                    score = (1, remaining_for_lower)
                
                if score < best_score:
                    best_score = score
                    best_vehicle = k
        if best_vehicle != -1:
            assignment[order_idx + 1] = best_vehicle + 1
            vehicle_load[best_vehicle] += quantity
    
    valid_vehicles = set()
    for k in range(K):
        if c1[k] <= vehicle_load[k] <= c2[k]:
            valid_vehicles.add(k + 1)
    
    result = []
    for order_id in sorted(assignment.keys()):
        if assignment[order_id] in valid_vehicles:
            result.append((order_id, assignment[order_id]))
    
    print(len(result))
    for order_id, vehicle_id in result:
        print(f"{order_id} {vehicle_id}")

if __name__ == '__main__':
    greedy()

