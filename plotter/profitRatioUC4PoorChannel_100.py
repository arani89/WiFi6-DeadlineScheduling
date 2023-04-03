# plot median and standard deviation of a list of data

from turtle import color
import matplotlib.pyplot as plt
import numpy as np
import scipy.stats as st
# take 100 integers from a file and store them in a list
data = np.loadtxt('outputUC4_ProfitRatio_PoorChannel_100.txt', dtype=float)

plt.rcParams.update({'font.size': 15})
# print(data)
slightly_poor_profit_ratio = np.array(data[0])
moderately_poor_profit_ratio = np.array(data[1])
very_poor_profit_ratio = np.array(data[2])

Slightly_median = np.median(slightly_poor_profit_ratio)
Moderately_median = np.median(moderately_poor_profit_ratio)
Very_median = np.median(very_poor_profit_ratio)

slightly_poor_ci = st.norm.interval(0.95, loc=Slightly_median, scale=st.sem(slightly_poor_profit_ratio))
moderately_poor_ci = st.norm.interval(0.95, loc=Moderately_median, scale=st.sem(moderately_poor_profit_ratio))
very_poor_ci = st.norm.interval(0.95, loc=Very_median, scale=st.sem(very_poor_profit_ratio))

slightly_poor_std = (slightly_poor_ci[1]-slightly_poor_ci[0])/2
moderately_poor_std = (moderately_poor_ci[1]-moderately_poor_ci[0])/2
very_poor_std = (very_poor_ci[1]-very_poor_ci[0])/2

materials = ['Slightly Poor', 'Moderately Poor', 'Very Poor']
x_pos = np.arange(len(materials))
CTEs = [Slightly_median, Moderately_median, Very_median]
error = [slightly_poor_std, moderately_poor_std, very_poor_std]

fig, ax = plt.subplots()
# change the bar color to be less bright blue
ax.bar(x_pos, CTEs, yerr=error, width=0.3, align='center', color='#1C7ED6', alpha=1, ecolor='#495057', capsize=10)
# ax.bar(x_pos, CTEs, align='center', color='#1C7ED6', alpha=1)
ax.set_ylabel('Profit Achieved/Profit Possible')
ax.set_xlabel('Channel Quality')
ax.set_ybound(0,1)
ax.set_xticks(x_pos)
ax.set_xticklabels(materials)
ax.yaxis.grid(True)

# Save the figure and show
plt.tight_layout()
plt.savefig('profitRatioUC4PoorChannel_100.pdf', bbox_inches='tight')
# plt.show()
