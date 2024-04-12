
import numpy as np

# lambda is the average rate of success (or the mean)
lambda_ = 80

# size is the number of random variates (which is 100 in this case)
size = 100

# Generate a Poisson distribution
poisson_distribution = np.random.poisson(lambda_, size)

print(poisson_distribution)