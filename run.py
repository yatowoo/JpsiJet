#!/usr/bin/env python3

import json
import argparse
import sys
import os

CONFIG_FILE = "Job/AlienJob.json"
with open(CONFIG_FILE) as f:
  CONFIG = json.load(f)
  print("[-] INFO - Read configuration file: " + CONFIG_FILE)
  print(CONFIG.keys())

JOB_NAME = sys.argv[1]
JOB_MODE = sys.argv[2]

doMult = int(CONFIG[JOB_NAME]["doMult"])
doEmcalCorrection = int(CONFIG[JOB_NAME]["doEmcalCorrection"])
doJetQA = int(CONFIG[JOB_NAME]["doJetQA"])
doJpsiQA = int(CONFIG[JOB_NAME]["doJpsiQA"])
doJpsiFilter = int(CONFIG[JOB_NAME]["doJpsiFilter"])
doPIDQA = int(CONFIG[JOB_NAME]["doPIDQA"])
doPhysAna = int(CONFIG[JOB_NAME]["doPhysAna"])

cmd = "aliroot -b -l -q -x \'runAnalysis.C(" \
    + repr(doMult) + ',' \
    + repr(doEmcalCorrection) + ',' \
    + repr(doJetQA) + ',' \
    + repr(doJpsiQA) + ',' \
    + repr(doJpsiFilter) + ',' \
    + repr(doPIDQA) + ',' \
    + repr(doPhysAna) + ',' \
    + '"' + JOB_MODE + '",' \
    + '"' + CONFIG[JOB_NAME]["datasets"] + '",' \
    + '"' + CONFIG[JOB_NAME]["data_dir"] + '",' \
    + '"' + CONFIG[JOB_NAME]["work_dir"] + '",' \
    + '"' + CONFIG[JOB_NAME]["task_name"] + '"' \
    + ")\'"

print("[-] INFO - Read job configuration: " + JOB_NAME)
print(CONFIG[JOB_NAME])
print("[-] INFO - Job running mode : " + JOB_MODE) 
print("------> " + cmd)

os.system(cmd)
