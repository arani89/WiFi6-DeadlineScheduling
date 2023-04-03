import matplotlib.pyplot as plt
import numpy as np

plt.rcParams.update({'font.size': 15})

runtimeFromRpi = [353, 23, 18, 2512, 10]
runtimeFromPc = [73, 5, 2, 498, 2]
runtimeFromServer = [126, 9, 10, 2412, 9]

# create plot where the x axis is rpi, PC and server and the y axis is the runtime in seconds for each use case and the ftc
# the x axis is the use case number and the y axis is the runtime in seconds
# add a line parallel to the x axis which denotes realtime execution (1 second) 

x = np.arange(5)

fig, ax = plt.subplots()
width = 0.25
plt.axhline(y = 200, color = 'r', linestyle = '-')
plt.bar(x-0.3, runtimeFromRpi, width)
plt.bar(x, runtimeFromPc, width)
plt.bar(x+0.3, runtimeFromServer, width)
plt.yscale('log')
plt.xticks(x, ('Use\nCase-1', 'Use\nCase-2', 'Use\nCase-3', 'Use\nCase-4', 'Use\nCase-5'))
plt.xlabel('Use case')
plt.ylabel('Runtime in (millisec)')
plt.legend(['Realtime(200ms)','Raspberry Pi', 'PC', 'Server'])
plt.savefig('runtimePlots.pdf', bbox_inches='tight')
plt.show()


