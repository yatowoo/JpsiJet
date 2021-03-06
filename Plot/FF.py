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
parser.add_argument('--lxyCut',nargs='+', help='Pseudo-proper decay length cut for prompt and non-prompt J/psi', type=float, default=(0.02,0.02))
parser.add_argument('--zCut',nargs='+', help='Range of fragmentation function', type=float, default=(0.4,1.0))
parser.add_argument('--zBin', help='Range of fragmentation function', type=int, default=6)
parser.add_argument('--more', help='More outputs for each canvas', default=False, action='store_true')
args = parser.parse_args()

# Analysis
import os, sys
import ana_util
from ana_util import *
import ana_phys
import ROOT
# Global cuts and variables
JET_PT_CUT_LOW  = float(args.jetCut[0])
JET_PT_CUT_UP   = float(args.jetCut[1])
JPSI_PT_CUT_LOW = max(5.0 if args.trig == 'L' else 10.0, args.jpsiCut[0])
JPSI_PT_CUT_UP  = min(float(args.jpsiCut[1]), JET_PT_CUT_UP)
JPSI_PROMPT_LXY = args.lxyCut[0]
JPSI_BDECAY_LXY = args.lxyCut[1]
ana_phys.PSEUDOLXY_PROMPT_CUT = JPSI_PROMPT_LXY
ana_phys.PSEUDOLXY_BDECAY_CUT = JPSI_BDECAY_LXY
JPSI_LXY_MAX    = 0.2
FF_Z_LOW        = args.zCut[0]
FF_Z_UP         = args.zCut[1]
FF_Z_BIN_WIDTH  = 0.1
FF_Z_BIN_N      = args.zBin

print("J/psi pT cut   : %.0f - %.0f (GeV/c)" % (JPSI_PT_CUT_LOW, JPSI_PT_CUT_UP))
print("Jet pT cut     : %.0f - %.0f (GeV/c)" % (JET_PT_CUT_LOW, JET_PT_CUT_UP))
print("Prompt region  : %.3f - %.3f (cm)" % (-JPSI_PROMPT_LXY, JPSI_PROMPT_LXY))
print("Non-prompt cut : %.3f - %.3f (cm)" % (JPSI_BDECAY_LXY, JPSI_LXY_MAX))

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
printPrefix = printName.replace('.pdf','')
# Drawing and printer
c = ROOT.TCanvas('cFF','J/#psi in jets analysis - FF', 800, 600)
ana_util.PrintCover(c, printFile)
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetPalette()
def PrintOut(c, title):
  c.Write('c' + title)
  c.Print(printFile, 'Title:' + title)
  if(args.more):
    c.SaveAs(printPrefix + '_' + title + '.pdf')
# Raw data and correction files
  # THnSparse - pT_ee, Mee, ~Lxy, z, R, pT_jet
ID_JPSI_PT, ID_JPSI_M, ID_JPSI_LXY, ID_Z, ID_R, ID_JET_PT = 0, 1, 2, 3, 4, 5
RAW = fData.Get('TagInfo'+args.trig)
  # Combine prompt and non-prompt signal
MC_SIGNAL = fMC.hJpsiPromptM.Clone('hMCsignal')
MC_SIGNAL.Add(fMC.hJpsiBdecayM)
  # Lxy sideband shape
fSB = ROOT.TFile('LxySBtest.root')
hLxySBtest = fSB.hLxySB.Rebin(5,"hLxySBtest")
# Basic cuts
  # Result range and binning
BINNING_FF_Z = [0.1*x for x in range(0,11,1)]
BINNING_FF_Z = array('d', BINNING_FF_Z)

BINNING_FF_JetPT = list(range(10,20,2))
BINNING_FF_JetPT += list(range(20,40,5))
BINNING_FF_JetPT = array('d', BINNING_FF_JetPT)
  # Default pT cuts
RAW.GetAxis(ID_JPSI_PT).SetRangeUser(JPSI_PT_CUT_LOW, JPSI_PT_CUT_UP)
RAW.GetAxis(ID_JET_PT).SetRangeUser(JET_PT_CUT_LOW, JET_PT_CUT_UP)
#RAW.GetAxis(ID_Z).SetRangeUser(0.0, 1.0)
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
PAVE_CUTS.AddText('|#eta_{jet}| < 0.5')
PAVE_CUTS.AddText('%.1f < p_{T,jet} < %.1f GeV/c' % (JET_PT_CUT_LOW, JET_PT_CUT_UP) )
def DrawCuts(PAVE_CUTS, xlow=0.13, ylow=0.4, xup=0.3, yup=0.55):
  PAVE_CUTS.SetX1NDC(xlow)
  PAVE_CUTS.SetX2NDC(xup)
  PAVE_CUTS.SetY1NDC(ylow)
  PAVE_CUTS.SetY2NDC(yup)
  PAVE_CUTS.Draw("same")
# Step 0 : Check jet pT
fout.cd()
c.Clear()
c.SetWindowSize(1600,600)
c.Divide(2)
c.cd(1)
ROOT.gPad.SetLogy()
RAW.GetAxis(ID_JET_PT).SetRangeUser(JPSI_PT_CUT_LOW, 50)
allJetPt  = RAW.Projection(ID_JET_PT)
allJetPt.SetName('hJetPtAll')
allJetPt = allJetPt.Rebin(len(ana_util.BINNING_JET_PT)-1, "", ana_util.BINNING_JET_PT)
allJetPt.Scale(1./allJetPt.Integral(),'width')
allJetPt.SetTitle('Raw p_{T} sepectra of all dielectron pair tagged jet')
allJetPt.SetXTitle('p_{T,jet} (GeV/c)')
allJetPt.SetYTitle('1/N_{jet} dN_{jet}/dp_{T}')
allJetPt.GetXaxis().SetRangeUser(JET_PT_CUT_LOW, JET_PT_CUT_UP)
ana_util.SetColorAndStyle(allJetPt, kBlack, kRound)
allJetPt.Draw("PE")
c.cd(2)
allJetPtZ = RAW.Projection(ID_Z, ID_JET_PT)
allJetPtZ.SetName('hJetPtZAll')
allJetPtZ.RebinX(2)
allJetPtZ.SetXTitle('p_{T,jet} (GeV/c)')
allJetPtZ.SetTitle('Dielectron pair tagged jet - z vs p_{T,jet}')
allJetPtZ.GetYaxis().SetRangeUser(0.0,1.0)
allJetPtZ.Draw("COLZ")
PrintOut(c, 'JetPt')
RAW.GetAxis(ID_JET_PT).SetRangeUser(JET_PT_CUT_LOW, JET_PT_CUT_UP)

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
Lxy = ana_phys.PseudoLxy(LxyData, fMC.hJpsiLxyPrompt, fMC.hJpsiLxyBdecay, hLxySBtest)
Lxy.result['Bkg'] = (0., Jpsi.result['SBfactor'][1] / Jpsi.result['SBfactor'][0])
Lxy.hData.Draw('PE')
Lxy.Fitting()
DrawCuts(PAVE_CUTS)
# End - Step 1
fout.cd()
PrintOut(c,'MLxy')

# Step 2: Raw FF, Sideband, After
c.Clear()
c.SetWindowSize(1600, 600)
c.Divide(2)
FF = {}
FF['Prompt'] = {}
FF['Prompt']['Color'] = kBlue
FF['Prompt']['Title'] = 'Prompt'
FF['Bdecay'] = {}
FF['Bdecay']['Color'] = kRed
FF['Bdecay']['Title'] = 'Non-prompt'
for i,tag in enumerate(['Prompt', 'Bdecay']):
  c.cd(i+1)
  CutJpsiPrompt((tag == 'Prompt'))
  FF[tag]['Legend'] = ROOT.TLegend(0.13, 0.62, 0.40, 0.80)
  FF[tag]['Legend'].SetName('lgd' + tag + 'RawFF')
  FF[tag]['Legend'].SetBorderSize(0)
  FF[tag]['Legend'].SetFillColor(0)
  # Total
  CutJpsiM()
  FF[tag]['TotalPtZ'] = RAW.Projection(ID_Z, ID_JPSI_PT)
  FF[tag]['TotalPtZ'].SetName('hPtZ' + tag + 'Total')
  FF[tag]['Total'] = RAW.Projection(ID_Z)
  FF[tag]['Total'].SetName('hFF' + tag + 'Total')
  ana_util.SetColorAndStyle(FF[tag]['Total'], ana_phys.PSEUDOLXY_TOTAL_COLOR, ana_util.kRound)
  FF[tag]['Total'].GetXaxis().SetRangeUser(0, 1.0)
  FF[tag]['Total'].Draw("PE1")
  FF[tag]['Legend'].AddEntry(FF[tag]['Total'], 'Total')
    # 2D result - z vs jet pT
  FF[tag]['Total2D'] = ana_util.Rebin2D(
    RAW.Projection(ID_Z, ID_JET_PT),
    BINNING_FF_JetPT,
    BINNING_FF_Z,
    'hFF' + tag + 'Total2D',
    "#it{p}_{T} ratio of J/#psi produced inside jet;#it{z}(#it{p}_{T,J/#psi}/#it{p}_{T,jet});#it{p}_{T,jet} (GeV/#it{c});1/N d^{2}N/d#it{z}d#it{p}_{T}")
  FF[tag]['Total2D'].Write()
  # Sideband
  CutJpsiM(Jpsi.result['Region']['SidebandL'][0], Jpsi.result['Region']['SidebandL'][1])
    # 2D result - z vs jet pT
  FF[tag]['SB2D'] = ana_util.Rebin2D(
    RAW.Projection(ID_Z, ID_JET_PT),
    BINNING_FF_JetPT,
    BINNING_FF_Z,
    'hFF' + tag + 'SB2D',
    "#it{p}_{T} ratio of J/#psi produced inside jet;#it{z}(#it{p}_{T,J/#psi}/#it{p}_{T,jet}) in sideband;#it{p}_{T,jet} (GeV/#it{c});1/N d^{2}N/d#it{z}d#it{p}_{T}")
  FF[tag]['SB2D'].Write()
    # 1D result
  FF[tag]['SBPtZ'] = RAW.Projection(ID_Z, ID_JPSI_PT)
  FF[tag]['SBPtZ'].SetName('hPtZ' + tag + 'SB')
  FF[tag]['SB'] = RAW.Projection(ID_Z)
  FF[tag]['SB'].SetName('hFF' + tag + 'SB')
  CutJpsiM(Jpsi.result['Region']['SidebandR'][0], Jpsi.result['Region']['SidebandR'][1])
  FF[tag]['SBPtZ'].Add(RAW.Projection(ID_Z, ID_JPSI_PT))
  FF[tag]['SBPtZ'].Scale(Jpsi.result['SBfactor'][0])
  FF[tag]['SB'].Add(RAW.Projection(ID_Z))
  ana_util.SetColorAndStyle(FF[tag]['SB'], ana_phys.PSEUDOLXY_BKG_COLOR, kRoundHollow)
  FF[tag]['SB'].Scale(Jpsi.result['SBfactor'][0])
  FF[tag]['SB'].Draw("SAME PE0")
  FF[tag]['Legend'].AddEntry(FF[tag]['SB'], 'Sideband')
  # Subtracted
    # DEBUG - 2D result
  FF[tag]['Signal2D'] = FF[tag]['Total2D'].Clone('hFF' + tag + 'Signal2D')
  FF[tag]['Signal2D'].Add(FF[tag]['SB2D'], -1.0)
  FF[tag]['Signal2D'].Write()
    # 1D result
  FF[tag]['SignalPtZ'] = FF[tag]['TotalPtZ'].Clone('hPtZ' + tag + 'Signal')
  FF[tag]['SignalPtZ'].Add(FF[tag]['SBPtZ'], -1.0)
  FF[tag]['Signal'] = FF[tag]['Total'].Clone('hFF' + tag + 'Signal')
  FF[tag]['Signal'].Add(FF[tag]['SB'], -1.0)
  ana_util.SetColorAndStyle(FF[tag]['Signal'], FF[tag]['Color'], kBlock)
  FF[tag]['Signal'].SetMarkerSize(2.5)
  FF[tag]['Signal'].Draw("SAME PE0")
  FF[tag]['Legend'].AddEntry(FF[tag]['Signal'], 'Total - SB')
  # Drawing
  FF[tag]['Label'] = ROOT.TPaveText(0.13, 0.8, 0.25, 0.88,"brNDC")
  FF[tag]['Label'].SetFillColor(0)
  FF[tag]['Label'].SetTextSize(0.04)
  FF[tag]['Label'].SetTextAlign(12) # Middle, Left
  txt = FF[tag]['Label'].AddText(FF[tag]['Title'])
  txt.SetTextFont(62)
  FF[tag]['Label'].Draw("same")
  FF[tag]['Legend'].Draw("SAME")
  DrawCuts(PAVE_CUTS, 0.13, 0.45, 0.35, 0.62)
PrintOut(c,'RawFF')

# Step 2: Efficiency correction
fMC.cJpsiEff.Draw()
fMC.cJpsiEff.Print(printFile, 'Title:JpsiEff')
fMC.cJpsiEff.Write('cJpsiEff')
fMC.cJpsiEff.Close()
  # Total
c.Clear()
c.SetWindowSize(1600, 600)
c.Divide(2)
ROOT.gStyle.SetPalette(ROOT.kInvertedDarkBodyRadiator)
c.cd(1)
FF['Prompt']['TotalPtZ'].SetTitle("Pair p_{T} vs Z(p_{T,ee}/p_{T,jet}) - Prompt (Total)")
FF['Prompt']['TotalPtZ'].GetYaxis().SetRangeUser(0., 1.0)
FF['Prompt']['TotalPtZ'].Draw("COLZ")
c.cd(2)
FF['Bdecay']['TotalPtZ'].SetTitle("Pair p_{T} vs Z(p_{T,ee}/p_{T,jet}) - Non-Prompt (Total)")
FF['Bdecay']['TotalPtZ'].GetYaxis().SetRangeUser(0., 1.0)
FF['Bdecay']['TotalPtZ'].Draw("COLZ")
PrintOut(c,'TotalPtZ')

def NormalizeFF(hZ):
  hZ.GetXaxis().SetRangeUser(0.,1.0)
  hZ.SetXTitle('z = (p_{T,J/#psi} / p_{T,jet})')
  hZ.GetYaxis().SetRangeUser(0.,10.)
  hZ.SetYTitle('1/N dN/dz')
  for iBin in range(1,hZ.GetNbinsX()):
    if(hZ.GetBinCenter(iBin) < FF_Z_LOW):
      hZ.SetBinContent(iBin, 0.)
      hZ.SetBinError(iBin, 0.)
  hZ.Scale(1./hZ.Integral(),'width')
for i,tag in enumerate(['Prompt', 'Bdecay']):
  c.Clear()
  c.SetWindowSize(1600,600)
  c.Divide(2)
  # Pt-Z
  c.cd(1)
  FF[tag]['SignalPtZ'].SetTitle("e^{+}e^{-} pair p_{T} vs Z(p_{T,ee}/p_{T,jet}) - Prompt (Total - SB)")
  FF[tag]['SignalPtZ'].GetYaxis().SetRangeUser(0., 1.0)
  FF[tag]['SignalPtZ'].SetMinimum(0.)
  FF[tag]['SignalPtZ'].Draw("COLZ")
  # FF
  c.cd(2)
  FF[tag]['Eff'] = fMC.Get('hJpsiEff' + tag)
  hEff = FF[tag]['Eff']
  FF[tag]['PtZ'] = FF[tag]['SignalPtZ'].Clone('hPtZ' + tag + 'Corrected')
  hPtZ = FF[tag]['PtZ']
  # Correction by pT bins
  for iBinZ in range(1,hPtZ.GetNbinsY()+1):
    for iBinPt in range(1,hPtZ.GetNbinsX()+1):
      rawVal = hPtZ.GetBinContent(iBinPt, iBinZ)
      if(rawVal < 1e-3):
        continue
      pt = hPtZ.GetXaxis().GetBinCenter(iBinPt)
      eff = hEff.GetBinContent(hEff.FindBin(pt))
      effErr = hEff.GetBinError(hEff.FindBin(pt))
      rawErr = hPtZ.GetBinError(iBinPt, iBinZ)
      hPtZ.SetBinContent(iBinPt, iBinZ, rawVal/eff)
      newErr = rawVal/eff * math.sqrt((rawErr/rawVal)**2 + (effErr/eff)**2)
      hPtZ.SetBinError(iBinPt, iBinZ, newErr)
  FF[tag]['Corrected'] = hPtZ.ProjectionY('hFF' + tag + 'Corrected')
  FF[tag]['Corrected'].SetTitle('FF after A #times #varepsilon correction - ' + tag)
  NormalizeFF(FF[tag]['Corrected'])
  NormalizeFF(FF[tag]['Signal'])
  # Drawing
  ana_util.ResetLegend(FF[tag]['Legend'], 0.15, 0.70, 0.30, 0.80)
  ana_util.SetColorAndStyle(FF[tag]['Signal'], kBlack, kRound)
  FF[tag]['Signal'].SetMarkerSize(1)
  FF[tag]['Signal'].Draw("PE1")
  FF[tag]['Legend'].AddEntry(FF[tag]['Signal'], 'RAW')
  ana_util.SetColorAndStyle(FF[tag]['Corrected'], FF[tag]['Color'], kBlock)
  FF[tag]['Corrected'].SetMarkerSize(2.5)
  FF[tag]['Corrected'].Draw("same PE1")
  FF[tag]['Legend'].AddEntry(FF[tag]['Corrected'], 'Corrected')
  FF[tag]['Label'].Draw("SAME")
  FF[tag]['Legend'].Draw('same')
  DrawCuts(PAVE_CUTS,0.15,0.4,0.35,0.65)
  # Output
  PrintOut(c, 'FF_' + tag + 'Corrected')

# Step 3: Non-prompt subtraction
if(args.trig == 'H'):
  NONPROMPT_RATIO = 0.395
elif(args.trig == 'L'):
  NONPROMPT_RATIO = 0.257
c.Clear()
c.SetWindowSize(800,600)
FF['Prompt']['Corrected2'] = FF['Prompt']['Corrected'].Clone('hFFPromptCorrected2')
FF['Prompt']['Corrected2'].Add(FF['Bdecay']['Corrected'], -NONPROMPT_RATIO)
FF['Bdecay']['Corrected'].Scale(NONPROMPT_RATIO)
FF['Bdecay']['Corrected'].SetMarkerSize(1.0)
FF['Prompt']['Corrected2'].Scale(1/FF['Prompt']['Corrected2'].Integral(), 'width')
FF['Prompt']['Corrected2'].SetTitle('Prompt FF after B-decay subtraction')
FF['Prompt']['Corrected2'].Draw('PE1')
ana_util.SetColorAndStyle(FF['Prompt']['Corrected'], kBlack, kBlock)
FF['Prompt']['Corrected'].SetMarkerSize(1.0)
FF['Prompt']['Corrected'].Draw('same PE1')
FF['Bdecay']['Corrected'].Draw('same PE1')
ana_util.ResetLegend(FF['Prompt']['Legend'], 0.15, 0.70, 0.30, 0.80)
FF['Prompt']['Legend'].AddEntry(FF['Prompt']['Corrected'],'Total')
FF['Prompt']['Legend'].AddEntry(FF['Bdecay']['Corrected'],'%.3f #times Non-prompt' % NONPROMPT_RATIO)
FF['Prompt']['Legend'].AddEntry(FF['Prompt']['Corrected2'],'Subtracted')
FF['Prompt']['Label'].Draw("SAME")
FF['Prompt']['Legend'].Draw('same')
DrawCuts(PAVE_CUTS,0.15,0.4,0.35,0.65)
PrintOut(c, 'FF_SubBdecay')


# End
c.Clear()
ana_util.PrintCover(c, printFile, isBack=True)
fData.Close()
fSB.Close()
fMC.Close()
fout.Close()