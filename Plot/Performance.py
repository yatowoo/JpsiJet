#!/usr/bin/env python3

import sys, os, json,argparse
from array import array

# Command-line Arguments
parser = argparse.ArgumentParser(description='Post-processing script for AliAnalysisTaskJpsiJet')
parser.add_argument('-f', '--file',help='Analysis results of AliAnalysisTaskJpsiJet', default="AnalysisResults.root")
parser.add_argument('-o', '--output',help='Output file path', default='JpsiJetAna.root')
parser.add_argument('-p', '--print',help='Print in PDF file', default='JpsiJetAna.pdf')
parser.add_argument('--calo',help='Plot calo QA figure', default=False, action='store_true')
parser.add_argument('--invmassH',help='Plot figure of invariant mass spectrum (EMCal high)', default=False, action='store_true')
parser.add_argument('--invmassL',help='Plot figure of invariant mass spectrum (EMCal low)', default=False, action='store_true')
parser.add_argument('--eff',help='Plot figure of J/psi efficiency', default=False, action='store_true')
parser.add_argument('--map',help='Plot figure of J/psi-jet correlation map', default=False, action='store_true')

args = parser.parse_args()

import ROOT
import ana_util
from ana_util import *
import ana_phys

args.print = args.output.replace('.root','.pdf')

TRIGGER_CLASSES = ['MB', 'EG1', 'EG2', 'DG1', 'DG2']

ana_util.ALICEStyle()
c = ROOT.TCanvas('cQA','ALICE Performance Figures',1280, 800)
c.SetMargin(0.15, 0.02, 0.15, 0.02)
c.Draw()

PAD_EDGE_LEFT = c.GetLeftMargin()
PAD_EDGE_RIGHT = 1 - c.GetRightMargin()
PAD_EDGE_BOTTOM   = c.GetBottomMargin()
PAD_EDGE_TOP   = 1 - c.GetTopMargin()

fout = ROOT.TFile(args.output,'RECREATE')
PrintCover(c, args.print)

# Label - ALICE figure
pTxtALICE = ROOT.TPaveText(PAD_EDGE_LEFT + 0.02, PAD_EDGE_TOP - 0.18, PAD_EDGE_LEFT + 0.35, PAD_EDGE_TOP - 0.01,"brNDC")
pTxtALICE.SetFillColor(0)
pTxtALICE.SetTextSize(0.05)
pTxtALICE.SetTextFont(42) # Helvetica
pTxtALICE.SetTextAlign(13) # Top Left
if(args.eff):
  txt = pTxtALICE.AddText("ALICE Simulation")
else:
  txt = pTxtALICE.AddText("ALICE Performance")
txt.SetTextFont(62) # Helvetica Bold
def DrawALICE(pTxt, x1, y1, x2, y2):
  pTxt.SetX1NDC(x1)
  pTxt.SetY1NDC(y1)
  pTxt.SetX2NDC(x2)
  pTxt.SetY2NDC(y2)
  pTxt.Draw("same")

def InitALICELabel(text, x1, y1, x2, y2, size=0.04):
  PAD_EDGE_LEFT = ROOT.gPad.GetLeftMargin()
  PAD_EDGE_RIGHT = 1 - ROOT.gPad.GetRightMargin()
  PAD_EDGE_BOTTOM   = ROOT.gPad.GetBottomMargin()
  PAD_EDGE_TOP   = 1 - ROOT.gPad.GetTopMargin()
  pTxtALICE = ROOT.TPaveText(PAD_EDGE_LEFT + x1, PAD_EDGE_TOP + y1, PAD_EDGE_LEFT + x2, PAD_EDGE_TOP + y2,"brNDC")
  pTxtALICE.SetFillColor(0)
  pTxtALICE.SetTextSize(size)
  pTxtALICE.SetTextFont(42) # Helvetica
  pTxtALICE.SetTextAlign(13) # Top Left
  txt = pTxtALICE.AddText(text)
  txt.SetTextFont(62) # Helvetica Bold
  return pTxtALICE

def PrintFigure(name):
  ROOT.gPad.SaveAs(name + ".pdf")
  ROOT.gPad.SaveAs(name + ".eps")
  ROOT.gPad.SaveAs(name + ".png")
  ROOT.gPad.SaveAs(name + ".root")

def DrawQA_Electron(output):
  CaloQA = {}
  c.Clear()
  c.SetWindowSize(1600, 1200)
  c.SetMargin(0.15, 0.02, 0.12, 0.02)
  lgdE = ROOT.TLegend(PAD_EDGE_RIGHT - 0.42, PAD_EDGE_TOP - 0.25, PAD_EDGE_TOP - 0.05, PAD_EDGE_TOP - 0.015, "", "brNDC")
  lgdE.SetBorderSize(0)
  lgdE.SetTextSize(0.04)
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
  ana_util.SetColorAndStyle(CaloQA['MB']['E'], ROOT.kBlack, ROOT.kFullTriangleUp, 2.0)
  CaloQA['MB']['E'].SetTitle('')
  CaloQA['MB']['E'].SetXTitle('#it{E} (GeV)')
  CaloQA['MB']['E'].SetYTitle('#frac{1}{#it{N}_{evts}} #frac{d#it{N}_{cluster}}{d#it{E}}')
  CaloQA['MB']['E'].GetXaxis().SetRangeUser(0., 30.)
  CaloQA['MB']['E'].GetXaxis().SetTitleSize(0.05)
  CaloQA['MB']['E'].GetYaxis().SetRangeUser(1e-9, 20)
  CaloQA['MB']['E'].GetYaxis().SetTitleOffset(1.2)
  CaloQA['MB']['E'].GetYaxis().SetTitleSize(0.05)
  ROOT.gPad.SetLogy()
  CaloQA['MB']['E'].Draw('PE')
  lgdE.AddEntry(CaloQA['MB']['E'], 'Minimum bias trigger')
    # Gamma lower
  CaloQA['EG2']['E'].Add(CaloQA['DG2']['E'])
  ana_util.SetColorAndStyle(CaloQA['EG2']['E'], ROOT.kRed, ROOT.kFullCircle, 2.0)
  CaloQA['EG2']['E'].Draw('same PE')
  lgdE.AddEntry(CaloQA['EG2']['E'], 'Trigger #it{E}_{EMC}^{Cluster} > 5 GeV')
    # Gamma higher
  CaloQA['EG1']['E'].Add(CaloQA['DG1']['E'])
  ana_util.SetColorAndStyle(CaloQA['EG1']['E'], ROOT.kBlue, ROOT.kFullSquare, 2.0)
  CaloQA['EG1']['E'].Draw('same PE')
  lgdE.AddEntry(CaloQA['EG1']['E'], 'Trigger #it{E}_{EMC}^{Cluster} > 10 GeV')
  lgdE.Draw('same')
    # Label
  pTxtALICE = InitALICELabel("ALICE Performance", 0.02, -0.17, 0.35, -0.01)
  txt = pTxtALICE.AddText("pp, #sqrt{#it{s}} = 13 TeV")
  txt = pTxtALICE.AddText("|#it{#eta}| < 0.7")
  pTxtALICE.Draw("same")
  # Output
  c.Print(args.print, 'Title:Dielectron_E')
  PrintFigure("PERF_JpsiJet_ElectronEMCalE_pp13TeV")
  fout.cd()
  c.Write('cDieleE')

def DrawQA_InvMass(tagInfo, JPSI_PT_CUT_LOW = 10., JPSI_PT_CUT_UP = 35., JET_PT_CUT_LOW = 10., JET_PT_CUT_UP = 35.):
  print('>>> Processing performance figure : Invariant Mass Spectrum')
  tagInfo.GetAxis(0).SetRangeUser(JPSI_PT_CUT_LOW, JPSI_PT_CUT_UP)
  tagInfo.GetAxis(5).SetRangeUser(JET_PT_CUT_LOW, JET_PT_CUT_UP)
  hM = tagInfo.Projection(1)
  hM.SetName('hM')
  hM.SetTitle('')
  c.Clear()
  c.SetWindowSize(1600, 1200)
  hM.GetXaxis().SetTitleSize(0.05)
  hM.GetYaxis().SetTitleSize(0.05)
  ana_phys.STYLE_PERFORMANCE = True
  jpsi = ana_phys.ProcessInvMass(hM, None, 1.5, 4.5)
  jpsi.hM.Draw('same PE')
  # Cuts
  PAVE_CUTS = ROOT.TPaveText(0.70, 0.50, 0.90, 0.76, "brNDC")
  PAVE_CUTS.SetName("pTxtCuts")
  PAVE_CUTS.SetFillColor(0)
  PAVE_CUTS.SetTextFont(42)
  PAVE_CUTS.SetTextSize(0.035)
  PAVE_CUTS.AddText('|#it{y}_{e^{+}e^{-}}| < 0.9')
  PAVE_CUTS.AddText('   %.1f < #it{p}_{T,e^{+}e^{-}} < %.1f GeV/#it{c}' % (JPSI_PT_CUT_LOW, JPSI_PT_CUT_UP) )
  PAVE_CUTS.AddText('|#it{#eta}_{jet}| < 0.5')
  PAVE_CUTS.AddText('    %.1f < #it{p}_{T,jet} < %.1f GeV/#it{c}' % (JET_PT_CUT_LOW, JET_PT_CUT_UP) )
  PAVE_CUTS.Draw('same')
  # Label
  pTxtALICE.Draw("same")
  # Output
  c.Print(args.print, 'Title:DieleJets_InvMas')
  if(args.invmassL):
    PrintFigure("PERF_JpsiJet_DielectronJets_InvMassL_pp13TeV")
  if(args.invmassH):
    PrintFigure("PERF_JpsiJet_DielectronJets_InvMassH_pp13TeV")
  fout.cd()
  c.Write('cM')

# Processing MC output from JpsiJetAna.py
def DrawQA_Eff(mc):
  c.Clear()
  c.SetWindowSize(1440,1200)
  c.SetLogy()
  hP = mc.hJpsiEffPrompt
  hP.SetMarkerSize(2)
  hP.SetXTitle("#it{p}_{T,J/#psi} (GeV/#it{c})")
  hP.SetYTitle('#it{A #times #varepsilon}')
  hP.GetYaxis().SetRangeUser(5e-4, 8e-1)
  hP.GetYaxis().CenterTitle(True)
  hP.GetYaxis().SetTitleFont(62)
  hP.GetYaxis().SetTitleSize(0.07)
  hP.GetYaxis().SetTitleOffset(0.9)
  hP.GetXaxis().SetTitleSize(0.05)
  hP.GetXaxis().SetTitleOffset(1.2)
  mc.hJpsiEffPrompt.Draw("PE")
  hB = mc.hJpsiEffBdecay
  hB.SetMarkerSize(2)
  mc.hJpsiEffBdecay.Draw("same PE")
  # Description
  pTxt = ROOT.TPaveText(0.6, 0.35, 0.9, 0.53,"tlNDC")
  pTxt.SetFillColor(0)
  pTxt.SetTextSize(0.04)
  pTxt.SetTextAlign(32)
  pTxt.AddText("J/#psi #rightarrow e^{+}e^{-}, |#it{y}_{J/#psi}| < 0.9")
  pTxt.AddText("reco. with EMCal/DCal")
  pTxt.AddText("#it{E} > 5 GeV")
  pTxt.Draw("same")
  # Legend
  lgd = ROOT.TLegend(0.65, 0.2, 0.88, 0.35)
  lgd.SetTextSize(0.04)
  lgd.AddEntry(hP, 'Prompt')
  lgd.AddEntry(hB, 'Non-prompt')
  lgd.Draw("same")
  # Label
  pTxtALICE.Draw("same")
  # Output
  c.Print(args.print, 'Title:MC_JpsiEff')
  PrintFigure("JpsiJet_SIMUL_JpsiEff_EMCal_pp13TeV")
  fout.cd()
  c.Write('cEff')

anaResult = ROOT.TFile(args.file)

if(args.calo):
  DrawQA_Electron(anaResult.JpsiJetAnalysis)

if(args.invmassL):
  pTxtALICE = InitALICELabel("ALICE Performance", 0.02, -0.20, 0.35, -0.02)
  txt = pTxtALICE.AddText("pp, #it{#sqrt{s}} = 13 TeV, #it{N}_{ev} = 126 M")
  txt.SetTextSize(0.035)
  txt = pTxtALICE.AddText("EMCal trigger, #it{E} > 5 GeV")
  txt.SetTextSize(0.035)
  DrawQA_InvMass(anaResult.TagInfoL, 5.0, 35., 5., 35.)


if(args.invmassH):
  pTxtALICE = InitALICELabel("ALICE Performance", 0.02, -0.20, 0.35, -0.02)
  txt = pTxtALICE.AddText("pp, #sqrt{#it{s}} = 13 TeV, #it{N}_{ev} = 96 M")
  txt.SetTextSize(0.035)
  txt = pTxtALICE.AddText("EMCal trigger, #it{E} > 10 GeV")
  txt.SetTextSize(0.035)
  DrawQA_InvMass(anaResult.TagInfoH, 10.0, 35., 10., 35.)

if(args.eff):
  txt = pTxtALICE.AddText("pp, #sqrt{#it{s}} = 13 TeV")
  txt = pTxtALICE.AddText("Pythia6, Perugia2011")
  DrawQA_Eff(anaResult)

# Input : JpsiJetFilterAna.root (TH2D of dEta-dPhi)
if(args.map):
  hMap = anaResult.hJpsiJet
  c.Clear()
  c.SetWindowSize(1440,1200)
  c.SetMargin(0,0,0,0)
  padBody = ROOT.TPad("padBody","padBody", 0., 0., 1.0, 1.0)
  padBody.SetMargin(0.12,0.12,0.05,0.)
  padBody.SetLogz()
  padBody.Draw()
  padBody.cd()
  # TH2
    # x
  xAxis = hMap.GetXaxis()
  xAxis.SetTitle("#it{#Delta#varphi}")
  xAxis.CenterTitle(True)
  xAxis.SetTitleSize(0.05)
  xAxis.SetTitleOffset(1.2)
    # y
  yAxis = hMap.GetYaxis()
  yAxis.SetTitle("#it{#Delta#eta}")
  yAxis.CenterTitle(True)
  yAxis.SetTitleSize(0.05)
  yAxis.SetTitleOffset(1.5)
    # z
  zAxis = hMap.GetZaxis()
  zAxis.SetTitle("Counts")
  zAxis.CenterTitle(True)
  zAxis.SetTitleSize(0.05)
  zAxis.SetTitleOffset(1.2)
  hMap.SetLineWidth(0)
  hMap.SetContour(30)
  hMap.Draw("LEGO2FBBB")
  # Palette
  pAxis = ROOT.TPaletteAxis(0.81, -0.9, 0.9, 0.8, hMap)
  pAxis.Draw("same")
  # Label
  pTxt = InitALICELabel("ALICE Performance", 0.02, -0.20, 0.35, -0.02)
  txt = pTxt.AddText("pp, #sqrt{#it{s}} = 13 TeV")
  txt = pTxt.AddText("EMCal trigger, #it{E} > 5 GeV")
  pTxt.Draw("same")
  # Cuts
  PAVE_CUTS = ROOT.TPaveText(0.55, 0.7, 0.88, 0.96, "brNDC")
  PAVE_CUTS.SetName("pTxtCuts")
  PAVE_CUTS.SetFillColor(0)
  PAVE_CUTS.SetTextFont(42)
  PAVE_CUTS.SetTextSize(0.035)
  PAVE_CUTS.SetTextAlign(22)
  txt = PAVE_CUTS.AddText('#it{M}_{e^{+}e^{-}} #in [2.92, 3.16] (GeV/#it{c}^{2})')
  txt.SetTextFont(62)
  PAVE_CUTS.AddText('|#it{y}_{e^{+}e^{-}}| < 0.9')
  PAVE_CUTS.AddText('#it{p}_{T,e^{+}e^{-}} > 5 GeV/#it{c}')
  PAVE_CUTS.AddText('|#it{#eta}_{jet}| < 0.5')
  PAVE_CUTS.AddText('#it{p}_{T,jet} > 1 GeV/#it{c}')
  PAVE_CUTS.Draw('same')
  # Output
  c.Print(args.print, 'Title:QA_JpsiJetMap')
  PrintFigure("JpsiJet_PERF_JpsiJetCorrelation_Raw_pp13TeV")
  fout.cd()
  c.Write('cMap')

PrintCover(c, args.print, isBack=True)

anaResult.Close()
fout.Close()
