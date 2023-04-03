# plot median and standard deviation of a list of data

from turtle import color
import matplotlib.pyplot as plt
import numpy as np
import scipy.stats as st
# take 100 integers from a file and store them in a list
data = np.loadtxt('outputv2.txt', dtype=float)

plt.rcParams.update({'font.size': 15})
# print(data)
lsds = []
lsdsc = []
lrf = []
lrfc = []
nlrf = []
nlrfc = []
edf = []
edfc = []
lsdsf = []
lsdsfc = []
for i in range(0, len(data)):
    # print(len(data))
    c=0
    # print(data[i])
    for j in data[i]:
        j = 100*j
        if c==0:
            lsds.append(j)
            c+=1
        elif c==1:
            lsdsc.append(j)
            c+=1
        elif c==4:
            lrf.append(j)
            c+=1
        elif c==5:
            lrfc.append(j)
            c+=1
        elif c==6:
            nlrf.append(j)
            c+=1
        elif c==7:
            nlrfc.append(j)
            c+=1
        elif c==2:
            edf.append(j)
            c+=1
        elif c==3:
            edfc.append(j)
            c+=1
        elif c==8:
            lsdsf.append(j)
            c+=1
        elif c==9:
            lsdsfc.append(j)
            c+=1

print("hi I am here 1")
lsds = np.array(lsds)
lsdsc = np.array(lsdsc)
lrf = np.array(lrf)
lrfc = np.array(lrfc)
nlrf = np.array(nlrf)
nlrfc = np.array(nlrfc)
edf = np.array(edf)
edfc = np.array(edfc)
lsdsf = np.array(lsdsf)
lsdsfc = np.array(lsdsfc)
print("hi I am here 2")


Lsds_median = np.median(lsds)
Lrf_median = np.median(lrf)
Nlrf_median = np.median(nlrf)
Edf_median = np.median(edf)
Lsdsf_median = np.median(lsdsf)
print("hi I am here 3")

lsds_ci = st.norm.interval(0.95, loc=Lsds_median, scale=st.sem(lsds))
lrf_ci = st.norm.interval(0.95, loc=Lrf_median, scale=st.sem(lrf))
nlrf_ci = st.norm.interval(0.95, loc=Nlrf_median, scale=st.sem(nlrf))
edf_ci = st.norm.interval(0.95, loc=Edf_median, scale=st.sem(edf))
lsdsf_ci = st.norm.interval(0.95, loc=Lsdsf_median, scale=st.sem(lsdsf))
print("hi I am here 4")


Lsds_c = np.median(lsdsc)
Lrf_c = np.median(lrfc)
Nlrf_c = np.median(nlrfc)
Edf_c = np.median(edfc)
Lsdsf_c = np.median(lsdsfc)
print("hi I am here 5")

lsds_ci2 = st.norm.interval(0.95, loc=np.median(lsdsc), scale=st.sem(lsdsc))
lrf_ci2 = st.norm.interval(0.95, loc=np.median(lrfc), scale=st.sem(lrfc))
nlrf_ci2 = st.norm.interval(0.95, loc=np.median(nlrfc), scale=st.sem(nlrfc))
edf_ci2 = st.norm.interval(0.95, loc=np.median(edfc), scale=st.sem(edfc))
lsdsf_ci2 = st.norm.interval(0.95, loc=np.median(lsdsfc), scale=st.sem(lsdsfc))
print("hi I am here 6")


lsds_std = (lsds_ci[1]-lsds_ci[0])/2
lrf_std = (lrf_ci[1]-lrf_ci[0])/2
nlrf_std = (nlrf_ci[1]-nlrf_ci[0])/2
edf_std = (edf_ci[1]-edf_ci[0])/2
lsdsf_std = (lsdsf_ci[1]-lsdsf_ci[0])/2
print(lsdsf_std, "lsdsf_std")
lsds_std2 = (lsds_ci2[1]-lsds_ci2[0])/2
lrf_std2 = (lrf_ci2[1]-lrf_ci2[0])/2
nlrf_std2 = (nlrf_ci2[1]-nlrf_ci2[0])/2
edf_std2 = (edf_ci2[1]-edf_ci2[0])/2
lsdsf_std2 = (lsdsf_ci2[1]-lsdsf_ci2[0])/2

materials = ['LSDS', 'LSDSF', 'LRF', 'NLRF', 'EDF']
x_pos = np.arange(len(materials))
CTEs = [Lsds_median, Lsdsf_median, Lrf_median, Nlrf_median, Edf_median] # total - critical packets
y1 = [Lsds_c, Lsdsf_c, Lrf_c, Nlrf_c, Edf_c] #critical packets dropped
error2 = [lsds_std, lsdsf_std, lrf_std, nlrf_std, edf_std]
error1 = [lsds_std2, lsdsf_std2, lrf_std2, nlrf_std2, edf_std2]
print(error1, "error1")
error1 = [0, 0, 0, 0, 0]
print(error2, "error2")

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
plt.savefig('percentageDroppedUC2_new2.pdf', bbox_inches='tight')
# plt.show()
