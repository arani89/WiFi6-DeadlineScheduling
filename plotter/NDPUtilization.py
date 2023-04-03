#!/usr/bin/env python
from matplotlib import pyplot as plt
import numpy as np
 
plt.rcParams.update({'font.size': 15})
# get data from usecase 1:


def read_usecase(filename):
	with open(filename, 'r', encoding='utf-8') as fi:
		f = []
		for temp in fi:
			if(len(temp) > 2):
				f.append(temp)

		line = str(f[0])
		throughput = float(line.strip().split()[2])
		line = str(f[2])
		resource_utilization = float(line.strip().split()[5])
		return [throughput, resource_utilization]


# plot a bar chart
# x = ['DPMSS']
# y = read_usecase('../outputs/output_usecase4.txt')
# plt.bar(x, y[0], width=0.1, color = '#FA5252', alpha=0.4)
# plt.bar(x, y[1], bottom=0, width=0.1, color = '#1C7ED6', alpha=0.8)
# plt.bar(x, y1, color = '#FA5252', alpha=1)
# plt.bar(x, y2, bottom=y1, color = '#1C7ED6', alpha=0.5)

# plt.bar(x, y2, bottom=y1, color = '#1C7ED6', alpha=0.5)
# decrease the width of the bars
# plt.xlabel("Algorithms")
# plt.ylabel("% of Packets Dropped")
# plt.xlim(-0.5, 0.5)
# plt.legend(["Critical", "Non Critical"])
# for i in range(len(x)):
# 	plt.text(x = i-0.055 , y = y[0] + 0.5, s = str(y[0]), size = 10)
# 	plt.text(x = i-0.055 , y = y[0] + y[1] + 0.5, s = str(y[1]), size = 10)

# # show the y - values on the bars
# plt.ylim(0, 100)

# plt.tight_layout()
# # plt.show()
# # save this plot as a pdf file
# plt.savefig('percentageDroppedUC4.pdf')

y1 = read_usecase('../outputs/old_output_usecase1.txt')
y2 = read_usecase('../outputs/old_output_usecase2.txt')
y3 = read_usecase('../outputs/old_output_usecase3.txt')
y4 = read_usecase('../outputs/old_output_usecase4.txt')
y5 = read_usecase('../outputs/old_output_factoryCase.txt')

# create data
x = ['Use\nCase-1', 'Use\nCase-2', 'Use\nCase-3', 'Use\nCase-4', 'Use\nCase-5']

y21 = [y1[0], y2[0], y3[0], y4[0], y5[0]]
y22 = [y1[1], y2[1], y3[1], y4[1], y5[1]]
# uc = ucs[3]
# print(uc.heu[1])
# y1.append(100*uc.heu[2]/uc.totpackets)
# y1.append(100*uc.lrf[2]/uc.totpackets)
# y1.append(100*uc.nlrf[2]/uc.totpackets)
# y1.append(100*uc.edf[2]/uc.totpackets)

# y2.append(100*(uc.heu[1]-uc.heu[2])/uc.totpackets)
# y2.append(100*(uc.lrf[1]-uc.lrf[2])/uc.totpackets)
# y2.append(100*(uc.nlrf[1]-uc.nlrf[2])/uc.totpackets)
# y2.append(100*(uc.edf[1]-uc.edf[2])/uc.totpackets)

y21 = np.array(y21)
print(y21)
# plot bars in stack manner
# plot log scale
# plt.yscale('log')
plt.bar(x, y22, width=0.6, color = '#1C7ED6', alpha=1)
plt.ylabel("NDP Resource Utilization Percentage")
plt.ylim(1, 100)
# plt.bar(x, y2, bottom=y1, color = '#1C7ED6', alpha=0.5)
plt.grid()
y22 = np.array(y22)
print(y22)
# ax2 = plt.twinx()
# ax2.set_ylabel("NDP Resource Utilization Percentage")
# ax2.bar(np.arange(5)+0.3, y22, width=0.3, color = '#1C7ED6', label='NDP Resource Utilization Percentage', alpha=1)
# ax2.set_ylim(0, 110)
# plt.legend(loc='upper right')
# plt.show()
plt.tight_layout()
# plt.show()
plt.savefig('NDPUtilization.pdf', bbox_inches='tight')
