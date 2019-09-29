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
ID_JPSI_PT, ID_JPSI_M, ID_JPSI_LXY, ID_Z, ID_R, ID_JET_PT = 0, 1, 2, 3, 4, 5
RAW = fData.Get('TagInfo'+args.trig)
  # Combine prompt and non-prompt signal
MC_SIGNAL = fMC.hJpsiPromptM.Clone('hMCsignal')
MC_SIGNAL.Add(fMC.hJpsiBdecayM)
# Basic cuts
  # Default pT cuts
RAW.GetAxis(ID_JPSI_PT).SetRangeUser(JPSI_PT_CUT_LOW, JPSI_PT_CUT_UP)
RAW.GetAxis(ID_JET_PT).SetRangeUser(JET_PT_CUT_LOW, JET_PT_CUT_UP)
def CutJpsiPt(ptMin, ptMax):
  RAW.GetAxis(ID_JPSI_PT).SetRangeUser(ptMin, ptMax)
def CutJpsiM(
  mMin=ana_phys.JPSI_MASS_LOWER,
  mMax=ana_phys.JPSI_MASS_UPPER):
  RAW.GetAxis(ID_JPSI_M).SetRangeUser(mMin, mMax)
def CutJpsiPrompt(isPrompt=True):
  if(isPrompt):
    RAW.GetAxis(ID_JPSI_LXY).SetRangeUser(-JPSI_PROMPT_LXY, JPSI_PROMPT_LXY)
  else:
    RAW.GetAxis(ID_JPSI_LXY).SetRangeUser(JPSI_BDECAY_LXY, JPSI_LXY_MAX)
# Drawing methods
  # Unit: NDC
PAVE_CUTS = ROOT.TPaveText(0.15, 0.5, 0.35, 0.65, "brNDC")
PAVE_CUTS.SetName("pTxtCuts")
PAVE_CUTS.SetFillColor(0)
PAVE_CUTS.AddText('|y_{e^{+}e^{-}}| < 0.9')
PAVE_CUTS.AddText('%.1f < p_{T,e^{+}e^{-}} < %.1f GeV/c' % (JPSI_PT_CUT_LOW, JPSI_PT_CUT_UP) )
PAVE_CUTS.AddText('|#eta_{jet}| < 0.9')
PAVE_CUTS.AddText('%.1f < p_{T,jet} < %.1f GeV/c' % (JET_PT_CUT_LOW, JET_PT_CUT_UP) )
def DrawCuts(PAVE_CUTS, xlow=0.13, ylow=0.4, xup=0.3, yup=0.55):
  PAVE_CUTS.SetX1NDC(xlow)
  PAVE_CUTS.SetX2NDC(xup)
  PAVE_CUTS.SetY1NDC(ylow)
  PAVE_CUTS.SetY2NDC(yup)
  PAVE_CUTS.Draw("same")
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
DrawCuts(PAVE_CUTS)
# Step 1 : Fitting pseudo-proper decay length
c.cd(2)
ROOT.gPad.SetLogy()
CutJpsiM()
LxyData = RAW.Projection(ID_JPSI_LXY)
LxyData.SetName('hLxyRaw')
LxyData.SetTitle('')
LxyData.GetXaxis().SetRangeUser(-0.2, 0.2)
CutJpsiM(Jpsi.result['Region']['SidebandL'][0], Jpsi.result['Region']['SidebandL'][1])
LxyBkg = RAW.Projection(ID_JPSI_LXY)
LxyBkg.SetName('hLxySB')
CutJpsiM(Jpsi.result['Region']['SidebandR'][0], Jpsi.result['Region']['SidebandR'][1])
LxyBkg.Add(RAW.Projection(ID_JPSI_LXY))
LxyBkg.Scale(Jpsi.result['SBfactor'][0])
# Rebin to 0.05?
LxyData.Rebin(5)
fMC.hJpsiLxyPrompt.Rebin(5)
fMC.hJpsiLxyBdecay.Rebin(5)
LxyBkg.Rebin(5)
# Rebin
Lxy = ana_phys.PseudoLxy(LxyData, fMC.hJpsiLxyPrompt, fMC.hJpsiLxyBdecay, LxyBkg)
Lxy.result['Bkg'] = (0., Jpsi.result['SBfactor'][1] / Jpsi.result['SBfactor'][0])
Lxy.hData.Draw('PE')
Lxy.Fitting()
DrawCuts(PAVE_CUTS)
# End - Step 1
fout.cd()
c.Write('cMLxy')
c.Print(printFile, 'Title:Fitting')

# Step 2: Raw FF, Sideband, After
c.Clear()
c.SetWindowSize(1600, 600)
c.Divide(2)
FF = {}
FF['Prompt'] = {}
FF['Bdecay'] = {}
for i,tag in enumerate(['Prompt', 'Bdecay']):
  c.cd(i+1)
  CutJpsiPrompt((tag == 'Prompt'))
  FF[tag]['Legend'] = ROOT.TLegend(0.13, 0.68, 0.40, 0.88)
  FF[tag]['Legend'].SetName('lgd' + tag + 'RawFF')
  FF[tag]['Legend'].SetBorderSize(0)
  FF[tag]['Legend'].SetFillColor(0)
  # Total
  CutJpsiM()
  FF[tag]['Total'] = RAW.Projection(ID_Z)
  FF[tag]['Total'].SetName('hFF' + tag + 'Total')
  ana_util.SetColorAndStyle(FF[tag]['Total'], ana_phys.PSEUDOLXY_TOTAL_COLOR, ana_util.kRound)
  FF[tag]['Total'].Draw("PE0")
  FF[tag]['Legend'].AddEntry(FF[tag]['Total'], 'Total (%s)' % tag)
  # Sideband
  CutJpsiM(Jpsi.result['Region']['SidebandL'][0], Jpsi.result['Region']['SidebandL'][1])
  FF[tag]['SB'] = RAW.Projection(ID_Z)
  FF[tag]['SB'].SetName('hFF' + tag + 'SB')
  CutJpsiM(Jpsi.result['Region']['SidebandR'][0], Jpsi.result['Region']['SidebandR'][1])
  FF[tag]['SB'].Add(RAW.Projection(ID_Z))
  ana_util.SetColorAndStyle(FF[tag]['SB'], ana_phys.PSEUDOLXY_BKG_COLOR, kRoundHollow)
  FF[tag]['SB'].Scale(Jpsi.result['SBfactor'][0])
  FF[tag]['SB'].Draw("SAME PE0")
  FF[tag]['Legend'].AddEntry(FF[tag]['SB'], 'Sideband (%s)' % tag)
  # Subtracted
  FF[tag]['Signal'] = FF[tag]['Total'].Clone('hFF' + tag + 'Signal')
  FF[tag]['Signal'].Add(FF[tag]['SB'], -1.0)
  if(tag == 'Prompt'):
    SignalColor = ana_phys.PSEUDOLXY_PROMPT_COLOR
  else:
    SignalColor = ana_phys.PSEUDOLXY_BDECAY_COLOR
  ana_util.SetColorAndStyle(FF[tag]['Signal'], SignalColor, kStar)
  FF[tag]['Signal'].SetMarkerSize(3.0)
  FF[tag]['Signal'].Draw("SAME PE0")
  FF[tag]['Legend'].AddEntry(FF[tag]['Signal'], 'Total - SB (%s)' % tag)
  FF[tag]['Legend'].Draw("SAME")
  DrawCuts(PAVE_CUTS, 0.13, 0.45, 0.40, 0.65)
c.Print(printFile, 'Titel:RawFF')
c.Write('cRawFF')

# End
c.Clear()
ana_util.PrintCover(c, printFile, isBack=True)
fData.Close()
fMC.Close()
fout.Close()