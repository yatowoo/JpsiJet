#!/usr/bin/env python3

# Script for runwise QA
# Input: Alien_work_directory/OutputAOD/$RUN/AnalysisResults.root

import ROOT
import os
import sys

if(len(sys.argv) == 1):
  work_dir = 'output/'
else:
  work_dir = sys.argv[1] + '/OutputAOD/'

gRootVer = ROOT.gROOT.GetVersion()
print('=' * 50)
print('\tJpsiJet Runwise QA\n  Python ' + sys.version[0:5] + ' + ROOT ' + gRootVer)
print('=' * 50)

for runDir in os.listdir(work_dir):
  if(not os.path.isdir(work_dir + runDir)):
    continue
  runID = int(runDir)
  print("[-] Run found : " + repr(runID))
  resultFile = ROOT.TFile(work_dir + runDir +'/AnalysisResults.root')
  # DEBUG - Test memory leak
  filterEventStat= resultFile.Get('PWGDQ_dielectronFilter/hEventStat')
  print('----> Event Number (EMCEGA) : '+repr(filterEventStat.GetBinContent(5)))
  filterQA = resultFile.Get('PWGDQ_dielectronFilter/jpsi_FilterQA')
  filterQA.SetOwner(True)
  eventQA = filterQA.FindObject('Event')
  eventQA.SetOwner(True)
  nPair = eventQA.FindObject('Npairs')
  print('----> Pair stat. : %d, %.3f' %(nPair.GetSum()-nPair.GetBinContent(1), nPair.GetMean()))
  resultFile.Close()
  filterQA.Delete("slow")
