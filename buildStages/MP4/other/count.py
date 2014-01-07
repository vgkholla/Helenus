#!/usr/bin/python
open_file = open("out", 'r')
words_list =[]
indexlist = []
contents = open_file.readlines()
for i in range(len(contents)):
     words_list.extend(contents[i].split())
open_file.close()
myset = set(words_list)
newlist = list(myset)

lineno = 0
t = len(newlist)
f = open('out.txt','w')
for i in range(len(newlist)):
    lineno = 0
    f.write(newlist[i])
    f.write(":index:")
    with open('out', 'r') as inF:
        for line in inF:
            lineno += 1
            for word in line.split():
                if newlist[i] == word:
                #indexlist.append(lineno)
                    print words_list[i]
                    print lineno
                    f.write(str(lineno))
                    f.write(",")
                    break
    f.write("\n")
    pec = (i / len(newlist)) * 100
#    print 'Status %age {} currect {} total {}'.format(pec, i, t)
    print pec
f.close()

