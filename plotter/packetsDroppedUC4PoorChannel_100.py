# plot median and standard deviation of a list of data

from turtle import color
import matplotlib.pyplot as plt
import numpy as np
import scipy.stats as st
# take 100 integers from a file and store them in a list
data = np.loadtxt('outputv4_new.txt', dtype=float)

plt.rcParams.update({'font.size': 15})
# print(data)
slightly_poor_nc = []
slightly_poor_c = []
moderately_poor_nc = []
moderately_poor_c = []
very_poor_nc = []
very_poor_c = []
# for i in range(0, len(data)):
    # print(len(data))
c=0
# print(data[i])
for j in data[0]:
    j = 100*j
    if c%2==0:
        slightly_poor_nc.append(j)
        c+=1
    else:
        slightly_poor_c.append(j)
        c+=1
c=0
for j in data[1]:
    j = 100*j
    if c%2==0:
        moderately_poor_nc.append(j)
        c+=1
    else:
        moderately_poor_c.append(j)
        c+=1
c=0
for j in data[2]:
    j = 100*j
    if c%2==0:
        very_poor_nc.append(j)
        c+=1
    else:
        very_poor_c.append(j)
        c+=1
print("hi I am here 1")
slightly_poor_c = np.array(slightly_poor_c)
slightly_poor_nc = np.array(slightly_poor_nc)
moderately_poor_c = np.array(moderately_poor_c)
moderately_poor_nc = np.array(moderately_poor_nc)
very_poor_c = np.array(very_poor_c)
very_poor_nc = np.array(very_poor_nc)

print("hi I am here 2")

Slightly_poor_median = np.median(slightly_poor_nc)
Moderately_poor_median = np.median(moderately_poor_nc)
Very_poor_median = np.median(very_poor_nc)
print("hi I am here 3")

slightly_poor_ci = st.norm.interval(0.95, loc=Slightly_poor_median, scale=st.sem(slightly_poor_nc))
moderately_poor_ci = st.norm.interval(0.95, loc=Moderately_poor_median, scale=st.sem(moderately_poor_nc))
very_poor_ci = st.norm.interval(0.95, loc=Very_poor_median, scale=st.sem(very_poor_nc))
print("hi I am here 4")

Slightly_poor_c = np.median(slightly_poor_c)
Moderately_poor_c = np.median(moderately_poor_c)
Very_poor_c = np.median(very_poor_c)

print("hi I am here 5")

slightly_poor_ci2 = st.norm.interval(0.95, loc=Slightly_poor_median, scale=st.sem(slightly_poor_c))
moderately_poor_ci2 = st.norm.interval(0.95, loc=Moderately_poor_median, scale=st.sem(moderately_poor_c))
very_poor_ci2 = st.norm.interval(0.95, loc=Very_poor_median, scale=st.sem(very_poor_c))

print("hi I am here 6")

slightly_poor_std = (slightly_poor_ci[1]-slightly_poor_ci[0])/2
moderately_poor_std = (moderately_poor_ci[1]-moderately_poor_ci[0])/2
very_poor_std = (very_poor_ci[1]-very_poor_ci[0])/2

slightly_poor_std2 = (slightly_poor_ci2[1]-slightly_poor_ci2[0])/2
moderately_poor_std2 = (moderately_poor_ci2[1]-moderately_poor_ci2[0])/2
very_poor_std2 = (very_poor_ci2[1]-very_poor_ci2[0])/2


materials = ['Slightly Poor', 'Moderately Poor', 'Very Poor']
x_pos = np.arange(len(materials))
CTEs = [Slightly_poor_median, Moderately_poor_median, Very_poor_median] # total - critical packets
y1 = [Slightly_poor_c, Moderately_poor_c, Very_poor_c] #critical packets dropped
error2 = [slightly_poor_std, moderately_poor_std, very_poor_std]
error1 = [slightly_poor_std2, moderately_poor_std2, very_poor_std2]
print(error1, "error1")
# error1 = [0, 0, 0, 0, 0]
print(error2, "error2")

fig, ax = plt.subplots()
ax.bar(x_pos, y1, width=0.3, yerr=error1, align='center', color='#FA5252', alpha=1, ecolor='#495057', capsize=10)
ax.bar(x_pos, CTEs, width=0.3, yerr=error2, align='center', color='#1C7ED6', alpha=0.5, ecolor='#495057', capsize=10, bottom=y1)
# ax.bar(x_pos, CTEs, align='center', color='grey', alpha=1)
ax.set_ylabel('% of Packets Dropped ')
plt.xlabel("Channel Quality")
ax.set_ybound(0,100)
ax.set_xticks(x_pos)
ax.set_xticklabels(materials)
ax.yaxis.grid(True)
plt.legend(["Critical", "Non Critical"], loc="upper right")
# Save the figure and show
plt.tight_layout()
plt.savefig('percentageDroppedUC4PoorChannel_new2.pdf', bbox_inches='tight')
# plt.show()
