# importing package
import matplotlib.pyplot as plt
import numpy as np
import scipy.stats as st
 
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

read_usecase('./output_new_1.txt', ucs[0])
read_usecase('./output_new_2.txt', ucs[1])
read_usecase('./output_new_3.txt', ucs[2])
read_usecase('./output_new_4.txt', ucs[3])
read_usecase('./output_factoryCase.txt', ucs[4])


uc = ucs[3]
print(uc.heu[0], uc.edf[0], uc.lrf[0], uc.nlrf[0])
materials = ['LSDS', 'LSDSF', 'LRF', 'NLRF', 'EDF']
x_pos = np.arange(len(materials))
CTEs = []

CTEs.append(uc.heu[0]/uc.maxprofit)
CTEs.append(uc.heu2[0]/uc.maxprofit)
CTEs.append(uc.lrf[0]/uc.maxprofit)
CTEs.append(uc.nlrf[0]/uc.maxprofit)
CTEs.append(uc.edf[0]/uc.maxprofit)

CTEs = np.array(CTEs)


fig, ax = plt.subplots()
# change the bar color to be less bright blue
# ax.bar(x_pos, CTEs, yerr=error, align='center', color='grey', alpha=1, ecolor='black', capsize=10)
ax.bar(x_pos, CTEs, align='center', color='#1C7ED6', alpha=1)
ax.set_ylabel('Profit Achieved/Profit Possible')
ax.set_ybound(0,1)
ax.set_xticks(x_pos)
ax.set_xticklabels(materials)
ax.yaxis.grid(True)

# Save the figure and show
plt.tight_layout()
plt.savefig('evalUC4_160.pdf', bbox_inches='tight')
# plt.show()
