import matplotlib.pyplot as plt
plt.rcParams["figure.figsize"] = (5.5,3.3)

y1 = [23.65*1.1, 23.65*1.0, 23.65*0.9, 23.65*0.8, 23.65*0.7, 23.65*0.6, 23.65*0.5, 23.65*0.4, 23.65*0.3, 23.65*0.2, 23.65*0.1]
y2 = [23.65*1.0, 23.65*0.9, 23.65*0.8, 23.65*0.7, 23.65*0.6, 23.65*0.5, 23.65*0.4, 23.65*0.3, 23.65*0.2, 23.65*0.1, 23.65*0.0]
y3 = [19] * 11
x = [0,10,20,30,40,50,60,70,80,90,100]

fig=plt.figure()
ax = fig.add_subplot(1,1,1)


# plt.plot(x,y,'o',color='b')
plt.plot(x,y1,linewidth=2.0,label='with EdgeRM')
plt.plot(x,y2,linewidth=2.0,label='without EdgeRM')
plt.plot(x,y3, '--', linewidth=2.0, color='r')
plt.legend(prop={'size': 12})
plt.grid()
plt.xlabel('Filter Rate (%)')
plt.ylabel('Power (nW)')

# plt.show()
plt.savefig('powertradeoff.png', bbox_inches='tight')