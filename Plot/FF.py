#!/usr/bin/env python3

# Post-processing for J/psi tagged jets

from pprint import pprint
import argparse
# Command-line Arguments
parser = argparse.ArgumentParser(description='Test script for post-processing')
parser.add_argument('-f', '--file',help='Input file', default='AnaMerge_LHC16_Train.root')
parser.add_argument('--mc',help='MC signal file', default='MC16L.root')
parser.add_argument('-o','--output',help='ROOT file to store results', default='FF.root')
parser.add_argument('--trig',help='Trigger for analysis', default='L')
parser.add_argument('--jetCut',nargs='+', help='Jet pT cut', type=int, default=(15,50))
parser.add_argument('--jpsiCut',nargs='+', help='J/psi pT cut', type=int, default=(5,50))
parser.add_argument('--lxyCut',nargs='+', help='Pseudo-proper decay length cut for prompt and non-prompt J/psi', type=float, default=(0.01,0.01))
args = parser.parse_args()

# Global cuts and variables
JET_PT_CUT_LOW  = float(args.jetCut[0])
JET_PT_CUT_UP   = float(args.jetCut[1])
JPSI_PT_CUT_LOW = max(5.0 if args.trig == 'L' else 10.0, args.jpsiCut[0])
JPSI_PT_CUT_UP  = float(args.jpsiCut[1])
JPSI_PROMPT_LXY = args.lxyCut[0]
JPSI_BDECAY_LXY = args.lxyCut[1]
JPSI_LXY_MAX    = 0.3

print("J/psi pT cut   : %.0f - %.0f (GeV/c)" % (JPSI_PT_CUT_LOW, JPSI_PT_CUT_UP))
print("Jet pT cut     : %.0f - %.0f (GeV/c)" % (JET_PT_CUT_LOW, JET_PT_CUT_UP))
print("Prompt region  : %.3f - %.3f (cm)" % (-JPSI_PROMPT_LXY, JPSI_PROMPT_LXY))
print("Non-prompt cut : %.3f - %.3f (cm)" % (JPSI_BDECAY_LXY, JPSI_LXY_MAX))

# Analysis
import os, sys
import ana_util
from ana_util import *
import ana_phys
import ROOT
# I/O
fData = ROOT.TFile(args.file)
fMC   = ROOT.TFile(args.mc)
fout  = ROOT.TFile(args.output, "RECREATE")
outputDir = os.path.dirname(args.output)
if(outputDir == ''):
  outputDir = '.'
outputName = os.path.basename(args.output)
printName = outputName.replace('.root','.pdf')
printFile = outputDir + '/' + printName
# Drawing and printer
c = ROOT.TCanvas('cFF','J/#psi in jets analysis - FF', 800, 600)
ana_util.PrintCover(c, printFile)
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetPalette()

# Raw data and correction files
  # THnSparse - pT_ee, Mee, ~Lxy, z, R, pT_jet
RAW = fData.Get('TagInfo'+args.trig)
  # Combine prompt and non-prompt signal
MC_SIGNAL = fMC.hJpsiPromptM.Clone('hMCsignal')
MC_SIGNAL.Add(fMC.hJpsiBdecayM)
# Basic cuts
  # Defatul jet pt
RAW.GetAxis(5).SetRangeUser(JET_PT_CUT_LOW, JET_PT_CUT_UP)
def CutJpsiPt(ptMin, ptMax):
  RAW.GetAxis(0).SetRangeUser(ptMin, ptMax)
def CutJpsiM(
  mMin=ana_phys.JPSI_MASS_LOWER,
  mMax=ana_phys.JPSI_MASS_UPPER):
  RAW.GetAxis(1).SetRangeUser(ptMin, ptMax)
def CutJpsiPrompt(isPrompt=True):
  if(isPrompt):
    RAW.GetAxis(2).SetRangeUser(-JPSI_PROMPT_LXY, JPSI_PROMPT_LXY)
  else:
    RAW.GetAxis(2).SetRangeUser(JPSI_BDECAY_LXY, JPSI_LXY_MAX)
# Step 1 : Fitting invariant mass
HistM = RAW.Projection(1)
HistM.SetName('hM')
HistM.SetTitle('')
c.Clear()
c.SetWindowSize(1600, 600)
c.Divide(2)
c.cd(1)
Jpsi = ana_phys.ProcessInvMass(HistM, MC_SIGNAL)
Jpsi.hM.Draw("same PE")
# Step 1 : Fitting pseudo-proper decay length
fout.cd()
c.Write('cMLxy')
c.Print(printFile, 'Title:Fitting')

# End
c.Clear()
ana_util.PrintCover(c, printFile, isBack=True)
fData.Close()
fMC.Close()
fout.Close()