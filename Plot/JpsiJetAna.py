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
from ana_util import HistNorm
import sys, os, json

resultFile = "AnalysisResults.root"
if(len(sys.argv) > 1):
  resultFile = sys.argv[1]

# Outputs from AliAnalysisTaskJpsiJet
f = ROOT.TFile(resultFile)
outputs = f.Get("JpsiJetAnalysis")
if(outputs == None):
  print("[X] ERROR - Outputs not found.")
  exit(1)

# Global Settings and Variables
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetPalette()

JPSI_MASS_LOWER = 2.92
JPSI_MASS_UPPER = 3.16
JPSI_LXY_PROMPT = 0.01
JPSI_LXY_BDECAY = 0.02

def DrawQA(qa, tag):
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
  incl.SetLineColor(ROOT.kBlack)
  incl.SetMarkerColor(ROOT.kBlack)
  incl.SetMarkerStyle(20)
  incl.Draw("EP")
  tagPrompt.SetLineColor(ROOT.kRed)
  tagPrompt.SetMarkerColor(ROOT.kRed)
  tagPrompt.SetMarkerStyle(21)
  tagPrompt.Draw("same EP")
  tagBdecay.SetLineColor(ROOT.kBlue)
  tagBdecay.SetMarkerColor(ROOT.kBlue)
  tagBdecay.SetMarkerStyle(22)
  tagBdecay.Draw("same EP")
  ## Legend
  lgd = ROOT.TLegend(0.6, 0.64, 0.85, 0.85)
  lgd.AddEntry(incl, "Inclusive jets")
  lgd.AddEntry(tagPrompt, "Prompt tagged jets")
  lgd.AddEntry(tagBdecay, "Non-prompt tagged jets")
  lgd.Draw("same")
  c.SaveAs("MC-JetPt.pdf")
  c.SaveAs("MC-JetPt.root")

qaName = outputs.GetListOfKeys().At(0).GetName()
#DrawQA(outputs.Get(qaName), qaName.split('_')[1])
DrawQA_JetPt(outputs.Get(qaName))

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

fIsMC = False
mc = outputs.Get("MChistos")
if(mc):
  fIsMC = True
  #DrawMC(mc)
  #DrawMC_DetectorResponse(mc)

f.Close()