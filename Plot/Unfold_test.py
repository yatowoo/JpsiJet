#!/usr/bin/env python3

import argparse
# Command-line Arguments
parser = argparse.ArgumentParser(description='Test script for post-processing')
parser.add_argument('-f', '--raw',help='Input file', default='FF_5GeV.root')
parser.add_argument('--mc',help='MC detector response', default='JpsiBJet_DetResponse_18.root')
parser.add_argument('--closure',help='MC sub-sample for closure test', default='JpsiBJet_DetResponse_16.root')
parser.add_argument('-o','--output',help='ROOT file to store results', default='Unfold.root')
parser.add_argument('--trig',help='Trigger for analysis', default='L', choices=['L', 'H'])
parser.add_argument('--type',help='J/psi type (Prompt-P, Non-prompt/B-decayed/B)', default='P', choices=['P', 'B', 'All'])
parser.add_argument('--cut', help='Apply user-defined cuts', default=False, action='store_true')
parser.add_argument('--jetCut',nargs='+', help='Jet pT cut', type=float, default=(15.0,35.0))
parser.add_argument('--jpsiCut',nargs='+', help='J/psi pT cut', type=float, default=(5.0,35.0))
parser.add_argument('--zCut',nargs='+', help='Range of fragmentation function', type=float, default=(0.4,1.0))
parser.add_argument('--more', help='More outputs for each canvas', default=False, action='store_true')
args = parser.parse_args()

import ROOT
import ana_util
import functools
from pprint import pprint

def ApplyCutZ(hFF):
  for i in range(1,5):
    hFF.SetBinContent(i, 0)
  hFF.Scale(1./hFF.Integral(),'width')

# Input data/MC files
fRM = ROOT.TFile(args.mc)
fRMClosure = ROOT.TFile(args.closure)
  # Prompt :    cFF_SubBdecay.FindObject('hFFPromptCorrected2')
  # Non-prompt: cFF_BdecayCorrected.FindObject('hFFBdecayCorrected')
  # Triggerr cuts:
  #  - 5 GeV EMCal low  ( 5 <pT,ee < 35, 15 < pT,jet < 35)
  #  - 10GeV EMCal high (10 <pT,ee < 35, 25 < pT,jet < 35)
fDet = ROOT.TFile(args.raw)
if(not args.cut and args.trig is 'H'):
  args.jetCut = [25.0, 35.0]
  args.jpsiCut = [10.0, 35.0]

pprint(vars(args))

# Output
fOut = ROOT.TFile(args.output,'RECREATE')
pdfOut = args.output.replace('.root','.pdf')
ROOT.gStyle.SetPalette(ROOT.kInvertedDarkBodyRadiator)
c = ROOT.TCanvas('cUnfold','Unfolding',1600,600)
ana_util.PrintCover(c, pdfOut, title='J/#psi in jets - Unfolding')
PrintPage = functools.partial(ana_util.PrintOut, canvas=c, printFile=pdfOut)

###
# Raw spectrum & Detector response
###
if(args.type is 'P'):
  detCanvas = fDet.cFF_SubBdecay
  hDet = detCanvas.FindObject('hFFPromptCorrected2').Clone('hDet')
else:
  detCanvas = fDet.cFF_BdecayCorrected
  hDet = detCanvas.FindObject('hFFBdecayCorrected').Clone('hDet')

RM_INFO = fRM.Jet_DetResponse # THnSparse
RM_INFO.GetAxis(0).SetRangeUser(0,1.0) # z-det
RM_INFO.GetAxis(1).SetRangeUser(0,1.0) # z-gen
RM_INFO.GetAxis(2).SetRangeUser(args.jetCut[0],args.jetCut[1]) # pT,jet-det
RM_INFO.GetAxis(3).SetRangeUser(args.jetCut[0],args.jetCut[1]) # pT,jet-gen
hRM = RM_INFO.Projection(1,0)
# Closure
RMClosure_INFO = fRMClosure.Jet_DetResponse # THnSparse
RMClosure_INFO.GetAxis(0).SetRangeUser(0,1.0) # z-det
RMClosure_INFO.GetAxis(1).SetRangeUser(0,1.0) # z-gen
RMClosure_INFO.GetAxis(2).SetRangeUser(args.jetCut[0],args.jetCut[1]) # pT,jet-det
RMClosure_INFO.GetAxis(3).SetRangeUser(args.jetCut[0],args.jetCut[1]) # pT,jet-gen
hRMClosure = RMClosure_INFO.Projection(1,0)
# Spectrum from MC
hDetClosure_raw = RMClosure_INFO.Projection(0)
hDetClosure = ROOT.TH1D("hDetClosure","Spectra from detector level (MC 2016)", 10, 0, 1)
hTrueClosure_raw = RMClosure_INFO.Projection(1)
hTrueClosure = ROOT.TH1D("hTrueClosure","Spectra from generator level (MC 2018)", 10, 0, 1)

hTrue = RM_INFO.Projection(1)
hTrue_input = ROOT.TH1D("hTrue","Spectra from generator", 10, 0, 1)
for i in range(1,11):
  hTrue_input.SetBinContent(i,hTrue.GetBinContent(i))
  hDetClosure.SetBinContent(i,hDetClosure_raw.GetBinContent(i))
  hTrueClosure.SetBinContent(i,hTrueClosure_raw.GetBinContent(i))

# Detector response
hRM_input = ROOT.TH2D("hRM","Response matrix", 10, 0, 1, 10, 0, 1)
response = ROOT.RooUnfoldResponse()
response.Setup(hDet, hTrue_input)
responseClosure = ROOT.RooUnfoldResponse()
responseClosure.Setup(hDetClosure, hTrueClosure)
# Fill RM
for i in range(1,11):
  for j in range(1,11):
    response.Fill(hDet.GetBinCenter(i),hTrue_input.GetBinCenter(j),hRM.GetBinContent(i,j))
    responseClosure.Fill(hDet.GetBinCenter(i),hTrue_input.GetBinCenter(j),hRM.GetBinContent(i,j))
# Draw detector response
c.SetWindowSize(1200, 1000)
c.cd()
response.Hresponse().Draw('COLZ')
PrintPage(title='RM')

###
# Unfolding - Bayes
###
N_ITERATIONS = 9
hDet.SetTitle('Unfolding and Refolding for #it{z} (LHC19i2a/2018)')
for nIter in range(1,N_ITERATIONS+1):
  bayes = ROOT.RooUnfoldBayes(response,hDet, nIter)
  # Unfolded spectra
  hUnfold = bayes.Hreco(0)
  hUnfold.SetName('hUnfold')
  ApplyCutZ(hUnfold)
  # Refold
  hRefold = response.ApplyToTruth(hUnfold)
  hRefold.SetName('hRefold')
  ApplyCutZ(hRefold)
  # Error
  for i in range(1,11):
    hUnfold.SetBinError(i, hDet.GetBinError(i))
    hRefold.SetBinError(i, hDet.GetBinError(i))
  # Style
  ana_util.COLOR = ana_util.SelectColor()
  ana_util.SetColorAndStyle(hDet, None, ROOT.kFullSquare, 1.5)
  ana_util.SetColorAndStyle(hUnfold, None, ROOT.kOpenCircle, 1.0)
  ana_util.SetColorAndStyle(hRefold, None, ROOT.kOpenSquare, 1.0)
  ana_util.SetColorAndStyle(hTrue_input, None, ROOT.kFullCircle, 1.5)
  ApplyCutZ(hTrue_input)
  # Drawing
    # Label
  label = detCanvas.FindObject('TPave').Clone('paveLable')
    # Cuts
  pTxtCuts = detCanvas.FindObject('pTxtCuts').Clone('paveTxtCuts')
  pTxtCuts.SetTextSize(0.035)
  pTxtCuts.SetTextFont(42)
  pTxtCuts.SetY1NDC(0.3)
  pTxtCuts.SetY2NDC(0.5)
    # Legend
  lgd = ROOT.TLegend(0.13, 0.55, 0.40, 0.80)
  lgd.SetName('lgdPromptRawFF')
  lgd.SetBorderSize(0)
  lgd.SetFillColor(0)
  lgd.SetTextSize(0.04)
  lgd.AddEntry(hDet,'Measured')
  lgd.AddEntry(hUnfold,'Unfolded (N_{bayes} = %d)' % nIter)
  lgd.AddEntry(hRefold,'Refolded (N_{bayes} = %d)' % nIter)
  lgd.AddEntry(hTrue_input,'MC (J/#psi in b-jets)')
    # Spectrum
  c.Clear()
  c.SetWindowSize(1000,1000)
  padFF, padRatio = ana_util.NewRatioPads(ROOT.gPad, "cFF", "cRatio")
  padFF.cd()
  hDet.GetYaxis().SetRangeUser(0, 3.5)
  hDet.Draw('PE1')
  hUnfold.Draw('SAME PE1')
  hRefold.Draw('SAME PE1')
  hTrue_input.Draw('SAME PE1')
  label.Draw('same')
  lgd.Draw('same')
  pTxtCuts.Draw('same')
    # Ratio
  padRatio.cd()
  padRatio.SetGrid(0,0)
  hDet.GetXaxis().SetLabelSize(0)
  hDet.GetXaxis().SetTitleSize(0)
  hDet.GetYaxis().SetTitleSize(0.05)
    # Ratio - Refold
  hDet.Sumw2()
  hRefold.Sumw2()
  hRatioRefold = hRefold.Clone('hRatioRefold')
  hRatioRefold.Divide(hDet)
  hRatioRefold.SetTitle('')
  hRatioRefold.SetYTitle('X / Measured')
  hRatioRefold.GetYaxis().SetNdivisions(505)
  ana_util.SetRatioPlot(hRatioRefold, 0., 2.)
  hRatioRefold.Draw('PE1')
    # Ratio - Unfold
  hUnfold.Sumw2()
  hRatioUnfold = hUnfold.Clone('hRatioUnfold')
  hRatioUnfold.Divide(hDet)
  hRatioUnfold.Draw('SAME PE1')
    # Lines
  padObjects = []
  for ratioVal in [0.7, 1.0, 1.3]:
    line = ROOT.TLine(0.0, ratioVal, 1.0, ratioVal)
    line.SetLineStyle(ROOT.kDashed)
    line.Draw('SAME')
    padObjects.append(line)
    # Print
  PrintPage(title='RefoldTest_Bayes' + repr(nIter))

# Closre test
hDet = hDetClosure
hDet.SetTitle('Unfolding closure test - RM from LHC19i2a/2018')
hTrue = hTrueClosure
for nIter in range(1,N_ITERATIONS+1):
  bayesClosure = ROOT.RooUnfoldBayes(responseClosure,hDet, nIter)
  # Unfolded spectra
  hUnfold = bayesClosure.Hreco(0)
  hUnfold.SetName('hUnfold')
  ApplyCutZ(hUnfold)
  # Refold
  hRefold = responseClosure.ApplyToTruth(hUnfold)
  hRefold.SetName('hRefold')
  ApplyCutZ(hRefold)
  # Style
  ana_util.COLOR = ana_util.SelectColor()
  ana_util.SetColorAndStyle(hDet, None, ROOT.kFullSquare, 1.5)
  ana_util.SetColorAndStyle(hUnfold, None, ROOT.kOpenCircle, 1.0)
  ana_util.SetColorAndStyle(hRefold, None, ROOT.kOpenSquare, 1.0)
  ana_util.SetColorAndStyle(hTrue, None, ROOT.kFullCircle, 1.5)
  ApplyCutZ(hDet)
  ApplyCutZ(hTrue)
  # Drawing
    # Label
  label = detCanvas.FindObject('TPave').Clone('paveLable')
    # Cuts
  pTxtCuts = detCanvas.FindObject('pTxtCuts').Clone('paveTxtCuts')
  pTxtCuts.SetTextSize(0.035)
  pTxtCuts.SetTextFont(42)
  pTxtCuts.SetY1NDC(0.3)
  pTxtCuts.SetY2NDC(0.5)
    # Legend
  lgd = ROOT.TLegend(0.13, 0.55, 0.40, 0.80)
  lgd.SetName('lgdPromptRawFF')
  lgd.SetBorderSize(0)
  lgd.SetFillColor(0)
  lgd.SetTextSize(0.04)
  lgd.AddEntry(hDet,'Detector level (LHC19i2c/2016)')
  lgd.AddEntry(hUnfold,'Unfolded (N_{bayes} = %d)' % nIter)
  lgd.AddEntry(hRefold,'Refolded (N_{bayes} = %d)' % nIter)
  lgd.AddEntry(hTrue,'Generator level / MC truth')
    # Spectrum
  c.Clear()
  c.SetWindowSize(1000,1000)
  padFF, padRatio = ana_util.NewRatioPads(ROOT.gPad, "cFF", "cRatio")
  padFF.cd()
  hDet.GetYaxis().SetRangeUser(0, 3.5)
  hDet.Draw('PE1')
  hUnfold.Draw('SAME PE1')
  hRefold.Draw('SAME PE1')
  hTrue.Draw('SAME PE1')
  label.Draw('same')
  lgd.Draw('same')
  pTxtCuts.Draw('same')
    # Ratio
  padRatio.cd()
  padRatio.SetGrid(0,0)
  hDet.GetXaxis().SetLabelSize(0)
  hDet.GetXaxis().SetTitleSize(0)
  hDet.GetYaxis().SetTitleSize(0.05)
    # Ratio - Unfold/Generator truth
  hDet.Sumw2()
  hUnfold.Sumw2()
  hRatioUnfold = hUnfold.Clone('hRatioUnfold')
  hRatioUnfold.Divide(hTrue)
  hRatioUnfold.SetTitle('')
  hRatioUnfold.SetYTitle('Unfolded / MC truth')
  hRatioUnfold.GetYaxis().SetNdivisions(502)
  ana_util.SetRatioPlot(hRatioUnfold, 0.5, 1.5)
  hRatioUnfold.Draw('PE1')
    # Lines
  padObjects = []
  for ratioVal in [0.9, 1.0, 1.1]:
    line = ROOT.TLine(0.0, ratioVal, 1.0, ratioVal)
    line.SetLineStyle(ROOT.kDashed)
    line.Draw('SAME')
    padObjects.append(line)
    # Print
  PrintPage(title='ClosureTest_Bayes' + repr(nIter))

# End
ana_util.PrintCover(c, pdfOut, isBack = True)
fOut.Close()
