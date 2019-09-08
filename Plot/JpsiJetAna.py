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

JPSI_MASS_LOWER = 2.92
JPSI_MASS_UPPER = 3.16

def DrawQA(qa, tag):
  print("[-] INFO - Processing QA plots for " + tag)
  qa.SetOwner(True)
  qa.Delete("C")
  return
# END - Drawing QA plots

qaName = outputs.GetListOfKeys().At(0).GetName()
DrawQA(outputs.Get(qaName), qaName.split('_')[1])

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
  DrawMC(mc)

f.Close()