#!/usr/bin/env python3

import sys, os, time, json
import argparse

# Command-line arguments
parser = argparse.ArgumentParser(description='AliEn job running script for ALICE analysis tasks')
parser.add_argument('period',help='ALICE run period')
parser.add_argument('mode',help='Analysis mode : local, test, full, merge')
parser.add_argument('--tag',help='Job tags for masterjob and work dir', default='QM')
parser.add_argument('--date',help='Job date stamp (YYMMDD)', default='190920')
parser.add_argument('--mc',help='MC flag for DrawMC methods', default=False, action='store_true')
parser.add_argument('--nano',help='Forced to enable AOD filter task', default=False, action='store_true')
args = parser.parse_args()

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

year  = args.period[0:2]

datasets = args.period + '_pass1'
if(args.mc):
  if(year == '16'):
    mcProd = mc[args.period]
    data_dir = '2017/LHC' + mcProd
  elif(year == '17'):
    mcProd = mc[year]
    data_dir = '2018/LHC18b1a'
  elif(year == '18'):
    mcProd = mc[year]
    data_dir = '2019/LHC19c6'
  task_name = 'JpsiJetMC_' + args.tag + args.period
  work_dir = task_name + '_' + mcProd + '_' + args.date
else:
  data_dir = "20" + year + "/LHC" + args.period
  task_name = "JpsiJet_" + args.tag + args.period
  work_dir = task_name + '_' + args.date 

# Job infomation
print("\n\n======Processing "+work_dir+"======")
print('>>> Task\t: ' + task_name)
print('>>> Mode\t: ' + args.mode)
print('>>> Datasets\t: ' + datasets)
print('>>> Data dir.\t: ' + data_dir)
print('>>> Work dir.\t: ' + work_dir)

# Run option
doDevPWG          = 1
doMult            = 0
doEmcalCorrection = (0 if args.mode == 'local' else 1)
doJetQA           = 0
doJpsiQA          = 0
doJpsiFilter      = int(args.nano)
doPIDQA           = 0
doPhysAna         = 0


cmd = "aliroot -b -l -q -x \'runAnalysis.C(" \
    + repr(doDevPWG) + ','   \
    + repr(doMult) + ','   \
    + repr(doEmcalCorrection) + ','   \
    + repr(doJetQA) + ','   \
    + repr(doJpsiQA) + ','   \
    + repr(doJpsiFilter) + ','   \
    + repr(doPIDQA) + ','   \
    + repr(doPhysAna) + ','   \
    + '"' + args.mode + '",' \
    + '"' + datasets  + '",' \
    + '"' + data_dir  + '",' \
    + '"' + work_dir  + '",' \
    + '"' + task_name + '"' \
    + ")\'"
logfile = 'run' + work_dir + '_' + args.mode + '.log'
os.system(cmd + '| tee ' + logfile)
print("\n======End of "+work_dir+"======\n\n")
