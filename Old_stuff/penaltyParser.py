file = open('output.txt')
outfile = open('penalty.txt','w')
content = file.readlines()

key = "Dropped MPDU (in AP Queue)\n"


sepa = ') -------------------------------------------------------------------------\n'
sepa2 = '--------------------------------------------------------------------------\n'

validlines = []
for i in range(len(content)):
	if(content[i] == key):
		validlines.append(i)

outfile.write(str(len(validlines)))
outfile.write("\n")
outfile.write(sepa2)

prev = 0
ctr = 0
for x in validlines:
    ctr+=1
    outfile.write(str(ctr))
    outfile.write(sepa)
    # printing intervals
    for i in range(prev,x):
        if(content[i][0:8] == "interval"):
            outfile.write(content[i])

    #printing other data
    outfile.write(content[x+4])
    outfile.write(content[x+10])
    prev = x+66
    outfile.write(sepa2)
	