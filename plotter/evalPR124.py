# plot median and standard deviation of a list of data

from turtle import color
import matplotlib.pyplot as plt
import numpy as np
import scipy.stats as st
# take 100 integers from a file and store them in a list
data = np.loadtxt('outputUC2100_ProfitRatio.txt', dtype=float)

plt.rcParams.update({'font.size': 15})
# print(data)
lsds = np.array(data[4])
lrf = np.array(data[2])
nlrf = np.array(data[1])
edf = np.array(data[3])
lsdsf = np.array(data[0])

Lsds_median = np.median(lsds)
Lrf_median = np.median(lrf)
Nlrf_median = np.median(nlrf)
Edf_median = np.median(edf)
Lsdsf_median = np.median(lsdsf)

lsds_ci = st.norm.interval(0.95, loc=np.median(lsds), scale=st.sem(lsds))
lrf_ci = st.norm.interval(0.95, loc=np.median(lrf), scale=st.sem(lrf))
nlrf_ci = st.norm.interval(0.95, loc=np.median(nlrf), scale=st.sem(nlrf))
edf_ci = st.norm.interval(0.95, loc=np.median(edf), scale=st.sem(edf))
lsdsf_ci = st.norm.interval(0.95, loc=np.median(lsdsf), scale=st.sem(lsdsf))
print(lsds_ci)
print(lrf_ci)
print(nlrf_ci)
print(edf_ci)
print(lsdsf_ci)
lsds_std = (lsds_ci[1]-lsds_ci[0])/2
lrf_std = (lrf_ci[1]-lrf_ci[0])/2
nlrf_std = (nlrf_ci[1]-nlrf_ci[0])/2
edf_std = (edf_ci[1]-edf_ci[0])/2
lsdsf_std = (lsdsf_ci[1]-lsdsf_ci[0])/2

materials = ['LSDS', 'LSDSF', 'LRF', 'NLRF', 'EDF']
x_pos = np.arange(len(materials))
CTEs = [Lsds_median, Lsdsf_median, Lrf_median, Nlrf_median, Edf_median]
error = [lsds_std, lsdsf_std, lrf_std, nlrf_std, edf_std]

fig, ax = plt.subplots()
# change the bar color to be less bright blue
ax.bar(x_pos, CTEs, yerr=error, align='center',
       color='#1C7ED6', alpha=1, ecolor='#495057', capsize=10)
# ax.bar(x_pos, CTEs, align='center', color='#1C7ED6', alpha=1)
ax.set_ylabel('Profit Achieved/Profit Possible')
ax.set_ybound(0, 1)
ax.set_xticks(x_pos)
ax.set_xticklabels(materials)
ax.yaxis.grid(True)

# Save the figure and show
plt.tight_layout()
plt.savefig('evalUC2_new2.pdf', bbox_inches='tight')
# plt.show()
