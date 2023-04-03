# plot median and standard deviation of a list of data

from turtle import color
import matplotlib.pyplot as plt
import numpy as np
import scipy.stats as st
# take 100 integers from a file and store them in a list
data = np.loadtxt('satifactionResults.txt', dtype=float)

plt.rcParams.update({'font.size': 15})
# print(data)
uc1 = np.array(data[0])
uc2 = np.array(data[1])
uc3 = np.array(data[2])
uc4 = np.array(data[3])
uc5 = np.array(data[4])

# Slightly_median = np.median(slightly_poor_profit_ratio)
# Moderately_median = np.median(moderately_poor_profit_ratio)
# Very_median = np.median(very_poor_profit_ratio)

uc1_median = np.median(uc1)
uc2_median = np.median(uc2)
uc3_median = np.median(uc3)
uc4_median = np.median(uc4)
uc5_median = np.median(uc5)

uc1_ci = st.norm.interval(0.95, loc=uc1_median, scale=st.sem(uc1))
uc2_ci = st.norm.interval(0.95, loc=uc2_median, scale=st.sem(uc2))
uc3_ci = st.norm.interval(0.95, loc=uc3_median, scale=st.sem(uc3))
uc4_ci = st.norm.interval(0.95, loc=uc4_median, scale=st.sem(uc4))
uc5_ci = st.norm.interval(0.95, loc=uc5_median, scale=st.sem(uc5))

# slightly_poor_ci = st.norm.interval(0.95, loc=Slightly_median, scale=st.sem(slightly_poor_profit_ratio))
# moderately_poor_ci = st.norm.interval(0.95, loc=Moderately_median, scale=st.sem(moderately_poor_profit_ratio))
# very_poor_ci = st.norm.interval(0.95, loc=Very_median, scale=st.sem(very_poor_profit_ratio))

uc1_std = (uc1_ci[1]-uc1_ci[0])/2
uc2_std = (uc2_ci[1]-uc2_ci[0])/2
uc3_std = (uc3_ci[1]-uc3_ci[0])/2
uc4_std = (uc4_ci[1]-uc4_ci[0])/2
uc5_std = (uc5_ci[1]-uc5_ci[0])/2


# slightly_poor_std = (slightly_poor_ci[1]-slightly_poor_ci[0])/2
# moderately_poor_std = (moderately_poor_ci[1]-moderately_poor_ci[0])/2
# very_poor_std = (very_poor_ci[1]-very_poor_ci[0])/2

# materials = ['UC1', 'Moderately Poor', 'Very Poor']
materials = ['Use\nCase-1', 'Use\nCase-2', 'Use\nCase-3', 'Use\nCase-4', 'Use\nCase-5']
x_pos = np.arange(len(materials))
# CTEs = [Slightly_median, Moderately_median, Very_median]
CTEs = [uc1_median, uc2_median, uc3_median, uc4_median, uc5_median]
# error = [slightly_poor_std, moderately_poor_std, very_poor_std]
error = [uc1_std, uc2_std, uc3_std, uc4_std, uc5_std]

fig, ax = plt.subplots()
# change the bar color to be less bright blue
ax.bar(x_pos, CTEs, yerr=error, align='center', color='#1C7ED6', alpha=1, ecolor='#495057', capsize=10)
# ax.bar(x_pos, CTEs, align='center', color='#1C7ED6', alpha=1)
ax.set_ylabel('Resource Utilization (%)')
ax.set_xlabel('Use Cases')
ax.set_ybound(0,100)
ax.set_xticks(x_pos)
ax.set_xticklabels(materials)
ax.yaxis.grid(True)

# Save the figure and show
plt.tight_layout()
plt.savefig('resourceUtilization_100.pdf', bbox_inches='tight')
# plt.show()
