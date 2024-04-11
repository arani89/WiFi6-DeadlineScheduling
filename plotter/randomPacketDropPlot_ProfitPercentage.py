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
lsds_10_loss = []
for i in data:
    lsds.append(i[0])
    lsds_10_loss.append(i[1])

Lsds_median = np.median(lsds)
Lsds_10_loss_median = np.median(lsds_10_loss)

lsds_ci = st.norm.interval(0.95, loc=np.median(lsds), scale=st.sem(lsds))
lsds_10_loss_ci = st.norm.interval(0.95, loc=np.median(
    lsds_10_loss), scale=st.sem(lsds_10_loss))

lsds_std = (lsds_ci[1]-lsds_ci[0])/2
lsds_10_loss_std = (lsds_10_loss_ci[1]-lsds_10_loss_ci[0])/2

materials = ['LSDS', 'LSDS_rtLoss']
x_pos = np.arange(len(materials))

CTEs = [Lsds_median, Lsds_10_loss_median]
error = [lsds_std, lsds_10_loss_std]
# error = [0, 0]
print(error, "error")
fig, ax = plt.subplots()
# change the bar color to be less bright blue
ax.bar(x_pos, CTEs, yerr=error, align='center',
       color='#1C7ED6', alpha=1, ecolor='#495057', capsize=10)
ax.set_ylabel('Profit Achieved/Profit Possible')
ax.set_ybound(0, 1)
ax.set_xticks(x_pos)
ax.set_xticklabels(materials)
ax.yaxis.grid(True)

# Save the figure and show
plt.tight_layout()
plt.savefig('evalUC4_rtLoss.pdf', bbox_inches='tight')
plt.show()