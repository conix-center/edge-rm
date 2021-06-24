import matplotlib.pyplot as plt
plt.rcParams["figure.figsize"] = (5.5,2)

y1 = [1,1,1,1,2,2,2,2,3,3,3,3,4,4,5,5,6,6,6,6,7,7,7,8,8]
x = [10,20,30,40,40,50,60,70,70,80,90,100,100,110,110,130,130,140,150,155,155,170,190,190,200]

iteration = [0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9]
time = [0,42,42,62,62,87,87,131,131,189,189,241,241,265,265,325,325,373,373,400]
time_m = [x / 60.0 for x in time]

fig=plt.figure()

# plt.plot(x,y,'o',color='b')
plt.plot(iteration,time_m,linewidth=2.0,label='Embedded Agent')
# plt.plot(x,y2,linewidth=2.0,label='Standard Agent')
# plt.legend(prop={'size': 12})
plt.grid()
plt.ylabel('Elapsed Time (m)')
plt.xlabel('Development Iteration')

plt.savefig('interactivity.png', bbox_inches='tight')