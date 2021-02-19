import matplotlib.pyplot as plt
plt.rcParams["figure.figsize"] = (5.5,3.3)
y1 = [100, 97, 97, 95, 94, 90, 90, 86, 85, 83, 81]
y2 = [100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100]
x = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
fig=plt.figure()
ax = fig.add_subplot(1,1,1)
ax.set_ylim([0,110])
# ax.set_xscale('log')

# plt.plot(x,y,'o',color='b')
plt.plot(x,y1,linewidth=2.0,label='Firmware Update')
plt.plot(x,y2,linewidth=2.0,label='WebAssembly Task Update')
plt.legend(prop={'size': 12}, loc="lower right")
plt.grid()
plt.xlabel('Device Updates')
plt.ylabel('Device Availability (%)')
# plt.show()
plt.savefig('firmwareUpdates.png', bbox_inches='tight')