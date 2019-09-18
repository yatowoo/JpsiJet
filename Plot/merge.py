#!/usr/bin/env python3 -

# To merge results output

# EG1 + DG1, exclude EG1 in 16l
# EG2 + DG2

import ROOT
from ROOT import TFile

def AddPairVars(output, trig, pairVars):
  qa = output.Get('QAhistos_' + trig)
  qa.SetOwner(True)
  taggedQA = qa.FindObject('PairInJet')
  taggedQA.SetOwner(True)
  pVars = taggedQA.FindObject('PairVars')
  if(not pairVars):
    pairVars = pVars.Clone()
  else:
    pairVars.Add(pVars)
  qa.Delete()
  return pairVars

fileList = ['Ana16k.root' , 'Ana16l.root', 'Ana_LHC16.root']

outALL = None
outMB = None
outG1 = None
outG2 = None

for fileName in fileList:
  f = TFile(fileName)
  if(f.IsOpen()):
    print('[-] INFO - Processing ' + fileName)
  else:
    print('[-] ERROR - Fail to open ' + fileName)
    
  outALL = AddPairVars(f.JpsiJetAnalysis, 'ALL', outALL)
  outMB = AddPairVars(f.JpsiJetAnalysis, 'MB', outMB)
  if(fileName.find('16l') == -1):
    outG1 = AddPairVars(f.JpsiJetAnalysis, 'EG1', outG1)
  outG1 = AddPairVars(f.JpsiJetAnalysis, 'DG1', outG1)
  outG2 = AddPairVars(f.JpsiJetAnalysis, 'EG2', outG2)
  outG2 = AddPairVars(f.JpsiJetAnalysis, 'DG2', outG2)
  f.Close()
  # DEBUG
  outALL.Print()
  outMB.Print()
  outG1.Print()
  outG2.Print()

outALL.SaveAs('PairVars_LHC16l_ALL.root')
outMB.SaveAs('PairVars_LHC16l_MB.root')
outG1.SaveAs('PairVars_LHC16_G1.root')
outG2.SaveAs('PairVars_LHC16_G2.root')
