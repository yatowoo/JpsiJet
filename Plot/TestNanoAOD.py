#!/usr/bin/env python3

# Test script for AliAOD.Dielectron*.root
# Based on ROOT6

import ROOT
from math import *
import sys
import os

if(len(sys.argv) == 1):
  aodFileName = "AliAOD.Dielectron.root"
else:
  aodFileName = sys.argv[1]

aodFile = ROOT.TFile(aodFileName)
aod = ROOT.AliAODEvent()
aod.ReadFromTree(aodFile.aodTree)

hOfflineTrigger = ROOT.TH1D("hOT","Offline Triggers",32,0,32)

for ev in aodFile.aodTree:
  trig = aod.GetHeader().GetOfflineTrigger()
  for offset in range(0,32):
    if(trig & 1<<offset):
      hOfflineTrigger.Fill(offset)

hOfflineTrigger.SaveAs("Trigger.root")
  
