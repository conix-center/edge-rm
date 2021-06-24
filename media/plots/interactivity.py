import matplotlib.pyplot as plt
plt.rcParams["figure.figsize"] = (5.5,3.3)

y1 = [1,1,1,2,2,2,3,3,3,4,5,6,6,6,7,7,8,8]
x = [10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180]

fig=plt.figure()

# plt.plot(x,y,'o',color='b')
plt.plot(x,y1,linewidth=2.0,label='Embedded Agent')
# plt.plot(x,y2,linewidth=2.0,label='Standard Agent')
# plt.legend(prop={'size': 12})
plt.grid()
plt.xlabel('Time (s)')
plt.ylabel('Development Iteration')

plt.savefig('interactivity.png', bbox_inches='tight')