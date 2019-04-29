import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt
from collections import defaultdict
from matplotlib.colors import ListedColormap
import itertools
from scipy.signal import savgol_filter
from pathlib import Path
import math
from matplotlib.ticker import MaxNLocator
from collections import namedtuple
import statistics
from matplotlib.patches import Rectangle

plt.style.use('seaborn-white')
sns.set_style('whitegrid', {'legend.frameon':True})
#Direct input
plt.rcParams['text.latex.preamble']=[r"\usepackage{lmodern}"]
#Options
params = {'text.usetex' : True,
    'font.size' : 15,
        'font.family' : 'lmodern',
            'text.latex.unicode': True,
            }
plt.rcParams.update(params)

sns.set_palette("deep")
palette = sns.color_palette("deep", 6)


def read_file(name):
    qubits = []
    nodes = []
    time = []

    f = open(name, "r")
    for line in f.readlines():
        if "CONFIGURATION: " in line:
            split_line = line.split()
            N = int(split_line[3][:-1]) + 1
            n = int(split_line[6])
            n_ranks = 2 * nodes
        if "Avg time [ms] = " in line:
            split_line = line.split()
            print(split_line)
            t = int(split_line[4][:-1])
            nodes.insert(len(nodes), n)
            qubits.insert(len(qubits), N)
            time.insert(len(time), t)
    f.close()
    return qubits, nodes, time

def filter_case(case, qubits, nodes, time):
    nn = []
    tt = []
    for i, qubit in enumerate(qubits):
        if (qubit == case):
            nn.insert(len(nn), nodes[i])
            tt.insert(len(tt), time[i])
    return nn, tt

def scale_time(time1, time2):
    new_time1 = time1
    new_time2 = time2
    label = "time [ms]"
    if len(time1) > 0:
        if time1[0] >= 500:
            if len(time2) == 0 or (len(time2) > 0 and time2[0] >= 500):
                label = "time [s]"
                new_time1 = [1.0 * x / 1000.0 for x in time1]
                new_time2 = [1.0 * x / 1000.0 for x in time2]

    return new_time1, new_time2, label

(i_qubits, i_nodes, i_time) = read_file("../output-files/weak-dj-benchmark-intel.txt")
(q_qubits, q_nodes, q_time) = read_file("../output-files/weak-dj-benchmark-quest.txt")
i_time = [1.0 * x / 1000.0 for x in i_time]
q_time = [1.0 * x / 1000.0 for x in q_time]

# plotting
a = np.arange(1, 11)
fig = plt.figure(figsize=(4, 4))
fig.suptitle("DJ-algorithm, weak scaling")

time_label = "time [s]"

ax = fig.add_subplot(1, 1, 1)
ax.plot(a, i_time, label="Intel-QS")
ax.scatter(a, i_time)
ax.plot(a, q_time, label="QuEST")
ax.scatter(a, q_time)
ax.set_xlabel("number of nodes")
#ax.set_xticks([0, 2, 4, 8, 16, 32, 64, 128, 256, 512])
ax.set_xticks([0,1,2,3,4,5,6,8,10])
ax.set_xticklabels([0, 1, 2, 4, 8, 16, 32, 128, 512])
ax.set_ylabel(time_label)
ax.set_xlim(0, 11)
# ax.set_title("qubits = " + str(case))
ax.legend()
ax2 = ax.twiny()
ax2.set_xlabel("number of qubits")
#ax2.set_xticks([2, 8, 16, 32, 64, 128, 256, 512])
ax2.set_xticks([2, 4, 6, 8, 10])
ax2.set_xticklabels([30, 32, 34, 36, 38])
ax2.set_xlim(0, 11)

fig.tight_layout()
fig.subplots_adjust(top=0.86)
fig.show()
fig.savefig("../plots/dj-weak-scaling-new.pdf")

