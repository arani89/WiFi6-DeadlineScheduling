# plot median and standard deviation of a list of data

from turtle import color
import matplotlib.pyplot as plt
import numpy as np
import scipy.stats as st
# take 100 integers from a file and store them in a list
data = np.loadtxt('outputUC4100.txt', dtype=float)

plt.rcParams.update({'font.size': 15})
# print(data)
lsds = []
lsdsc = []
lsds_10_loss = []
lsds_10_lossc = []
for i in data:
    lsds.append(i[0]*100)
    lsdsc.append(i[1]*100)
    lsds_10_loss.append(i[2]*100)
    lsds_10_lossc.append(i[3]*100)

print("hi I am here 1")
lsds = np.array(lsds)
lsdsc = np.array(lsdsc)
lsds_10_loss = np.array(lsds_10_loss)
lsds_10_lossc = np.array(lsds_10_lossc)
print("hi I am here 2")


Lsds_median = np.median(lsds)
Lsds_10_loss_median = np.median(lsds_10_loss)
print("hi I am here 3")

lsds_ci = st.norm.interval(0.95, loc=Lsds_median, scale=st.sem(lsds))
lsds_10_loss_ci = st.norm.interval(0.95, loc=Lsds_10_loss_median, scale=st.sem(lsds_10_loss))
print("hi I am here 4")


Lsds_c = np.median(lsdsc)
Lsds_10_loss_c = np.median(lsds_10_lossc)
print("hi I am here 5")

lsds_ci2 = st.norm.interval(0.95, loc=np.median(lsdsc), scale=st.sem(lsdsc))
lsds_10_loss_ci2 = st.norm.interval(0.95, loc=np.median(lsds_10_lossc), scale=st.sem(lsds_10_lossc))
print("hi I am here 6")


lsds_std = (lsds_ci[1]-lsds_ci[0])/2
lsds_10_loss_std = (lsds_10_loss_ci[1]-lsds_10_loss_ci[0])/2
lsds_std2 = (lsds_ci2[1]-lsds_ci2[0])/2
lsds_10_loss_std2 = (lsds_10_loss_ci2[1]-lsds_10_loss_ci2[0])/2

materials = ['LSDS', 'LSDS_rtLoss']
x_pos = np.arange(len(materials))

CTEs = [Lsds_median, Lsds_10_loss_median]# total - critical packets
y1 = [Lsds_c, Lsds_10_loss_c] #critical packets dropped

error2 = [lsds_std, lsds_10_loss_std]
          
error1 = [lsds_std2, lsds_10_loss_std]

print(error1, "error1")
# error1 = [0, 0]
print(error2, "error2")
# error2 = [0, 0]
fig, ax = plt.subplots()
ax.bar(x_pos, y1, yerr=error1, align='center', color='#FA5252', alpha=1, ecolor='#495057', capsize=10)
ax.bar(x_pos, CTEs, yerr=error2, align='center', color='#1C7ED6', alpha=0.5, ecolor='#495057', capsize=10, bottom=y1)
# ax.bar(x_pos, CTEs, align='center', color='grey', alpha=1)
ax.set_ylabel('% of Packets Dropped ')
ax.set_ybound(0,100)
ax.set_xticks(x_pos)
ax.set_xticklabels(materials)
ax.yaxis.grid(True)
plt.legend(["Critical", "Non Critical"], loc="upper right")
# Save the figure and show
plt.tight_layout()
plt.savefig('pctgDrpRtloss_UC4.pdf', bbox_inches='tight')
# plt.show()
