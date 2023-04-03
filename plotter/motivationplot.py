# importing package
import matplotlib.pyplot as plt
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

# create data
x = ['Use\nCase-1', 'Use\nCase-2', 'Use\nCase-3', 'Use\nCase-4', 'Use\nCase-5']

y1 = []
y2 = []

for uc in ucs:
	y1.append(100*uc.edf[2]/uc.totpackets)
	y2.append(100*(uc.edf[1]-uc.edf[2])/uc.totpackets)

y1 = np.array(y1)
y2 = np.array(y2)
for i in range(len(y1)):
    print("Use Case ", i+1, " :")
    print("critical percentage : ",y1[i])
    print("total drop percentage : ",y1[i]+y2[i])
    print()
 
# plot bars in stack manner
plt.bar(x, y1, color='#FA5252')
plt.bar(x, y2, bottom=y1, color='#1C7ED6', alpha=0.5)
plt.xlabel("Use Cases")
plt.ylabel("% of Packets Dropped")
plt.legend(["Critical", "Non Critical"])
plt.grid()
plt.ylim(0,100)
# plt.show()

plt.tight_layout()
# plt.show()
plt.savefig('motivation_edf.pdf', bbox_inches='tight')