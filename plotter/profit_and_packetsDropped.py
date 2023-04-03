import matplotlib.pyplot as plt
import numpy as np

plt.rcParams.update({'font.size': 9})
# get data from usecase 1:

def read_usecase(filename):
	with open(filename, 'r', encoding='utf-8') as fi:
		f = []
		for temp in fi:
			if(len(temp) > 2):
				f.append(temp)

		line = str(f[0])
		maxprofit = int(line.split()[3])
		print(line.split())
		line = str(f[1])
		totalPackets = int(line.split()[3])
		line = str(f[3])
		# print(line, "heu")
		arr = [int(x) for x in line.strip().split()]
		currProfit = int(arr[0])
		totalPacketsDropped = int(arr[1])
		criticalPacketsDropped = int(arr[2])
		return [totalPackets, totalPacketsDropped, criticalPacketsDropped, currProfit, maxprofit]

x = ['DPMSS']
y = read_usecase('../outputs/old_output_usecase4.txt')
y1 = float(y[1]/y[0])
y2 = float(y[3]/y[4])
print(y1, " ,y1")
print(y2, " ,y2")
# draw a bar graph with 2 y-axis and 1 x-axis
fig, ax1 = plt.subplots()
ax2 = ax1.twinx()
line1, = ax1.bar(-1, y1, width=2, color = '#FA5252', alpha=1, label='Packets Dropped:Total Packets')
ax1.set_xlabel('DPMSS')
ax1.set_ylabel('Packets Dropped:Total Packets')
ax1.tick_params(axis='y', labelcolor='#FA5252')
ax1.set_ybound(lower=0, upper=1)
ax1.set_yticks(np.arange(0, 1.1, 0.1))


line2, = ax2.bar(1, y2, width=2, color = '#1C7ED6', alpha=1, label='Profit Achieved/Profit Possible')
ax2.set_ylabel('Profit Achieved/Profit Possible')
ax2.tick_params(axis='y', labelcolor='#1C7ED6')
ax2.set_yticks(np.arange(0, 1.1, 0.1))
# set the jump of y-axis to 0.1
ax2.set_ybound(lower=0, upper=1)


ax1.legend(loc='upper left')
# decrease the size of the legend

ax2.legend(loc='upper right')
fig.tight_layout()
plt.legend()
plt.xlim(-20, 20)
# dont show x-axis numbers
plt.xticks([])
plt.savefig('PercentageDrop_and_Profit_UC4.pdf', bbox_inches='tight')