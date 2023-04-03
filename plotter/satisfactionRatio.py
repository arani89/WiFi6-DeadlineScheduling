import matplotlib.pyplot as plt
import numpy as np

plt.rcParams.update({'font.size': 15})

satisfactionRatio=[0.240768, 1, 0.516948, 1, 1]

# create plot where the x axis is rpi, PC and server and the y axis is the runtime in seconds for each use case and the ftc
# the x axis is the use case number and the y axis is the runtime in seconds
# add a line parallel to the x axis which denotes realtime execution (1 second) 

x = ['Use\nCase-1', 'Use\nCase-2', 'Use\nCase-3', 'Use\nCase-4', 'Use\nCase-5']
y1 = np.array(satisfactionRatio)
plt.bar(x, y1, width=0.6, color='black')
plt.xlabel("Use Cases")
plt.ylabel("Satisfaction Ratio")
plt.grid()
plt.ylim(0, 1)
plt.tight_layout()
plt.savefig('SatisfactionRatio.pdf', bbox_inches='tight')