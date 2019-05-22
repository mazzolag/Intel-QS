import math
import os
import datetime
from subprocess import call
import numpy as np

n_nodes = NODES
N_dj = GLOBAL_N_DJ
N_rqc = GLOBAL_N_RQC
depth_rqc = GLOBAL_DEPTH_RQC


exec_names = ["dj", "rqc"]

for exe in exec_names:
    N_size_name = "N_" + exe
    N_size = eval(N_size_name)
    output_name = exe + "_full.txt"
    exe_output = open(output_name, "w")
    cnt = 0
    temp_cnt = 0
    for N in N_size:

        for nodes in n_nodes:
            file_name = exe + "_" + str(nodes) + ".txt"
            file = open(file_name, "r")

            for i, line in enumerate(file):
                temp_cnt = i
                if i < cnt:
                    continue
                exe_output.write(line)
                if line.startswith("--"):
                    break

            if nodes == n_nodes[-1]:
                cnt = temp_cnt

            file.close()

    exe_output.close()

