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
# arri = [1, 1, 1, 1, 6]              #arrival
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

z = p_model.continuous_var_matrix(T, T, lb = 0, ub = 1, name = 'z')
x = p_model.continuous_var_cube(k, T, T, lb = 0, ub = 1, name = 'x')
lam = p_model.continuous_var_list(k, lb = 0, ub = 1, name = 'lam')

# z[i,j] = 0 for i >= j
p_model.add_constraints((z[i,j] == 0) for i in range(T) for j in range(i+1));

# z[i,j] = 0 for j >= i+6
p_model.add_constraints((z[i,j] == 0) for i in range(T) for j in range(i+6, T));

# Equation 20 
p_model.add_constraints( sum(z[i,j] for i in range(mid+1) for j in range(mid+1,T)) <= 1 
                    for mid in range(T-1) );

# Equation 21 (k replaced with m)
p_model.add_constraints((sum(x[pkt,i,j] for pkt in range(k))) <= z[i,j]*m
                       for i in range(T) for j in range(T));

# Equation 22
p_model.add_constraints( sum( 
    (sum(x[pkt,i,j] for j in range(i, T))) for i in range(arri[pkt],math.floor(ded[pkt] - tdur[pkt]) + 1)
    ) == lam[pkt] for pkt in range(k) );

# Equation 23
# p_model.add_constraints((sum(x[pkt,i,j] for pkt in range(k))) <= m
#                        for i in range(T) for j in range(T));

# Equation 24
p_model.add_constraints( x[pkt,i,j] == 0 for pkt in range(k) 
                        for i in range(T) for j in range( min(i+math.ceil(tdur[pkt]), T)));


#-----------------------------------------------------------------------------
# Solve the model
#-----------------------------------------------------------------------------
obj_fn = sum((lam[pkt])*penalty[pkt] for pkt in range(k))
p_model.set_objective('max', obj_fn)

# p_model.print_information()

print("----------")
p_model.solve()
print(p_model.objective_value)
print("----------")


