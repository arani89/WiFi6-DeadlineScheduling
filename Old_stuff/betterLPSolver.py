# -*- coding: utf-8 -*-
"""
Spyder Editor

"""
from docplex.mp.model import Model
import math
# import numpy as np

#-----------------------------------------------------------------------------
# Initialize the problem data
#-----------------------------------------------------------------------------



# penalty = [10, 2, 4, 5, 20]         # also known as profit
# arri = [1, 1, 1, 1, 3]              #arrival
# tdur = [1, 1, 1, 1, 3]              #transmission duration
# ded = [2, 2, 2, 2, 10]              #deadline 
# k = len(penalty)                     # no. of packets
# T = 50                    # total time period of a round (in ms)
# m = 2                     # no. of RUs

m, T, k = map(int, input().split())
penalty = []
arri = []
tdur = []
ded = []

for i in range(k):
    pen, ar, tdu, de = input().split()
    pen = int(pen)
    ar = int(ar)
    tdu = float(tdu)
    de = float(de)
    penalty.append(pen)
    arri.append(ar)
    tdur.append(tdu)
    ded.append(de)

#-----------------------------------------------------------------------------
# Build the model
#-----------------------------------------------------------------------------

p_model = Model(name = "opt partition")

delta = 6;
z = p_model.continuous_var_matrix(T, delta, lb = 0, ub = 1, name = 'z')
x = p_model.continuous_var_cube(k, T, delta, lb = 0, ub = 1, name = 'x')
lam = p_model.continuous_var_list(k, lb = 0, ub = 1, name = 'lam')

# for size = 0, the packet is non existant
p_model.add_constraints((z[i,0] == 0) for i in range(T));
for st in range(T-5, T):
    p_model.add_constraints((z[st,d] == 0) for d in range(T-st, delta))

# Equation 20 
p_model.add_constraints( sum( sum(z[i,d] for d in range(mid-i+1, delta)) for i in range(0, mid+1) ) <= 1
    for mid in range(T-1) );

# Equation 21 (k replaced with m)
p_model.add_constraints((sum(x[pkt,i,d] for pkt in range(k))) <= z[i,d]*m
                       for i in range(T) for d in range(delta));

# Equation 22
# i <= math.floor(ded[pkt] - tdur[pkt])
p_model.add_constraints( sum( 
    (sum(x[pkt,i,d] for d in range(delta))) for i in range(arri[pkt],min(math.floor(ded[pkt]-tdur[pkt]) + 1, T))
    ) == lam[pkt] for pkt in range(k) );

# Equation 24
p_model.add_constraints( x[pkt,i,d] == 0 for pkt in range(k) 
                        for i in range(T) for d in range( min(math.ceil(min(tdur[pkt], T-i)), delta)) );

#-----------------------------------------------------------------------------
# Solve the model
#-----------------------------------------------------------------------------
obj_fn = sum((lam[pkt])*penalty[pkt] for pkt in range(k))
p_model.set_objective('max', obj_fn)

# p_model.print_information()

print("----------")
p_model.solve()
p_model.print_solution()
print(p_model.objective_value)
print("----------")


