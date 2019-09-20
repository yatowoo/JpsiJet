#!/usr/bin/env python3
# Test for RooUnfold

import argparse

# Command-line Arguments
parser = argparse.ArgumentParser(description='Post-processing script for AliAnalysisTaskJpsiJet')
parser.add_argument('--raw',help='Outputs contains raw Z (from JpsiJetAna)', default="../JpsiJetAna.root")
parser.add_argument('--mc',help='Outputs contains raw Z (from JpsiJetAna --mc)', default="../JpsiJetMC.root")
parser.add_argument('-o', '--output',help='Output file path', default='UnfoldFF.root')
parser.add_argument('-p', '--print',help='Output file path', default='UnfoldFF.pdf')
args = parser.parse_args()

# ROOT libs
import ROOT
from ROOT import TFile, TCanvas, TLegend
from ROOT import RooUnfold, RooUnfoldBayes, RooUnfoldSvd, RooUnfoldResponse
import ana_util
from ana_util import ResponseNorm, SelectColor, SelectMarker, PrintCover

# Drawing
padFF = TCanvas('cFF','Unfolding', 1600, 600)
padFF.Divide(2)
padFF.Draw()

# Data outputs
raw = TFile(args.raw)
# MC outputs
mc = TFile(args.mc)

printFile = args.print
out = TFile(args.output, 'RECREATE')
PrintCover(padFF, printFile, 'Jpsi in jets analysis - Unfolding', )

def DrawFF(hist, name):
  hist.SetName(name)
  hist.Scale(1./hist.Integral('width'))
  histColor = next(ana_util.COLOR)
  hist.SetLineColor(histColor)
  hist.SetMarkerColor(histColor)
  hist.SetMarkerStyle(next(ana_util.MARKER))
  hist.Draw('same PE')
  return hist

def UnfoldFF(rawFF, detResponse, tag):
  padFF.Clear()
  padFF.Divide(2)
  # Detector response
  ResponseNorm(detResponse)
  padFF.cd(1)
  detResponse.Draw('COLZ')
  response = RooUnfoldResponse(rawFF, rawFF, detResponse)
  bayes = RooUnfoldBayes(response, rawFF)
  # Unfolding - Bayesian
  ana_util.COLOR = SelectColor(0)
  ana_util.MARKER = SelectMarker(0)
  padFF.cd(2)
  # Legend
  lgd = TLegend(0.12, 0.6, 0.3, 0.85)
  lgd.SetName('lgd' + tag)
  lgd.SetBorderSize(0)
  lgd.SetFillColor(0)
  # Z measured
  rawFF.SetBins(10, 0, 1.0)
  DrawFF(rawFF, 'hRaw_' + tag)
  lgd.AddEntry(rawFF, 'Raw')
  for nIter in range(4,5):
    bayes.SetIterations(nIter)
    hist = DrawFF(bayes.Hreco(0), 'hBayes' + repr(nIter) + '_' + tag)
    for ix in range(1,hist.GetNbinsX()):
      hist.SetBinError(ix, rawFF.GetBinError(ix))
    lgd.AddEntry(hist, 'Bayes (N=%d)' % nIter)
  lgd.Draw('same')
  padFF.Print(printFile, 'Title:'+tag)
  padFF.Write('c' + tag)

detMix = mc.hResponseFF_Prompt.Clone('hDetMix')
detMix.Add(mc.hResponseFF_Bdecay)
detMix.SetTitle('Detector response matrix - z (Combined)')

UnfoldFF(raw.hZPromptAfter, mc.hResponseFF_Prompt, 'Prompt')
UnfoldFF(raw.hZBdecayAfter, mc.hResponseFF_Bdecay, 'Bdecay')
UnfoldFF(raw.hZPromptAfter, detMix, 'PromptMix')
UnfoldFF(raw.hZBdecayAfter, detMix, 'BdecayMix')

padFF.Clear()
PrintCover(padFF, printFile, '', isBack=True)
out.Write()
out.Close()




