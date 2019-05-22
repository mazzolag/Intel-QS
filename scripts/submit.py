import math
import os
import time
from subprocess import call

# strong benchmark values for DJ, RQC, Grover and Shor
n_nodes = [1, 2, 4, 8, 16, 32, 64, 128]
n_tasks_per_node = 2
n_threads_per_task = 18

# parameters
N_dj = [16, 19, 22, 24, 26, 27, 28, 29, 30, 31, 33]
nrep = 10
N_rqc = [4, 5, 6]
depth_rqc = [20, 40, 60]
nrep_rqc = 5



#########################
#    STRONG SCALING
#########################


def get_string(num_list):
    result = '('
    for x in num_list:
        result += str(x) + " "
    result += ')'
    return result

def get_string_list(str_list):
    result = "("
    for x in str_list:
        result += "\"" + x + "\"" + " "
    result += ")"
    print("Result = ", result)
    return result

n_time = "03:00:00"

template_file = "experiment.sh"

# create a new folder with current date and time
directory = str(time.strftime("%d:%m:%Y-%H:%M:%S"))
if not os.path.exists(directory):
    os.makedirs(directory)


scripts = []

for node_id, nodes in enumerate(n_nodes):
    tmpl_file = open(template_file, "r")
    output_name = directory + "/script_" + str(nodes) + ".sh"
    scripts.append(output_name)
    output_file = open(output_name, "w")

    for line in tmpl_file:
        line = line.replace("GLOBAL_TIME", n_time)
        line = line.replace("GLOBAL_NODES", str(nodes))
        line = line.replace("GLOBAL_TASKS", str(n_tasks_per_node))
        line = line.replace("GLOBAL_THREADS", str(n_threads_per_task))
        line = line.replace("GLOBAL_NREP", str(nrep))
        line = line.replace("GLOBAL_NREP_RQC", str(nrep_rqc))

        line = line.replace("GLOBAL_N_DJ", get_string(N_dj))
        line = line.replace("GLOBAL_N_RQC", get_string(N_rqc))
        line = line.replace("GLOBAL_DEPTH_RQC", get_string(depth_rqc))

        line = line.replace("GLOBAL_DIRECTORY", directory)

        output_file.write(line)

    output_file.close()
    tmpl_file.close()

# create the merge script to the directory folder (for merging results output files)
merge_file = "merge_files.py"
m_file = open(merge_file, "r")
m_output_name = directory + "/merge.py"
m_output = open(m_output_name, "w")

def get_python_array_string(arr):
    sarr = [str(x) for x in arr]
    return '[' + ','.join(sarr) + ']'

for line in m_file:
    line = line.replace("NODES", get_python_array_string(n_nodes))
    line = line.replace("GLOBAL_N_DJ", get_python_array_string(N_dj))
    line = line.replace("GLOBAL_N_RQC", get_python_array_string(N_rqc))
    line = line.replace("GLOBAL_DEPTH_RQC", get_python_array_string(depth_rqc))

    m_output.write(line)

m_output.close()
m_file.close()

# load modules and compile all
call("source ./load_compile.sh", shell=True)

for f in scripts:
    print("Submitting script " + f)
    call("chmod a+x " + f, shell=True)
    call("sbatch " + f, shell=True)

