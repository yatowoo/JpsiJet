#!/usr/bin/env python3

import sys, os, json,argparse
from array import array

# Command-line Arguments
parser = argparse.ArgumentParser(description='Post-processing script for AliAnalysisTaskJpsiJet')
parser.add_argument('-f', '--file',help='Analysis results of AliAnalysisTaskJpsiJet', default="AnalysisResults.root")
parser.add_argument('-o', '--output',help='Output file path', default='JpsiJetAna.root')
parser.add_argument('-p', '--print',help='Print in PDF file', default='JpsiJetAna.pdf')
args = parser.parse_args()

import ROOT
import ana_util
from ana_util import *

TRIGGER_CLASSES = ['MB', 'EG1', 'EG2', 'DG1', 'DG2']

c = ROOT.TCanvas('cQA','ALICE Performance Figures',1280, 800)

fout = ROOT.TFile(args.output,'RECREATE')
PrintCover(c, args.print)

# Label - ALICE figure
c.cd(1)
pTxtALICE = ROOT.TPaveText(0.12, 0.75, 0.35, 0.88,"brNDC")
pTxtALICE.SetFillColor(0)
txt = pTxtALICE.AddText("ALICE Performance")
txt.SetTextFont(62) # Helvetica Bold
txt.SetTextAlign(13) # Top Left
def DrawALICE(x1, y1, x2, y2):
  pTxtALICE.SetX1NDC(x1)
  pTxtALICE.SetY1NDC(y1)
  pTxtALICE.SetX2NDC(x2)
  pTxtALICE.SetY2NDC(y2)
  pTxtALICE.Draw("same")

def DrawQA_Electron(output):
  CaloQA = {}
  c.Clear()
  c.SetWindowSize(1600, 600)
  c.Divide(2)
  lgdE = ROOT.TLegend(0.55, 0.65, 0.85, 0.85, "", "brNDC")
  lgdE.SetBorderSize(0)
  lgdRF = ROOT.TLegend(0.5, 0.12, 0.88, 0.45, "", "brNDC")
  lgdRF.SetBorderSize(0)
  for trig in TRIGGER_CLASSES:
    CaloQA[trig] = {}
    CaloQA[trig]['NEvent'] = 0
    CaloQA[trig]['E'] = None
    CaloQA[trig]['RF'] = None
    CaloQA[trig]['Eth'] = 11.0 if trig.count('1') else 6.0
    # Init
    qa = output.Get('QAhistos_' + trig)
    qa.SetOwner(True)
    # Event numbers
    evStats = qa.FindObject('EventStats') # TH1
    CaloQA[trig]['NEvent'] = evStats.GetBinContent(6)
    # Dielectron
    diele = qa.FindObject('Dielectron')
    diele.SetOwner(True)
    eleP = diele.FindObject('Track_ev1+')
    eleP.SetOwner(True)
    eleN = diele.FindObject('Track_ev1-')
    eleN.SetOwner(True)
    hE = eleP.FindObject('EMCalE').Clone('hE_' + trig)
    hE.Add(eleN.FindObject('EMCalE'))
    hE.Rebin(2)
    hE.Scale(1./CaloQA[trig]['NEvent'], 'width')
    CaloQA[trig]['E'] = hE
    # End
    qa.Delete("C")
  # Drawing
    # MB
  ana_util.SetColorAndStyle(CaloQA['MB']['E'], ROOT.kBlack, ROOT.kFullTriangleUp)
  CaloQA['MB']['E'].SetTitle('')
  CaloQA['MB']['E'].SetXTitle('#it{E} (GeV)')
  CaloQA['MB']['E'].SetYTitle('#frac{1}{#it{N}_{evts}} #frac{d#it{N}_{cluster}}{d#it{E}}')
  CaloQA['MB']['E'].GetXaxis().SetRangeUser(0., 30.)
  CaloQA['MB']['E'].GetYaxis().SetRangeUser(1e-9, 20)
  CaloQA['MB']['E'].GetYaxis().SetTitleOffset(1.5)
  CaloQA['MB']['E'].GetYaxis().SetTitleSize(0.03)
  c.cd(1)
  ROOT.gPad.SetLogy()
  CaloQA['MB']['E'].Draw('PE')
  lgdE.AddEntry(CaloQA['MB']['E'], 'Minimum bias trigger')
    # Gamma lower
  CaloQA['EG2']['E'].Add(CaloQA['DG2']['E'])
  ana_util.SetColorAndStyle(CaloQA['EG2']['E'], ROOT.kRed, ROOT.kFullCircle)
  CaloQA['EG2']['E'].Draw('same PE')
  lgdE.AddEntry(CaloQA['EG2']['E'], 'Trigger E_{EMC}^{Cluster} > 5 GeV')
    # Gamma higher
  CaloQA['EG1']['E'].Add(CaloQA['DG1']['E'])
  ana_util.SetColorAndStyle(CaloQA['EG1']['E'], ROOT.kBlue, ROOT.kFullSquare)
  CaloQA['EG1']['E'].Draw('same PE')
  lgdE.AddEntry(CaloQA['EG1']['E'], 'Trigger E_{EMC}^{Cluster} > 10 GeV')
  lgdE.Draw('same')
    # Label
  txt = pTxtALICE.AddText("pp, #it{#sqrt{s}} = 13 TeV")
  txt.SetTextFont(42) # Helvetica
  txt.SetTextAlign(13) # Top Left
  txt = pTxtALICE.AddText("|#it{#eta}| < 0.7")
  txt.SetTextFont(42) # Helvetica
  txt.SetTextAlign(13) # Top Left
  DrawALICE(0.15, 0.75, 0.35, 0.88)
  # Output
  c.Print(args.print, 'Title:Dielectron_E')
  ROOT.gPad.SaveAs("PERF_JpsiJet_ElectronEMCalE_pp13TeV.pdf")
  ROOT.gPad.SaveAs("PERF_JpsiJet_ElectronEMCalE_pp13TeV.eps")
  fout.cd()
  c.Write('cDieleE')

anaResult = ROOT.TFile(args.file)

anaOutput = anaResult.JpsiJetAnalysis

DrawQA_Electron(anaOutput)

PrintCover(c, args.print, isBack=True)

anaResult.Close()
fout.Close()
