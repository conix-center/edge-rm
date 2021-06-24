import matplotlib
import matplotlib.pyplot as plt
import numpy as np

plt.rcParams["figure.figsize"] = (5,2.5)
plt.rcParams['axes.axisbelow'] = True

labels = ['1', '2', '4', '16', '32']
men_means = [1.342,2.533,4.998,19.578,38.891]
women_means = [1.159668,2.288818,4.608154,18.402100,36.804]

x = np.arange(len(labels))  # the label locations
width = 0.35  # the width of the bars

fig, ax = plt.subplots()
rects1 = ax.bar(x - width-0.025, men_means, width, label='WebAssembly', color='cornflowerblue')
rects2 = ax.bar(x+0.025, women_means, width, label='Native',color='green')

# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel('Latency (ms)')
ax.set_xlabel('Number of Consecutive Sensor Accesses')
ax.yaxis.grid()
ax.set_xticks(x)
ax.set_xticklabels(labels)
ax.legend(loc='upper left', prop={'size': 12})


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
plt.savefig('wasi-sn-overhead.png', bbox_inches='tight')