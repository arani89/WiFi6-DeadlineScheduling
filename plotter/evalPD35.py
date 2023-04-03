#!/usr/bin/env python
from matplotlib import pyplot as plt
import numpy as np
 
plt.rcParams.update({'font.size': 15})
# get data from usecase 1:

score = 0
totp = 1
critp = 1

class UCdata:
	def __init__(self):
		self.heu = [-1, -1, -1]
		self.heu2 = [-1, -1, -1]
		self.edf = [-1, -1, -1]
		self.nlrf = [-1, -1, -1]
		self.lrf = [-1, -1, -1]
		self.totpackets = 0
		self.maxprofit = 0
	
	def heufill(self, arr):
		self.heu = arr

	def heu2fill(self, arr):
		self.heu2 = arr

	def edffill(self, arr):
		self.edf = arr

	def lrffill(self, arr):
		self.lrf = arr

	def nlrffill(self, arr):
		self.nlrf = arr


uc1 = UCdata()
uc2 = UCdata()
uc3 = UCdata()
uc4 = UCdata()
ftc = UCdata()
ucs = [uc1, uc2, uc3, uc4, ftc]

# read usecase 3
# def read_usecase(filename):
# 	with open(filename, 'r', encoding='utf-8',) as fi:
# 		f = []
# 		for temp in fi:
# 			if(len(temp) > 2):
# 				f.append(temp)
# 		line1 = str(f[len(f)-3].strip())
# 		print(line1)
# 		line1 = line1.split()
# 		line2 = str(f[len(f)-2].strip())
# 		print(line2)
# 		line2 = line2.split()
# 		if line1[len(line1)-1] == 'nan' : 
# 			val1 = 0 
# 		else: 
# 			val1 = float(line1[len(line1)-1])
		
# 		if line2[len(line2)-1] == 'nan' : 
# 			val2 = 0
# 		else:
# 			val2 = float(line2[len(line2)-1])
		
# 		return [val1, val2]

def read_usecase(filename, uc):
	with open(filename, 'r', encoding='utf-8') as fi:
		f = []
		for temp in fi:
			if(len(temp) > 2):
				f.append(temp)

		line = str(f[0])
		uc.maxprofit = int(line.split()[3])
		print(line.split())
		line = str(f[1])
		uc.totpackets = int(line.split()[3])
		line = str(f[3])
		# print(line, "heu")
		arr = [int(x) for x in line.strip().split()]
		print(arr, "arr")
		uc.heufill(arr)
		print(uc.heu[0])
		line = str(f[5])
		arr = [int(x) for x in line.strip().split()]
		uc.heu2fill(arr)
		line = str(f[7])
		arr = [int(x) for x in line.strip().split()]
		uc.edffill(arr)
		line = str(f[9])
		arr = [int(x) for x in line.strip().split()]
		uc.lrffill(arr)
		line = str(f[11])
		arr = [int(x) for x in line.strip().split()]
		uc.nlrffill(arr)
		# if(len(line) > 5):
		# 	arr = [int(x) for x in line.strip().split()]
		# 	uc.nlrffill(arr)


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

read_usecase('./output_new_1.txt', ucs[0])
read_usecase('./output_new_2.txt', ucs[1])
read_usecase('./output_new_3.txt', ucs[2])
read_usecase('./output_new_4.txt', ucs[3])
read_usecase('./output_factoryCase.txt', ucs[4])

# create data
x = ['LSDS', 'LSDSF', 'LRF', 'NLRF', 'EDF']

y1 = []
y2 = []

uc = ucs[3]
print(uc.heu[1])
y1.append(100*uc.heu[2]/uc.totpackets)
y1.append(100*uc.heu2[2]/uc.totpackets)
y1.append(100*uc.lrf[2]/uc.totpackets)
y1.append(100*uc.nlrf[2]/uc.totpackets)
y1.append(100*uc.edf[2]/uc.totpackets)

y2.append(100*(uc.heu[1]-uc.heu[2])/uc.totpackets)
y2.append(100*(uc.heu2[1]-uc.heu2[2])/uc.totpackets)
y2.append(100*(uc.lrf[1]-uc.lrf[2])/uc.totpackets)
y2.append(100*(uc.nlrf[1]-uc.nlrf[2])/uc.totpackets)
y2.append(100*(uc.edf[1]-uc.edf[2])/uc.totpackets)

y1 = np.array(y1)

# plot bars in stack manner
plt.bar(x, y1, color = '#FA5252', alpha=1)
plt.bar(x, y2, bottom=y1, color = '#1C7ED6', alpha=0.5)
# plt.xlabel("Algorithms")
plt.ylabel("% of Packets Dropped")
plt.ylim(0, 100)
plt.grid()
plt.legend(["Critical", "Non Critical"])
# plt.show()
plt.tight_layout()
# plt.show()
plt.savefig('percentageDroppedUC4_160.pdf', bbox_inches='tight')
