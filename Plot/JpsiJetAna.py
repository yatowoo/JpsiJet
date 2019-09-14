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

import ROOT
from ROOT import TFile, TCanvas, TLegend, gPad, TPaveText
from ROOT import TH1, TH2
from ana_util import HistNorm
import sys, os, json, argparse

# Command-line Arguments
parser = argparse.ArgumentParser(description='Post-processing script for AliAnalysisTaskJpsiJet')
parser.add_argument('-f', '--file',help='Analysis results of AliAnalysisTaskJpsiJet', default="AnalysisResults.root")
parser.add_argument('-o', '--output',help='Output file path', default='JpsiJetAna.root')
parser.add_argument('-p', '--print',help='Print in PDF file', default='JpsiJetAna.pdf')
parser.add_argument('--mc',help='MC flag for DrawMC methods', default=False, action='store_true')
parser.add_argument('--calo',help='QA for EMCal cluster energy', default=False, action='store_true')
parser.add_argument('--all',help='Run all QA processing', default=False, action='store_true')
args = parser.parse_args()

# Input - analysis results from ALICE tasks
fin = TFile(args.file)
outputs = fin.Get("JpsiJetAnalysis")
if(outputs == None):
  print("[X] ERROR - JpsiJetAnalysis not found.")
  exit(1)
# Output - ROOT file and PDF
fout = TFile(args.output, "RECREATE")
padQA = TCanvas("cAna", "JpsiJet Analysis",800,600)
# Cover
pTxt = TPaveText(0.4,0.45,0.6,0.55, "brNDC")
pTxt.AddText("Jpsi in jets analysis")
padQA.cd()
pTxt.Draw()
  # Open PDF file and draw cover
padQA.Print(args.print + '(', "Title:JpsiJet")
pTxt.Delete()
# End - Cover

# Global Settings and Variables
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetPalette()

JPSI_MASS_LOWER = 2.92
JPSI_MASS_UPPER = 3.16
JPSI_LXY_PROMPT = 0.01
JPSI_LXY_BDECAY = 0.02

TRIGGER_CLASSES = ['INT7', 'EG1', 'EG2', 'DG1', 'DG2']
TRIGGER_TAG = ['MB', 'EG1', 'EG2', 'DG1', 'DG2']

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

def DrawQA_JetPt(qa):
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

def DrawQA_Calo(qa):
  CaloQA = {}
  for trig in TRIGGER_CLASSES:
    CaloQA[trig] = {}
    CaloQA[trig]['NEvent'] = 0
    CaloQA[trig]['E'] = None
    CaloQA[trig]['RF'] = None
  # Event numbers
  evQA = qa.FindObject('Event_afterCuts') # TList
  evQA.SetOwner(True)
  hTrig = evQA.FindObject('FiredTag') # TH1D
  for i in range(1, hTrig.GetNbinsX() + 1):
    tag = hTrig.GetXaxis().GetBinLabel(i)
    if(tag == ''):
      break
    for trig in tag.split('_'):
      CaloQA[trig]['NEvent'] += hTrig.GetBinContent(i)
  # End - read with fired tags
  padQA.SetWindowSize(1600, 600)
  padQA.Divide(2)
  for trig in TRIGGER_CLASSES:
    calo = qa.FindObject('Cluster_' + trig) # TList
    calo.SetOwner(True)
    caloE = calo.FindObject('ClsVars').Projection(0)
    caloE.SetName('hClusterE_' + trig)
    HistNorm(caloE, CaloQA[trig]['NEvent'])
    caloE.GetXaxis().SetRangeUser(0, 40)
    caloE.GetYaxis().SetRangeUser(1e-8, 100.)
    padQA.cd(1)
    caloE.Draw("same PE")
    gPad.SetLogy()
    if(CaloQA['INT7']['E']):
      caloRF = caloE.Clone('hClusterRF_' + trig)
      caloRF.GetXaxis().SetRangeUser(0, 40)
      caloRF.GetYaxis().SetRangeUser(0.1, 1e4)
      caloRF.Divide(CaloQA['INT7']['E'])
      padQA.cd(2)
      caloRF.Draw("same PE")
      gPad.SetLogy()
      CaloQA[trig]['RF'] = caloRF
    CaloQA[trig]['E'] = caloE
    calo.Delete()
  # End - trigger loop
  padQA.Print(args.print,'Title:ClusterQA')
  padQA.SaveAs('../output/QM19/ClusterQA.root') # Debug

# Calo cluster QA


if(args.all):
  qaName = outputs.GetListOfKeys().At(0).GetName()
  #DrawQA(outputs.Get(qaName), qaName.split('_')[1])
  #DrawQA_JetPt(outputs.Get(qaName))
elif(args.calo):
  qaName = "QAhistos_ALL"
  qa = outputs.Get(qaName)
  qa.SetOwner(True)
  DrawQA_Calo(qa)
  qa.Delete()

# Detectro response matrix - 4 dim.
## THnSparse: z_det, z_gen, jetPt_det, jetPt_gen
def GetDetectorResponse_FF(drm, tag):
  drm.GetAxis(0).SetRangeUser(0., 1.)
  drm.GetAxis(1).SetRangeUser(0., 1.)
  drm.GetAxis(2).SetRangeUser(20., 60.)
  drm.GetAxis(3).SetRangeUser(20., 60.)
  zMatrix = drm.Projection(1,0)
  zMatrix.SetTitle("Detector response matrix - J/#psi in Jet (z) ("+tag+")")
  return zMatrix
def GetDetectorResponse_JetPt(drm, tag):
  drm.GetAxis(2).SetRangeUser(0., 60.)
  drm.GetAxis(3).SetRangeUser(0., 60.)
  ptMatrix = drm.Projection(3,2)
  ptMatrix.SetTitle("Detector response matrix - J/#psi tagged jet p_{T} ("+tag+")")
  return ptMatrix
# MChistos
def DrawMC_DetectorResponse(mc):
  # Canvas
  c = ROOT.TCanvas("cMC","Detector Response", 1600, 600)
  c.Divide(2)
  c.SetTicks()
  c.Draw()
  # Histos
  drmPrompt = mc.FindObject("JpsiPrompt").FindObject("Jet_DetResponse")
  drmBdecay = mc.FindObject("JpsiBdecay").FindObject("Jet_DetResponse")
  # jet pT
  ptPrompt = GetDetectorResponse_JetPt(drmPrompt, "Prompt")
  ptBdecay = GetDetectorResponse_JetPt(drmBdecay, "Non-prompt")
  c.cd(1)
  ptPrompt.Draw("COLZ")
  ROOT.gPad.SetLogz()
  c.cd(2)
  ptBdecay.Draw("COLZ")
  ROOT.gPad.SetLogz()
  c.SaveAs("DetRes-jetPt.root")
  c.SaveAs("DetRes-jetPt.pdf")
  # z - Fragmentation Function
  zPrompt = GetDetectorResponse_FF(drmPrompt, "Prompt")
  zBdecay = GetDetectorResponse_FF(drmBdecay, "Non-prompt")
  c.cd(1)
  zPrompt.Draw("COLZ")
  ROOT.gPad.SetLogz()
  c.cd(2)
  zBdecay.Draw("COLZ")
  ROOT.gPad.SetLogz()
  c.SaveAs("DetRes-FF.root")
  c.SaveAs("DetRes-FF.pdf")
  c.Clear()

def DrawMC(mc):
  print("[-] INFO - Processing MC plots")
  mc.SetOwner(True)
  # J/psi generator info
  # Electron PID
  c = ROOT.TCanvas("cMC","MC canvas", 800, 600)
  c.SetGrid()
  c.SetTicks()
  c.Draw()
  c.cd()
  mcEle = mc.FindObject("Electron")
  mcEle.SetOwner(True)
    # TPC
  eleAllTPC = mcEle.FindObject("eleVars").Projection(0)
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
    # EMCal
  eleAllEMC = mcEle.FindObject("EMCal_all")
  eleDetEMC = mcEle.FindObject("EMCal_det")
  eleEffEMC = eleDetEMC.Clone("hEleEffTPC")
  eleEffEMC.Divide(eleAllEMC)
  eleEffEMC.SetLineColor(ROOT.kRed)
  eleEffEMC.SetLineWidth(2)
  #eleEffEMC.Rebin(5) # Bin width : 0.2 -> 1.0
  eleEffEMC.GetXaxis().SetRangeUser(5.0, 45)
  eleEffEMC.Draw("same")
    # Legend
  lgd = ROOT.TLegend(0.6, 0.7, 0.85, 0.85)
  lgd.AddEntry(eleEffTPC, "TPC")
  lgd.AddEntry(eleEffEMC, "TPC + EMCal/DCal")
  lgd.Draw("same")
  c.SaveAs("ElectronPID_eff.pdf")
  c.SaveAs("ElectronPID_eff.root")
  # Electron Purity
  elePurityTPC = elePureTPC.Clone("hElePurityTPC")
  elePurityTPC.Divide(eleTPC)
  elePurityTPC.GetXaxis().SetRangeUser(0., 50.)
  elePurityTPC.GetYaxis().SetRangeUser(0.97, 1.01)
  elePurityTPC.GetYaxis().SetTitle("Purity (%)")
  elePurityTPC.GetYaxis().SetTitleOffset(1.5)
  elePurityTPC.SetTitle("Electron PID purity by TPC only")
  elePurityTPC.SetLineWidth(2)
  c.Clear()
  elePurityTPC.Draw()
  c.SaveAs("ElectronPID_purity.pdf")
  c.SaveAs("ElectronPID_purity.root")
  ###
  # J/psi reconstruction
    # Prompt
  jpsiPrompt = mc.FindObject("JpsiPrompt")
  jpsiPrompt.SetOwner(True)
    # THnSparse - pT, Y, phi, E
  jpsiPromptAll = jpsiPrompt.FindObject("jpsiVars").Projection(0)
    # THnSparse - pT, Mee, Lxy
  jpsiPromptReco = jpsiPrompt.FindObject("Reco_sig")
  jpsiPromptReco.GetAxis(1).SetRangeUser(JPSI_MASS_LOWER, JPSI_MASS_UPPER)
  jpsiPromptPt = jpsiPromptReco.Projection(0)
  jpsiLxyPrompt = jpsiPromptReco.Projection(2)
    # Prompt background
  jpsiPromptBkg = jpsiPrompt.FindObject("Reco_bkg")
  jpsiPromptBkg.GetAxis(1).SetRangeUser(JPSI_MASS_LOWER, JPSI_MASS_UPPER)
  jpsiLxyBkg = jpsiPromptBkg.Projection(2)
    # Prompt Eff.
  jpsiPromptEff = jpsiPromptPt.Clone("hJpsiEffPrompt")
  jpsiPromptEff.Divide(jpsiPromptAll)
  jpsiPromptEff.SetLineWidth(2)
  jpsiPromptEff.SetTitle("J/#psi Reconstruction - Acceptance and Efficiency")
  c.Clear()
  jpsiPromptEff.GetXaxis().SetRangeUser(0., 20.)
  jpsiPromptEff.GetYaxis().SetTitle("A #times #varepsilon")
  jpsiPromptEff.Draw()
    # Non-prompt / B-decay
  jpsiBdecay = mc.FindObject("JpsiBdecay")
  jpsiBdecay.SetOwner(True)
    # THnSparse - pT, Y, phi, E
  jpsiBdecayAll = jpsiBdecay.FindObject("jpsiVars").Projection(0)
    # THnSparse - pT, Mee, Lxy
  jpsiBdecayReco = jpsiBdecay.FindObject("Reco_sig")
  jpsiBdecayReco.GetAxis(1).SetRangeUser(JPSI_MASS_LOWER, JPSI_MASS_UPPER)
  jpsiBdecayPt = jpsiBdecayReco.Projection(0)
  jpsiLxyBdecay = jpsiBdecayReco.Projection(2)
    # Non-prompt background
  jpsiBdecayBkg = jpsiBdecay.FindObject("Reco_bkg")
  jpsiBdecayBkg.GetAxis(1).SetRangeUser(JPSI_MASS_LOWER, JPSI_MASS_UPPER)
  jpsiLxyBkg.Add(jpsiBdecayBkg.Projection(2))
  jpsiBdecayEff = jpsiBdecayPt.Clone("hJpsiEffBdecay")
  jpsiBdecayEff.Divide(jpsiBdecayAll)
  jpsiBdecayEff.SetLineColor(ROOT.kRed)
  jpsiBdecayEff.SetLineWidth(2)
  jpsiBdecayEff.Draw("same")
    # Legend
  if(lgd):
    lgd.Delete("C")
  lgd = ROOT.TLegend(0.2, 0.5, 0.4, 0.65)
  lgd.AddEntry(jpsiPromptEff, "Prompt")
  lgd.AddEntry(jpsiBdecayEff, "Non-prompt")
  lgd.Draw("same")
  c.SaveAs("MC_JpsiEff.pdf")
  c.SaveAs("MC_JpsiEff.root")
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
  c.Clear()
  jpsiLxyAll.Draw()
  jpsiLxyPrompt.Draw("same")
  jpsiLxyBdecay.Draw("same")
  jpsiLxyBkg.Draw("same")
  c.SetLogy(True)
    # Legend
  if(lgd):
    lgd.Delete("C")
  lgd = ROOT.TLegend(0.2, 0.5, 0.4, 0.85)
  lgd.AddEntry(jpsiLxyAll, "Total")
  lgd.AddEntry(jpsiLxyPrompt, "Prompt")
  lgd.AddEntry(jpsiLxyBdecay, "Non-Prompt")
  lgd.AddEntry(jpsiLxyBkg, "Background")
  lgd.Draw("same")
  c.SaveAs("MC_JpsiLxy.pdf")
  c.SaveAs("MC_JpsiLxy.root")
  # Delete all list
  mc.Delete("C")
  return
# END - Drawing MC plots

if(args.mc):
  mc = outputs.Get("MChistos")
  DrawMC(mc)
  DrawMC_DetectorResponse(mc)

# Close files
padQA.Clear()
padQA.Print(args.print + ')', "Title:End")
fin.Close()
fout.Write()
fout.Close()