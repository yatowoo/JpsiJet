#!/usr/bin/env python3

# Test script for AnalysisResults.root
# Based on ROOT6

import ROOT
from math import *
import sys
import os

fcn = ROOT.TF1("fNorm","[0]",-1e6,1e6)

def HistoNorm(h, nEvent):
  if(not nEvent):
    nEvent = h.GetEntries()
  normFactor = nEvent * h.GetBinWidth(1)
  print('[+] Histogram Normalization Factor: ' + repr(normFactor))
  fcn.SetParameter(0, normFactor)
  h.Divide(fcn)
  for i in range(1,h.GetNbinsX()+1):
    h.SetBinError(i, h.GetBinWidth(i) / sqrt(normFactor))
  return h

def HistoStyle(h, color, marker):
  h.SetLineColor(color)
  h.SetMarkerColor(color)
  h.SetMarkerStyle(marker)
  h.SetMarkerSize(1.2)

def PtFromPWGJEQA(jetQA, jetName, hName):
  jets = jetQA.FindObject(jetName)
  jets.SetOwner(True)
  h = jets.FindObject("fHistJetObservables").Projection(2)
  hnew = h.Clone(hName)
  h.Delete()
  return hnew

if(len(sys.argv) == 1):
  anaResult = "AnalysisResults.root"
else:
  anaResult = sys.argv[1]


anaFile = ROOT.TFile(anaResult)
jetEGA_QA = anaFile.Get("AliAnalysisTaskPWGJEQA_tracks_caloClusters_emcalCells_histos")
jetEGA_QA.SetOwner(True)
jetMB_QA = anaFile.Get("AliAnalysisTaskEmcalJetSpectraQA_histos")
jetMB_QA.SetOwner(True)

nEGA = jetEGA_QA.FindObject("fHistZVertex").GetEntries()
nMB = jetMB_QA.FindObject("fHistZVertex").GetEntries()

jets02 = "Jet_AKTChargedR020_tracks_pT0150_pt_scheme"
jets04 = "Jet_AKTChargedR040_tracks_pT0150_pt_scheme"
jets02_EGA = PtFromPWGJEQA(jetEGA_QA, jets02, "hEGA02") 
HistoNorm(jets02_EGA, nEGA)
HistoStyle(jets02_EGA, ROOT.kBlack, 24)
jets04_EGA = PtFromPWGJEQA(jetEGA_QA, jets04, "hEGA04") 
HistoNorm(jets04_EGA, nEGA)
HistoStyle(jets04_EGA, ROOT.kRed, 20)

cSpectra = ROOT.TCanvas("cJetPt","Jet Spectra", 800, 600)
cSpectra.SetLogy()
ROOT.gStyle.SetOptStat(False)
jets02_EGA.Draw("E")
h = jets02_EGA
h.SetTitle("Raw p_{T} spectra of jets")
h.SetYTitle("1/N_{ev} dN/dp_{T}")
h.GetXaxis().SetRangeUser(0, 50)
h.GetYaxis().SetRangeUser(1e-8, 10)
jets04_EGA.Draw("same")
ratio_EGA = ROOT.TRatioPlot(jets02_EGA, jets04_EGA)
ratio_EGA.SetSeparationMargin(0.)
ratio_EGA.Draw()
ratio_EGA.GetLowerRefGraph().SetMinimum(0)
ratio_EGA.GetLowerRefGraph().SetMaximum(2)
h.Draw("same E")
cSpectra.Update()
cSpectra.SaveAs("JetPt.png")

jetEGA_QA.Delete("slow")
jetMB_QA.Delete("slow")

qaFile = ROOT.TFile("JetPt.root","RECREATE")
jets02_EGA.Write()
jets04_EGA.Write()
cSpectra.Write()



