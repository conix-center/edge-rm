import matplotlib.pyplot as plt
plt.rcParams["figure.figsize"] = (5.5,3.3)
y = [7.52, 15.66, 28.83, 55.31, 21.5, 43.3, 100, 217, 184, 241]
x = [1998, 2004, 2005, 2012, 2015, 2016, 2017, 2018, 2019, 2020]
# fig=plt.figure()
# ax=fig.add_axes([0,0,1,1])
# ax.scatter(grades_range, girls_grades, color='b')
# ax.set_xlabel('Year')
# ax.set_ylabel('MCU cycles per bit TX at unit energy')
# ax.set_title('scatter plot')
plt.plot(x,y,'o',color='b')
plt.grid()
plt.xlabel('Year')
plt.ylabel('MCU cycles per bit TX at unit energy')
# plt.show()
plt.savefig('energy_compute_vs_radio.png', bbox_inches='tight')