#!/usr/bin/env python3

import json
import argparse
import sys
import os

period = {}
period['16'] = 'ghijop'
period['17'] = 'rih'
period['18'] = 'bdefklop'

data = sys.argv[1]
year  = data[0:2]
JOB_MODE = sys.argv[2]

datasets = data + '_pass1'
data_dir = "20" + year + "/LHC" + data
work_dir = "JpsiJet_QM_" + data + "_190914"
task_name = "JpsiJet_" + data

cmd = "aliroot -b -l -q -x \'runAnalysis.C(1, 0, 1, 0, 0, 0, 0, 0," \
    + '"' + JOB_MODE + '",' \
    + '"' + datasets + '",' \
    + '"' + data_dir + '",' \
    + '"' + work_dir + '",' \
    + '"' + task_name + '"' \
    + ")\'"
print("\n\n======Processing "+data+"======")
os.system(cmd + '| tee runQM_' + data + '_' + JOB_MODE + '.log')
print("\n======End of "+data+"======\n\n")
