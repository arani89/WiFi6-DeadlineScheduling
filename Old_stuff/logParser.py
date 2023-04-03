file = open('output.txt')
outfile = open('betteroutput.txt','w')

content = file.readlines()

key = "Dropped MPDU (in AP Queue)\n"


sepa = '------------------------------CHECKPOINT-----------------------------------\n'
sepa2 = '---------------------------------END--------------------------------------\n'
sepa3 = '--------------------------------------------------------------------------\n'
validlines = []
for i in range(len(content)):
	if(content[i] == key):
		validlines.append(i)

outfile.write(str(len(validlines)))
outfile.write("\n")
outfile.write(sepa3)

for x in validlines:
	outfile.write(sepa)

	for i in range(0,18):
		outfile.write(content[x+i])
	for i in range(33,36):
		outfile.write(content[x+i])
	for i in range(58,66):
		outfile.write(content[x+i])

	outfile.write(sepa2)
	