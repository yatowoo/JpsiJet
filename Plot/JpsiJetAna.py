#!/usr/bin/env python3

######
##
##    Post-processing script for JpsiJet analysis in ALICE
##
##  Software: python3 on ROOT6 (version >= 6.16)
##  Input: AliAnalysisTaskJpsiJet on data and MC
##  Outputs: 
##    - QA: Event, track, cluster, dielectron, jet
##    - Raw: J/psi signal extraction, J/psi or jet pT spectrum, z
##    - Correction: Electron and J/psi efficiency, detector respones
##  Arguments:
##    - Result file name
##    - Job tag: datasets, cuts, usage, ...
##    - MC flag?
##    - Output directory
######

import sys, os, json, argparse
from array import array

# Command-line Arguments
parser = argparse.ArgumentParser(description='Post-processing script for AliAnalysisTaskJpsiJet')
parser.add_argument('-f', '--file',help='Analysis results of AliAnalysisTaskJpsiJet', default="AnalysisResults.root")
parser.add_argument('-o', '--output',help='Output file path', default='JpsiJetAna.root')
parser.add_argument('-p', '--print',help='Print in PDF file', default='JpsiJetAna.pdf')
parser.add_argument('--mc',help='MC flag for DrawMC methods', default=False, action='store_true')
parser.add_argument('--calo',help='QA for EMCal cluster energy', default=False, action='store_true')
parser.add_argument('--jet',help='QA for jet', default=False, action='store_true')
parser.add_argument('--all',help='Run all QA processing', default=False, action='store_true')
args = parser.parse_args()

# ROOT libs
import ROOT
from ROOT import TFile, TCanvas, TLegend, gPad, TPaveText
from ROOT import TH1, TH2, TF1
import ana_util
from ana_util import *

# Input - analysis results from ALICE tasks
fin = TFile(args.file)
outputs = fin.Get("JpsiJetAnalysis")
if(outputs == None):
  print("[X] ERROR - JpsiJetAnalysis not found.")
  exit(1)
# Output - ROOT file and PDF
fout = TFile(args.output, "RECREATE")
if(args.mc):
  padQA = TCanvas("cMC", "Jpsi in jets analysis on MC",800,600)
else:
  padQA = TCanvas("cAna", "Jpsi in jets analysis",800,600)
# PDF Cover - Open PDF file and draw cover
PrintCover(padQA, args.print)
# End - Cover

# Global Settings and Variables
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetPalette()

COLOR = SelectColor()
MARKER = SelectMarker()

JPSI_MASS_LOWER = 2.92
JPSI_MASS_UPPER = 3.16
JPSI_LXY_PROMPT = 0.01
JPSI_LXY_BDECAY = 0.01
JPSI_PT_LOWER = 10.0
JPSI_PT_UPPER = 50.0
JPSI_Y_LOWER = -0.9
JPSI_Y_UPPER = 0.9
JET_PT_LOWER = 15.0
JET_PT_UPPER = 50.0

# J/psi pT bins : 0 - 50, binw = 0.2, 0.5, 1, 2, 5
BINNING_JPSI_PT = [0.2*x for x in range(0,25,1)]
BINNING_JPSI_PT += [ 0.1*x for x in range(50,100,5)]
BINNING_JPSI_PT += list(range(10, 15, 1))
BINNING_JPSI_PT += list(range(15, 25, 2))
BINNING_JPSI_PT += list(range(25, 55, 5))
BINNING_JPSI_PT = array('d', BINNING_JPSI_PT) # Convert to double*

# Jet pT bins - Edge=0, 0.3, 1, 3, 10, 20, 50, 100
BINNING_JET_PT = [0.05*x for x in range(0,6,1)]
BINNING_JET_PT += [ 0.1*x for x in range(3,10,1)]
BINNING_JET_PT += [ 0.2*x for x in range(5,15,1)]
BINNING_JET_PT += [ 0.5*x for x in range(6,20,1)]
BINNING_JET_PT += list(range(10,20,2))
BINNING_JET_PT += list(range(20,50,5))
BINNING_JET_PT += list(range(50,110,10))
BINNING_JET_PT = array('d', BINNING_JET_PT)

TRIGGER_CLASSES = ['MB', 'EG1', 'EG2', 'DG1', 'DG2']
# QA index and results
EvStats = {}
RunStats = {}
QAhists = {}

def DrawQA_PairInJet(qa, tag):
  print("[-] INFO - Processing QA plots for " + tag)
  qa.SetOwner(True)
  # Canvas
  c = ROOT.TCanvas("cDet","Plots on Detector Level", 800, 600)
  c.cd()
  # Pair tagged jets
    # N constituents
  pairJet = qa.FindObject("PairInJet")
  pairJet.SetOwner(True)
  Ntrk = pairJet.FindObject("Ntracks_pT") # TH2D
  Ntrk.GetYaxis().SetRangeUser(0, 25)
  Ntrk.Draw("COLZ")
    ## Profile
  pfx = Ntrk.ProfileX()
  pfx.SetLineColor(ROOT.kRed)
  pfx.SetMarkerStyle(20)
  pfx.Draw("same")
    ## Legend
  lgd = ROOT.TLegend(0.2, 0.65, 0.4, 0.8)
  lgd.AddEntry(pfx,"<N_{tracks}>")
  lgd.SetBorderSize(0)
  lgd.Draw("same")
    ## Save
  c.SaveAs("Jet-Ntrk.pdf")
  c.SaveAs("Jet-Ntrk.root")
  # Free
  qa.Delete("C")
  return
# END - Drawing QA plots

# ONLY for old 16k outputs
def DrawQA_Jet(outputs):
  # Pad initialization
  padMain, padRatio = ana_util.NewRatioPads(padQA, 'padJetPt', 'padJetPtRatio')
  # Legend
  lgd = ROOT.TLegend(0.6, 0.5, 0.85, 0.88)
  lgd.SetBorderSize(0)
  lgd.SetFillColor(0)
  lgd.SetNColumns(2)
  # Loop on triggers
  QAhists['jet'] = {}
  for i,trig in enumerate(TRIGGER_CLASSES):
    if(trig == 'MB'):
      continue
    qa = outputs.Get('QAhistos_' + trig)
    qa.SetOwner(True)
    jetQA = qa.FindObject('Jet')
    jetQA.SetOwner(True)
    # jet pT spectra
    padMain.cd()
    color = next(COLOR)
      # Original
    jetOrig = jetQA.FindObject("Jet_AKTChargedR040_tracks_pT0150_pt_scheme")
    jetOrig.SetOwner(True)
    ptOrig = jetOrig.FindObject('jetPtEtaPhi').Projection(0).Rebin(len(BINNING_JET_PT)-1, "hJetOrig" + trig, BINNING_JET_PT)
    ptOrig.Scale(1./EvStats[trig]['Dielectron'],'width')
    ana_util.SetColorAndStyle(ptOrig, color, DATA_MARKER[i])
    ptOrig.SetTitle('')
    ptOrig.SetXTitle('p_{T,jet} (GeV/c)')
    ptOrig.GetYaxis().SetRangeUser(5e-5,5)
    ptOrig.SetYTitle('1/N_{ev} dN_{jet}/dp_{T,jet}')
    ptOrig.Draw('SAME PE')
    lgd.AddEntry(ptOrig,trig + ' - Original')
      # Updated
    jetUpdated = jetQA.FindObject("JpsiJet_AKTChargedR040_tracksWithPair_pT0150_pt_scheme")
    jetUpdated.SetOwner(True)
    ptUpdated = jetUpdated.FindObject('jetPtEtaPhi').Projection(0).Rebin(len(BINNING_JET_PT)-1, "hJetNew_" + trig, BINNING_JET_PT)
    ptUpdated.Scale(1./EvStats[trig]['Dielectron'], 'width')
    ana_util.SetColorAndStyle(ptUpdated, color, MC_MARKER[i])
    ptUpdated.Draw('SAME PE')
    lgd.AddEntry(ptUpdated,trig + ' - Updated')
    # Ratio
    padRatio.cd()
    QAhists['jet']['hRatio_' + trig] = ptUpdated.Clone('hRatio_' + trig)
    rp = QAhists['jet']['hRatio_' + trig]
    rp.Divide(ptOrig)
    rp.GetYaxis().SetTitle('Updated/Original')
    ana_util.SetRatioPlot(rp, 0.2, 2.0)
    rp.SetXTitle('p_{T,jet} (GeV/c)')
    rp.Draw('same PE')
    qa.Delete()
  padMain.cd()
  lgd.Draw('same')
  padQA.Modified()
  padQA.Print(args.print,'Title:JetQA')
  padQA.Write('cJetPt')

def DrawMC_JetPt(qa):
  # Event norm.
  ev = qa.FindObject("EventStats")
  nEv = ev.GetBinContent(6)
  nEvPair = ev.GetBinContent(7) + ev.GetBinContent(8)
  nEvTagged = ev.GetBinContent(9)
  # Inclusive jet
  hsIncl = qa.FindObject("Jet_AKTChargedR040_tracks_pT0150_pt_scheme").FindObject("jetVars")
  incl = hsIncl.Projection(0)
  incl.SetName("hJetInclusive")
  incl.SetTitle("Jet pT spectra")
  incl.GetYaxis().SetTitle("1/N_{ev} dN/dp_{T}")
  incl.Rebin(10) # 0.05 -> 0.5 GeV/c
  # Jet with pair, Re-clustering with pair in jet
  #hsPair = qa.FindObject("JpsiJet_AKTChargedR040_tracksWithPair_pT0150_pt_scheme")
  # J/psi tagged jet
  hsTagged = qa.FindObject("PairInJet").FindObject("PairVars")
  hsTagged.GetAxis(0).SetRangeUser(10,50)
  hsTagged.GetAxis(1).SetRangeUser(JPSI_MASS_LOWER,JPSI_MASS_UPPER)
  hsTagged.GetAxis(2).SetRangeUser(-JPSI_LXY_PROMPT,JPSI_LXY_PROMPT)
  tagPrompt = hsTagged.Projection(5)
  tagPrompt.SetName("hTaggedJetPrompt")
  hsTagged.GetAxis(2).SetRangeUser(JPSI_LXY_BDECAY,1.0)
  tagBdecay = hsTagged.Projection(5)
  tagBdecay.SetName("hTaggedJetBdecay")
  # Sideband
  # Drawing
  c = ROOT.TCanvas("cQA","Jet spectra", 800, 600)
  c.Draw()
  c.SetLogy(True)
  HistNorm(incl, nEv)
  incl.SetLineColor(ROOT.kBlack)
  incl.SetMarkerColor(ROOT.kBlack)
  incl.SetMarkerStyle(20)
  incl.Draw("EP")
  HistNorm(tagPrompt, nEv)
  tagPrompt.SetLineColor(ROOT.kRed)
  tagPrompt.SetMarkerColor(ROOT.kRed)
  tagPrompt.SetMarkerStyle(21)
  tagPrompt.Draw("same EP")
  HistNorm(tagBdecay, nEv)
  tagBdecay.SetLineColor(ROOT.kBlue)
  tagBdecay.SetMarkerColor(ROOT.kBlue)
  tagBdecay.SetMarkerStyle(22)
  tagBdecay.Draw("same EP")
  ## Legend
  lgd = ROOT.TLegend(0.6, 0.64, 0.85, 0.79)
  lgd.AddEntry(incl, "Inclusive jets")
  lgd.AddEntry(tagPrompt, "Tagged jets (Prompt)")
  lgd.AddEntry(tagBdecay, "Tagged jets (Non-prompt)")
  lgd.Draw("same")
  c.SaveAs("MC-JetPt.pdf")
  c.SaveAs("MC-JetPt.root")

# Return legend entry with range and result
def FitRF(caloRF, tag = 'RF', Emin = 11.0, Emax = 40.0):
  fcn = TF1('fNorm' + tag, '[0]', 0., 100)
  fcn.SetLineColor(caloRF.GetLineColor())
  fcn.SetLineStyle(6)
  fcn.SetLineWidth(2)
  caloRF.Fit(fcn, "ISQ", "", Emin, Emax)
  fcn.Draw("same")
  factor = fcn.GetParameter(0)
  error = fcn.GetParError(0)
  return '%.1f < E < %.1f GeV, R_{trig}= %.1f #pm %.1f' % (Emin, Emax, factor, error)
# Input: QAhistos_ALL with Cluster_INT7|EG1|EG2|DG1|DG2
# Step: Normalization, Re-bining?, Ratio, Fitting
# Plot: Custer energy & RF, legend, text 
def DrawQA_Calo(qa):
  CaloQA = {}
  for trig in TRIGGER_CLASSES:
    CaloQA[trig] = {}
    CaloQA[trig]['NEvent'] = 0
    CaloQA[trig]['E'] = None
    CaloQA[trig]['RF'] = None
    CaloQA[trig]['Eth'] = 11.0 if trig.count('1') else 6.0
  # Event numbers
  evQA = qa.FindObject('Event_afterCuts') # TList
  evQA.SetOwner(True)
  hTrig = evQA.FindObject('FiredTag') # TH1D
  for i in range(1, hTrig.GetNbinsX() + 1):
    tag = hTrig.GetXaxis().GetBinLabel(i)
    if(tag == ''):
      break
    tag = tag.replace('INT7', 'MB')
    for trig in tag.split('_'):
      CaloQA[trig]['NEvent'] += hTrig.GetBinContent(i)
  # End - read with fired tags
  padQA.Clear()
  padQA.SetWindowSize(1600, 600)
  padQA.Divide(2)
  lgdE = TLegend(0.5, 0.7, 0.8, 0.85, "", "brNDC")
  lgdE.SetBorderSize(0)
  lgdE.SetNColumns(2)
  lgdRF = TLegend(0.5, 0.12, 0.88, 0.45, "", "brNDC")
  lgdRF.SetTextSize(0.02)
  lgdRF.SetBorderSize(0)
  lgdRF.SetNColumns(1)
  COLOR = SelectColor() # Reset
  for trig in TRIGGER_CLASSES:
    # Cluster energy
    calo = qa.FindObject('Cluster_' + trig) # TList
    calo.SetOwner(True)
    caloE = calo.FindObject('E').Clone('hClusterE_' + trig)
    HistNorm(caloE, CaloQA[trig]['NEvent'])
    caloE.SetTitle('EMCal/DCal cluster energy distribution')
    caloE.GetXaxis().SetRangeUser(0, 40)
    caloE.GetYaxis().SetRangeUser(1e-8, 100.)
    caloE.GetYaxis().SetTitle('#frac{1}{N_{ev}} #frac{dN_{cls}}{dE}')
    padQA.cd(1)
    colorHist = next(COLOR)
    caloE.SetLineColor(colorHist)
    caloE.SetMarkerColor(colorHist)
    caloE.SetMarkerStyle(kRound)
    caloE.Draw("same PE")
    lgdE.AddEntry(caloE, trig)
    gPad.SetLogy()
    # Rejection factor
    if(CaloQA['MB']['E']):
      caloRF = caloE.Clone('hClusterRF_' + trig)
      caloRF.SetTitle('Rejection factor of EMCal by ratio of cluster energy')
      caloRF.GetXaxis().SetRangeUser(0, 40)
      caloRF.GetYaxis().SetRangeUser(1e-2, 1e4)
      caloRF.GetYaxis().SetTitle('R_{trig}')
      caloRF.Divide(CaloQA['MB']['E'])
      padQA.cd(2)
      caloRF.Draw("same PE")
      gPad.SetLogy()
      caloRF.Write()
      CaloQA[trig]['RF'] = caloRF
      lgdRF.AddEntry(caloRF, trig + '/MB: ' + FitRF(caloRF, trig, CaloQA[trig]['Eth']))
    CaloQA[trig]['E'] = caloE
    calo.Delete()
  # End - trigger loop
  padQA.cd(1)
  lgdE.Draw("same")
  # RF - EG1/EG2
  caloRF_NEW = CaloQA['EG1']['E'].Clone('hClusterRF_EG12')
  caloRF_NEW.SetTitle('Rejection factor EMCal/DCal by ratio of cluster energy')
  caloRF_NEW.Divide(CaloQA['EG2']['E'])
  caloRF_NEW.SetMarkerStyle(kRoundHollow)
  padQA.cd(2)
  caloRF_NEW.Draw("same PE")
  caloRF_NEW.Write()
  lgdRF.AddEntry(caloRF_NEW, 'EG1/EG2: ' + FitRF(caloRF_NEW, 'EG12'))
  # RF -DG1/DG2
  caloRF = CaloQA['DG1']['E'].Clone('hClusterRF_DG12')
  caloRF.SetTitle('Rejection factor EMCal/DCal by ratio of cluster energy')
  caloRF.Divide(CaloQA['DG2']['E'])
  caloRF.SetMarkerStyle(kCrossHollow)
  caloRF.Draw("same PE")
  caloRF.Write()
  lgdRF.AddEntry(caloRF, 'DG1/DG2: ' + FitRF(caloRF, 'DG12'))
  lgdRF.Draw("same")
  # Output
  padQA.Print(args.print,'Title:ClusterQA')
  padQA.Write("cQA_Cluster")
# End - Calo cluster QA

def DrawQA_Runwise(outputs):
  # Processing - TH2 with x-RunNo, y-Label
  # N event only
  RunStats['NEvent'] = {}
  padQA.Clear()
  padQA.SetWindowSize(1000,600)
  padQA.SetLogy()
  # Legend
  lgd = ROOT.TLegend(0.15, 0.15, 0.55, 0.25)
  lgd.SetNColumns(len(TRIGGER_CLASSES))
  lgd.SetBorderSize(0)
  lgd.SetFillColor(0)
  for trig in TRIGGER_CLASSES:
    # Trigger
    qa = outputs.Get('QAhistos_' + trig)
    if(qa == None):
      print('[X] ERROR - %s not found.' % trig)
      continue
    qa.SetOwner(True)
    # Runwise
    runwise = qa.FindObject('Runwise')
    if(runwise == None):
      print('[X] ERROR - %s/Runwise QA not found' % qa.GetName())
      return
    runwise.SetOwner(True)
    hEv = runwise.FindObject('NEvent')
    hEv.LabelsDeflate()
    # N event
    RunStats['NEvent'][trig] = ROOT.TH1D('hNEv' + trig, 'Runwise QA - Number of selected events', 201, -0.5, 200)
    hnew = RunStats['NEvent'][trig]
    for iBin in range(1, hnew.GetNbinsX()+1):
      label = hEv.GetXaxis().GetBinLabel(iBin)
      nev = hEv.GetBinContent(iBin, 4)
      hnew.Fill(label, nev)
      binID = hnew.GetXaxis().FindBin(label)
      hnew.SetBinError(binID, math.sqrt(nev))
    hnew.SetMarkerStyle(next(MARKER))
    hnew.GetYaxis().SetRangeUser(1., 1e8)
    hnew.LabelsOption("av")
    hnew.LabelsDeflate()
    hnew.Draw('same PE')
    lgd.AddEntry(hnew, trig)
    qa.Delete()
  lgd.Draw('same')
  padQA.Print(args.print, 'Title:Runwise_NEv')
  # End

def DrawQA_Event(outputs):
  # Print event statistics
  evTable = ROOT.TPaveText(0.2, 0.2, 0.8, 0.8)
  txt = evTable.AddText("Event statistics")
  txt.SetTextFont(62) # Bold
  txt = evTable.AddText("EvTag | Trigger | Selected | Dielectron | N_{pair} = 1 | N_{pair} > 1 | Tagged jet |")
  for trig in ['ALL'] + TRIGGER_CLASSES:
    # Init
    EvStats[trig] = {}
    qa = outputs.Get('QAhistos_' + trig)
    if(qa == None):
      print('[X] ERROR - %s not found.' % trig)
      continue
    qa.SetOwner(True)
    # Processing
    hEv = qa.FindObject('EventStats')
    txt = trig + ' | '
    EvStats[trig]['Trigger'] = hEv.GetBinContent(2)
    txt += '%.2e | ' % EvStats[trig]['Trigger']
    EvStats[trig]['Ana'] = hEv.GetBinContent(6)
    txt += '%.2e | ' % EvStats[trig]['Ana']
    EvStats[trig]['Dielectron'] = hEv.GetBinContent(7) + hEv.GetBinContent(8)
    txt += '%.2e | ' % EvStats[trig]['Dielectron']
    EvStats[trig]['Single'] = hEv.GetBinContent(7)
    txt += '%.2e | ' % EvStats[trig]['Single']
    EvStats[trig]['MultiPair'] = hEv.GetBinContent(8)
    txt += '%.2e | ' % EvStats[trig]['MultiPair']
    EvStats[trig]['TaggedJet'] = hEv.GetBinContent(9)
    txt += '%.2e | ' % EvStats[trig]['TaggedJet']
    txt = evTable.AddText(txt)
    # End
    qa.Delete()
  padQA.Clear()
  padQA.SetWindowSize(800, 600)
  padQA.cd()
  evTable.Draw()
  padQA.Print(args.print, 'Title:EventStats')
  evTable.Delete()
  return EvStats

if(args.all):
  DrawQA_Event(outputs)
  DrawQA_Runwise(outputs)
elif(args.calo):
  qaName = "QAhistos_ALL"
  qa = outputs.Get(qaName)
  qa.SetOwner(True)
  DrawQA_Calo(qa)
  qa.Delete()

if(args.jet):
  DrawQA_Jet(outputs)

# Detectro response matrix - 4 dim.
## THnSparse: z_det, z_gen, jetPt_det, jetPt_gen
def GetDetectorResponse_FF(drm, tag):
  drm.GetAxis(0).SetRangeUser(0., 1.)
  drm.GetAxis(1).SetRangeUser(0., 1.)
  drm.GetAxis(2).SetRangeUser(JET_PT_LOWER, JET_PT_UPPER)
  drm.GetAxis(3).SetRangeUser(JET_PT_LOWER, JET_PT_UPPER)
  zMatrix = drm.Projection(1,0)
  zMatrix.SetName("hResponseFF_" + tag)
  zMatrix.SetTitle("Detector response matrix - J/#psi in Jet (z) ("+tag+")")
  return zMatrix
def GetDetectorResponse_JetPt(drm, tag):
  drm.GetAxis(2).SetRangeUser(JET_PT_LOWER, JET_PT_UPPER)
  drm.GetAxis(3).SetRangeUser(JET_PT_LOWER, JET_PT_UPPER)
  ptMatrix = drm.Projection(3,2)
  ptMatrix.SetName("hResponseJetPt_" + tag)
  ptMatrix.SetTitle("Detector response matrix - J/#psi tagged jet p_{T} ("+tag+")")
  return ptMatrix
# MChistos
def DrawMC_DetectorResponse(mc):
  # Canvas
  padQA.Clear()
  padQA.SetWindowSize(1600, 600)
  padQA.Divide(2)
  padQA.SetTicks()
  padQA.Draw()
  # Histos
  drmPrompt = mc.FindObject("JpsiPrompt").FindObject("Jet_DetResponse").Clone("hsDrmPrompt")
  drmBdecay = mc.FindObject("JpsiBdecay").FindObject("Jet_DetResponse").Clone("hsDrmBdecay")
  # jet pT
  ptPrompt = GetDetectorResponse_JetPt(drmPrompt, "Prompt")
  ptBdecay = GetDetectorResponse_JetPt(drmBdecay, "Bdecay")
  padQA.cd(1)
  ptPrompt.Draw("COLZ")
  gPad.SetLogz()
  padQA.cd(2)
  ptBdecay.Draw("COLZ")
  ROOT.gPad.SetLogz()
  padQA.Print(args.print, "Title:Response_JetPt")
  # z - Fragmentation Function
  zPrompt = GetDetectorResponse_FF(drmPrompt, "Prompt")
  zBdecay = GetDetectorResponse_FF(drmBdecay, "Bdecay")
  padQA.cd(1)
  zPrompt.Draw("COLZ")
  ROOT.gPad.SetLogz()
  padQA.cd(2)
  zBdecay.Draw("COLZ")
  ROOT.gPad.SetLogz()
  padQA.Print(args.print, "Title:Response_FF")
  padQA.Clear()

def DrawMC(mc):
  print("[-] INFO - Processing MC plots")
  mc.SetOwner(True)
  # J/psi generator info
  # Electron PID
  padQA.Clear()
  padQA.SetWindowSize(800, 600)
  padQA.SetGrid()
  padQA.SetTicks()
  padQA.cd()
  mcEle = mc.FindObject("Electron")
  mcEle.SetOwner(True)
    # TPC
  eleAllTPC = mcEle.FindObject("eleVars").Projection(0)
  eleAllTPC.SetName("hEleAllTPC")
  elePureTPC = mcEle.FindObject("PID_pure")
  eleWrongTPC = mcEle.FindObject("PID_wrong")
  eleTPC = elePureTPC.Clone("hEleDetTPC")
  eleTPC.Add(eleWrongTPC)
  eleEffTPC = elePureTPC.Clone("hEleEffTPC")
  eleEffTPC.Divide(eleAllTPC)
  eleEffTPC.GetXaxis().SetRangeUser(0., 50.)
  eleEffTPC.GetYaxis().SetTitle("A #times #varepsilon")
  eleEffTPC.SetTitle("Electron PID Acceptance and Efficiency")
  eleEffTPC.SetLineWidth(2)
  #eleEffTPC.Rebin(5) # Bin width : 0.2 -> 1.0
  eleEffTPC.Draw()
  eleEffTPC.Write()
    # EMCal
  eleAllEMC = mcEle.FindObject("EMCal_all")
  eleDetEMC = mcEle.FindObject("EMCal_det")
  eleEffEMC = eleDetEMC.Clone("hEleEffEMC")
  eleEffEMC.Divide(eleAllEMC)
  eleEffEMC.SetLineColor(ROOT.kRed)
  eleEffEMC.SetLineWidth(2)
  #eleEffEMC.Rebin(5) # Bin width : 0.2 -> 1.0
  eleEffEMC.GetXaxis().SetRangeUser(5.0, 45)
  eleEffEMC.Draw("same")
  eleEffEMC.Write()
    # Legend
  lgd = ROOT.TLegend(0.6, 0.7, 0.85, 0.85)
  lgd.AddEntry(eleEffTPC, "TPC")
  lgd.AddEntry(eleEffEMC, "TPC + EMCal/DCal")
  lgd.Draw("same")
  padQA.Print(args.print, "Title:Electron_Eff")
  # Electron Purity
  elePurityTPC = elePureTPC.Clone("hElePurityTPC")
  elePurityTPC.Divide(eleTPC)
  elePurityTPC.GetXaxis().SetRangeUser(0., 50.)
  elePurityTPC.GetYaxis().SetRangeUser(0.97, 1.01)
  elePurityTPC.GetYaxis().SetTitle("Purity (%)")
  elePurityTPC.GetYaxis().SetTitleOffset(1.5)
  elePurityTPC.SetTitle("Electron PID purity by TPC only")
  elePurityTPC.SetLineWidth(2)
  padQA.Clear()
  elePurityTPC.Draw()
  elePurityTPC.Write()
  padQA.Print(args.print, "Title:Electron_Purity")
  ###
  # J/psi reconstruction
    # Prompt
  jpsiPrompt = mc.FindObject("JpsiPrompt")
  jpsiPrompt.SetOwner(True)
    # THnSparse - pT, Y, phi, E
  jpsiPromptGen = jpsiPrompt.FindObject("jpsiVars")
  jpsiPromptGen.GetAxis(1).SetRangeUser(JPSI_Y_LOWER, JPSI_Y_UPPER)
  jpsiPromptAll = jpsiPromptGen.Projection(0).Rebin(len(BINNING_JPSI_PT)-1, "hJpsiPromptAll", BINNING_JPSI_PT)
    # THnSparse - pT, Mee, Lxy
  jpsiPromptReco = jpsiPrompt.FindObject("Reco_sig")
  jpsiPromptReco.GetAxis(1).SetRangeUser(JPSI_MASS_LOWER, JPSI_MASS_UPPER)
  jpsiPromptPt = jpsiPromptReco.Projection(0)
  jpsiPromptPt.SetName('hJpsiPromptReco')
  jpsiLxyPrompt = jpsiPromptReco.Projection(2)
  jpsiLxyPrompt.SetName('hJpsiLxyPrompt')
    # Prompt background
  jpsiPromptBkg = jpsiPrompt.FindObject("Reco_bkg")
  jpsiPromptBkg.GetAxis(1).SetRangeUser(JPSI_MASS_LOWER, JPSI_MASS_UPPER)
  jpsiLxyBkg = jpsiPromptBkg.Projection(2)
  jpsiLxyBkg.SetName('hJpsiLxyBkg')
    # Prompt Eff.
  jpsiPromptEff = jpsiPromptPt.Rebin(len(BINNING_JPSI_PT)-1, "hJpsiEffPrompt", BINNING_JPSI_PT)
  jpsiPromptEff.Sumw2()
  jpsiPromptAll.Sumw2()
  jpsiPromptEff.Divide(jpsiPromptAll)
  jpsiPromptEff.SetLineWidth(2)
  jpsiPromptEff.SetLineColor(kBlue)
  jpsiPromptEff.SetMarkerStyle(kRound)
  jpsiPromptEff.SetMarkerColor(kBlue)
  jpsiPromptEff.SetTitle("J/#psi Reconstruction - Acceptance and Efficiency")
  padQA.Clear()
  jpsiPromptEff.GetXaxis().SetRangeUser(0., 50.)
  jpsiPromptEff.GetYaxis().SetRangeUser(5e-4, 0.3)
  jpsiPromptEff.GetYaxis().SetTitle("A #times #varepsilon")
  jpsiPromptEff.Draw("PE")
  padQA.SetLogy(True)
  jpsiPromptEff.Write()
    # Non-prompt / B-decay
  jpsiBdecay = mc.FindObject("JpsiBdecay")
  jpsiBdecay.SetOwner(True)
    # THnSparse - pT, Y, phi, E
  jpsiBdecayGen = jpsiBdecay.FindObject("jpsiVars")
  jpsiBdecayGen.GetAxis(1).SetRangeUser(JPSI_Y_LOWER, JPSI_Y_UPPER)
  jpsiBdecayAll = jpsiBdecayGen.Projection(0).Rebin(len(BINNING_JPSI_PT)-1, "hJpsiBdecayAll", BINNING_JPSI_PT)
    # THnSparse - pT, Mee, Lxy
  jpsiBdecayReco = jpsiBdecay.FindObject("Reco_sig")
  jpsiBdecayReco.GetAxis(1).SetRangeUser(JPSI_MASS_LOWER, JPSI_MASS_UPPER)
  jpsiBdecayPt = jpsiBdecayReco.Projection(0)
  jpsiBdecayPt.SetName('hJpsiBdecayReco')
  jpsiLxyBdecay = jpsiBdecayReco.Projection(2)
  jpsiLxyBdecay.SetName('hJpsiLxyBdecay')
    # Non-prompt background
  jpsiBdecayBkg = jpsiBdecay.FindObject("Reco_bkg")
  jpsiBdecayBkg.GetAxis(1).SetRangeUser(JPSI_MASS_LOWER, JPSI_MASS_UPPER)
  jpsiLxyBkg.Add(jpsiBdecayBkg.Projection(2))
  jpsiBdecayEff = jpsiBdecayPt.Rebin(len(BINNING_JPSI_PT)-1, "hJpsiEffBdecay", BINNING_JPSI_PT)
  jpsiBdecayEff.Sumw2()
  jpsiBdecayAll.Sumw2()
  jpsiBdecayEff.Divide(jpsiBdecayAll)
  jpsiBdecayEff.SetLineColor(kRed)
  jpsiBdecayEff.SetLineWidth(2)
  jpsiBdecayEff.SetMarkerColor(kRed)
  jpsiBdecayEff.SetMarkerStyle(kRound)
  jpsiBdecayEff.Draw("same PE")
  jpsiBdecayEff.Write()
    # Legend
  if(lgd):
    lgd.Delete("C")
  lgd = ROOT.TLegend(0.6, 0.2, 0.85, 0.35)
  lgd.AddEntry(jpsiPromptEff, "Prompt")
  lgd.AddEntry(jpsiBdecayEff, "Non-prompt")
  lgd.Draw("same")
    # Text
  pTxt = ROOT.TPaveText(0.55, 0.4, 0.85, 0.55, "brNDC")
  pTxt.AddText("J/#psi #rightarrow e^{+}e^{-}, " + "|y_{J/#psi}| < %.1f" % JPSI_Y_UPPER)
  pTxt.AddText("reco. with EMCal/DCal")
  pTxt.Draw("same")
  padQA.Print(args.print, "Title:Jpsi_Eff")
  padQA.Write("cJpsiEff")
  ##
  ## Plotting : J/psi Lxy
  jpsiLxyAll = jpsiLxyPrompt.Clone("hJpsiLxyAll")
  jpsiLxyAll.Add(jpsiLxyBdecay)
  jpsiLxyAll.Add(jpsiLxyBkg)
  jpsiLxyAll.SetLineColor(ROOT.kBlack)
  jpsiLxyAll.SetTitle("J/#psi pseudo proper decay length")
  jpsiLxyPrompt.SetLineColor(ROOT.kBlue)
  jpsiLxyBdecay.SetLineColor(ROOT.kRed)
  jpsiLxyBkg.SetLineColor(ROOT.kGreen)
  padQA.Clear()
  jpsiLxyAll.Draw()
  jpsiLxyAll.Write()
  jpsiLxyPrompt.Draw("same")
  jpsiLxyBdecay.Draw("same")
  jpsiLxyBkg.Draw("same")
  padQA.SetLogy(True)
    # Legend
  if(lgd):
    lgd.Delete("C")
  lgd = ROOT.TLegend(0.2, 0.5, 0.4, 0.85)
  lgd.AddEntry(jpsiLxyAll, "Total")
  lgd.AddEntry(jpsiLxyPrompt, "Prompt")
  lgd.AddEntry(jpsiLxyBdecay, "Non-Prompt")
  lgd.AddEntry(jpsiLxyBkg, "Background")
  lgd.Draw("same")
  padQA.Print(args.print, "Title:Jpsi_Lxy")
  padQA.Write('cJpsiLxy')
  return
# END - Drawing MC plots

if(args.mc):
  mc = outputs.Get("MChistos")
  DrawMC(mc)
  DrawMC_DetectorResponse(mc)
  # Delete all list
  mc.Delete("C")

# Close files
padQA.Clear()
PrintCover(padQA, args.print, '', isBack=True)
fin.Close()
fout.Write()
fout.Close()