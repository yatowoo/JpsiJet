#!/usr/bin/env python3

import json
import argparse
import sys
import os

period = {}
period['16'] = 'ghijklop'
period['17'] = 'hiklmor'
period['18'] = 'bdefklmop'

mc = {}
mc['16g'] = '17h2d'
mc['16h'] = '17h2e'
mc['16i'] = '17h2f'
mc['16j'] = '17h2g'
mc['16k'] = '17h2h'
mc['16l'] = '17h2i'
mc['16o'] = '17h2j'
mc['16p'] = '17h2k'
mc['17'] = '18b1a'
mc['18'] = '19c6'

data = sys.argv[1]
year  = data[0:2]
JOB_MODE = sys.argv[2]
MC = (len(sys.argv) > 3)

datasets = data + '_pass1'
if(MC):
  if(year == '16'):
    data_dir = '2017/LHC' + mc[data]
    work_dir = 'JpsiJetMC_QM' + data + '_' + mc[data] + '_190917'
  elif(year == '17'):
    data_dir = '2018/LHC18b1a'
    work_dir = 'JpsiJetMC_QM' + data + '_' + mc[year] + '_190917'
  elif(year == '18'):
    data_dir = '2019/LHC19c6'
    work_dir = 'JpsiJetMC_QM' + data + '_' + mc[year] + '_190917'
  task_name = 'JpsiJetMC_' + data
else:
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
if(MC):
  logfile = 'runQM_MC' + data + '_' + JOB_MODE + '.log'
else:
  logfile = 'runQM_' + data + '_' + JOB_MODE + '.log'
os.system(cmd + '| tee ' + logfile)
print("\n======End of "+data+"======\n\n")
