import matplotlib.pyplot as plt
plt.rcParams["figure.figsize"] = (5.5,2.5)
y1 = [10, 5, 2.5, 1.25, 0.6, 0.3, 0.15, 0.07, 0.035, 0.02, 0.01, 0.005, 0.0025]
y2 = [2, 1, 0.5, 1.25, 0.6, 0.3, 0.15, 0.07, 0.035, 0.02, 0.01, 0.005, 0.0025]
x = [0.1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096]

y1 = [23.65/1, 23.65/5, 23.65/10, 23.65/50, 23.65/100, 23.65/200, 23.65/300, 23.65/400, 23.65/1000]
y2 = [2.6, 1.8, 1.5, 0.42, 0.3, 0.2, 0.15, 0.1, 0.04]
x = [0.1, 0.5, 1, 5, 10, 20, 30, 40, 100]

fig=plt.figure(figsize=(5,2.7))
ax = fig.add_subplot(1,1,1)
ax.set_xscale('log')
# ax.legend(loc='upper right', prop={'size': 12})
# ax.scatter(grades_range, girls_grades, color='b')
# ax.set_xlabel('Year')
# ax.set_ylabel('MCU cycles per bit TX at unit energy')
# ax.set_title('scatter plot')

# plt.plot(x,y,'o',color='b')
plt.plot(x,y1,linewidth=2.0,label='Embedded Agent CPU',zorder=2.5)
plt.plot(x,y2,linewidth=2.0,label='Standard Agent CPU',zorder=2.5)
plt.grid(zorder=2.5)
plt.legend(prop={'size': 10},bbox_to_anchor=(1, 1.03),facecolor='white',framealpha=1)
plt.xlabel('Ping Interval (s)',fontsize=12)
plt.ylabel('Utilization Overhead (%)',fontsize=12)
plt.xlim((0.1,100))
plt.ylim((0,25))
plt.yticks(fontsize=12)
plt.xticks(fontsize=12)


ax2 = ax.twinx()
ax2.plot(x, [34, 6.8, 3.4, 0.68, 0.34, 0.17, 0.113, 0.085, 0.034], '--', linewidth=2.0, label='Embedded Agent Power', color='r', zorder=2.5)
# ax2.set_ylim([0,50])
ax2.set_yscale('log')
plt.ylabel('Power (mW)', color='r', fontsize=12)
plt.grid(zorder=2.5)
plt.legend(prop={'size': 10},loc='upper right',bbox_to_anchor=(1, 0.8),facecolor='white',framealpha=1)
plt.yticks(fontsize=12)
# ax2.show()

# plt.show()
plt.savefig('agentutilization.png', bbox_inches='tight')
