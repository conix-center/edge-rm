import matplotlib
import matplotlib.pyplot as plt
import numpy as np

plt.rcParams["figure.figsize"] = (5,3.2)
plt.rcParams['axes.axisbelow'] = True

width = 35  # the width of the bars

labels = ['1', '2', '3', '4', '5', '6', '7', '8', '9', '10']
labels_ints = [60 * int(l) - width/2 for l in labels]

men_means = [20, 35, 30, 35, 27, 20, 35, 30, 35, 27]
women_means = [25, 32, 34, 20, 25, 25, 32, 34, 20, 25]

x = np.arange(len(labels))  # the label locations

fig, ax = plt.subplots()
ax.bar(labels_ints, men_means, width, label='MapReduce', color='cornflowerblue')
ax.bar(labels_ints, women_means, width, label='SimpleSensor', bottom=men_means, color='green')
ax.set_ylim([0,100])
# ax.set_xlim([0,11])

# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel('Sensor Utilization')
ax.set_xlabel('Time (s)')
ax.yaxis.grid()
ax.set_xticks([60, 180, 300, 420, 540])
# ax.set_xticklabels([60 * l for l in labels_ints])
ax.legend(loc='upper right', prop={'size': 12})


# def autolabel(rects):
#     """Attach a text label above each bar in *rects*, displaying its height."""
#     for rect in rects:
#         height = rect.get_height()
#         ax.annotate('{}'.format(height),
#                     xy=(rect.get_x() + rect.get_width() / 2, height),
#                     xytext=(0, 3),  # 3 points vertical offset
#                     textcoords="offset points",
#                     ha='center', va='bottom')


# autolabel(rects1)
# autolabel(rects2)

fig.tight_layout()
# plt.show()
plt.savefig('cluster_util.png', bbox_inches='tight')