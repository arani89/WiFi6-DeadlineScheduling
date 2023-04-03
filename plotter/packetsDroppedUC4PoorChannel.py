#!/usr/bin/env python
from matplotlib import pyplot as plt
import numpy as np
 
plt.rcParams.update({'font.size': 15})
# get data from usecase 1:

score = 0
totp = 1
critp = 1
# maxprofit1=0
# maxprofit2=0
# maxprofit3=0
# totalpackets1=0
# totalpackets2=0
# totalpackets3=0
# hscore1=0
# hscore2=0
# hscore3=0
# tdropped1=0
# tdropped2=0
# tdropped3=0
# cdropped1=0
# cdropped2=0
# cdropped3=0
# time1=0
# time2=0
# time3=0
def read_usecase(filename):
	with open(filename, 'r', encoding='utf-16') as fi:
		f = []
		for temp in fi:
			if(len(temp) > 2):
				f.append(temp)

		line = str(f[1])
		maxprofit1 = int(line.split()[3])
		# print(maxprofit1, "maxprofit1")
		line = str(f[2])
		totalpackets1 = int(line.split()[3])
		# print(totalpackets1, "totalpackets1")
		line = str(f[5])
		# print(line, "heu")
		arr = [int(x) for x in line.strip().split()]
		# print(arr, "arr")
		hscore1 = arr[0]
		tdropped1 = arr[1]
		cdropped1 = arr[2]
		# print(hscore1, "hscore1")
		# print(tdropped1, "tdropped1")
		# print(cdropped1, "cdropped1")
		# print(time1, "time1")
		# intentionally left blank
		line = str(f[9])
		print(line.strip(), " ----line")
		maxprofit2 = int(line.split()[3])
		# print(line.split())
		line = str(f[10])
		totalpackets2 = int(line.split()[3])
		line = str(f[13])
		# print(line, "heu")
		arr = [int(x) for x in line.strip().split()]
		# print(arr, "arr")
		hscore2 = arr[0]
		tdropped2 = arr[1]
		cdropped2 = arr[2]
		# intentionally left blank
		line = str(f[17])
		maxprofit3 = int(line.split()[3])
		# print(line.split())
		line = str(f[18])
		totalpackets3 = int(line.split()[3])
		line = str(f[21])
		# print(line, "heu")
		arr = [int(x) for x in line.strip().split()]
		# print(arr, "arr")
		hscore3 = arr[0]
		tdropped3 = arr[1]
		cdropped3 = arr[2]
		print(maxprofit1, maxprofit2, maxprofit3)
		print(totalpackets1, totalpackets2, totalpackets3)
		print(hscore1, hscore2, hscore3)
		return maxprofit1, maxprofit2, maxprofit3, totalpackets1, totalpackets2, totalpackets3, hscore1, hscore2, hscore3, tdropped1, tdropped2, tdropped3, cdropped1, cdropped2, cdropped3

[maxprofit1, maxprofit2, maxprofit3, totalpackets1, totalpackets2, totalpackets3, hscore1, hscore2, hscore3, tdropped1, tdropped2, tdropped3, cdropped1, cdropped2, cdropped3] = read_usecase('./output4.txt')

x = ['Slightly Poor', 'Moderately Poor', 'Very Poor']
y1 = []
y2 = []
y3 = []
y1.append(100*(cdropped1/totalpackets1))
y1.append(100*(cdropped2/totalpackets2))
y1.append(100*(cdropped3/totalpackets3))
y3.append(100*((tdropped1-cdropped1)/totalpackets1))
y3.append(100*((tdropped2-cdropped2)/totalpackets2))
y3.append(100*((tdropped3-cdropped3)/totalpackets3))
y2.append(hscore1/maxprofit1)
y2.append(hscore2/maxprofit2)
y2.append(hscore3/maxprofit3)
# make packet dropped second and profit ratio first
# plot the data
# plt.figure(figsize=(10, 6))

# plt.yscale('log')
plt.bar(x, y3, width=0.3, color='#1C7ED6', label='non-critical', alpha=0.5)
plt.bar(x, y1, width=0.3, color='#FA5252', label='critical', alpha=1)
plt.ylabel("% of Packets Dropped")
plt.xlabel("Channel Quality")
plt.legend(loc='upper right')
plt.ylim(0, 100)
plt.grid()
# ax2 = plt.twinx()
# plot the profit ratio bar on the 2nd y-axis and shift it by 0.1 on the x-axis to avoid overlap with the 1st y-axis data points 
# ax2.set_yscale('log')
# ax2.bar(np.arange(len(x))+0.3, y3, width=0.3, color='#1C7ED6', label='% non-critical packets dropped', alpha=0.5)
# ax2.bar(np.arange(len(x))+0.3, y1, width=0.3, color='#FA5252', label='% critical packets dropped', alpha=1)
# ax2.set_ylabel("% of Packets Dropped")
# ax2.legend(loc='upper right')
# plt.ylim(0, 100)
# plt.savefig('packetsDroppedUC4FreqInterference.pdf', bbox_inches='tight')

plt.savefig('packetsDroppedUC4PoorChannel.pdf', bbox_inches='tight')

# now plot another y-axis on the same graph with profit ratio as the 2nd y-axis
