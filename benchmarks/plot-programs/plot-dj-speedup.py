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
            #print(split_line)
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

(i_qubits, i_nodes, i_time) = read_file("../output-files/dj-benchmark-intel.txt")
(q_qubits, q_nodes, q_time) = read_file("../output-files/dj-benchmark-quest.txt")

# plotting
fig = plt.figure(figsize=(10, 10))
fig.suptitle("DJ-algorithm, strong scaling")

current_plot = 1

diff_qubits = sorted(set(i_qubits))
for c, case in enumerate(diff_qubits):
    if case in [17, 34]:
        continue
    i_nn, i_tt = filter_case(case, i_qubits, i_nodes, i_time)
    q_nn, q_tt = filter_case(case, q_qubits, q_nodes, q_time)

    i_tt, q_tt, time_label = scale_time(i_tt, q_tt)

    if c > 7:
        break

    i_T1 = i_tt[0]
    i_speedup = [i_T1 / x for x in i_tt]

    q_T1 = q_tt[0]
    q_speedup = [q_T1 / x for x in q_tt]

    ax = fig.add_subplot(3, 3, current_plot)
    current_plot = current_plot + 1
    ax.plot(i_nn, i_speedup, label="Intel-QS")
    ax.scatter(i_nn, i_speedup)
    ax.plot(q_nn, q_speedup, label="QuEST")
    ax.scatter(q_nn, q_speedup)
    ax.plot(i_nn, i_nn, linestyle='dashed', label='ideal', color='grey')
    ax.set_xlabel("number of nodes")
    ax.set_xticks([0, 2, 4, 8, 16])
    ax.set_ylabel("speedup ratio")
    ax.set_title("qubits = " + str(case))
    ax.legend(loc='upper left')

fig.tight_layout()
fig.subplots_adjust(top=0.86)
fig.savefig("../plots/dj-strong-speedup.pdf")

fig2 = plt.figure(figsize=(10, 10))
fig2.suptitle("DJ-algorithm, strong scaling")
current_plot=1
for c, case in enumerate(diff_qubits):
    if case in [17, 34]:
        continue
    i_nn, i_tt = filter_case(case, i_qubits, i_nodes, i_time)
    q_nn, q_tt = filter_case(case, q_qubits, q_nodes, q_time)

    i_tt, q_tt, time_label = scale_time(i_tt, q_tt)

    if c > 7:
        break

    i_T1 = i_tt[0]
    i_speedup = [i_T1 / x for x in i_tt]
    i_Eff = [i_speedup[k] / (2 ** k) for k in range(len(i_speedup))]

    q_T1 = q_tt[0]
    q_speedup = [q_T1 / x for x in q_tt]
    q_Eff = [q_speedup[k] / (2 ** k) for k in range(len(q_speedup))]

    a_ideal = np.ones_like(i_nn)

    ax = fig2.add_subplot(3, 3, current_plot)
    current_plot = current_plot + 1
    ax.plot(i_nn, i_Eff, label="Intel-QS")
    ax.scatter(i_nn, i_Eff)
    ax.plot(q_nn, q_Eff, label="QuEST")
    ax.scatter(q_nn, q_Eff)
    ax.plot(i_nn, a_ideal, linestyle='dashed', label='ideal', color='grey')
    ax.set_xlabel("number of nodes")
    ax.set_xticks([0, 2, 4, 8, 16])
    ax.set_ylabel("parallel efficiency")
    ax.set_title("qubits = " + str(case))
    ax.legend(loc='best')

fig2.tight_layout()
fig2.subplots_adjust(top=0.86)
fig2.savefig("../plots/dj-strong-parallelEff.pdf")


