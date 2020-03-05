#!/usr/bin/env python3

# Test TH2D rebinning

import ROOT
from array import array

f = ROOT.TFile("output/AnaMerge_Full_Train.root")

# z vs pT,jet
h2data = f.TagInfoH.Projection(3,5)
NX_DATA = h2data.GetNbinsX()
xdata = h2data.GetXaxis()
NY_DATA = h2data.GetNbinsY()
ydata = h2data.GetYaxis()

# Binning for processing (raw, result and final)
  # Low-edge
BINNING_Z = [0.1*x for x in range(0,11,1)]
NY = len(BINNING_Z) - 1
BINNING_Z = array('d', BINNING_Z)

BINNING_PT = list(range(10,20,2))
BINNING_PT += list(range(20,40,5))
NX = len(BINNING_PT) - 1
BINNING_PT = array('d', BINNING_PT)
# Histogram setup
h2 = ROOT.TH2D("h2zpt","#it{z}(#it{p}_{T,J/#psi}/p_{T,jet}) vs #it{p}_{T,jet}", NX, BINNING_PT, NY, BINNING_Z)
for i in range(1, NX_DATA+1, 1):
  for j in range(1, NY_DATA+1, 1):
    h2.Fill(xdata.GetBinCenter(i), ydata.GetBinCenter(j), h2data.GetBinContent(i,j))

c = ROOT.TCanvas('c1','FF',1600,800)
c.Divide(2)
c.cd(1)
h2data.Draw("COLZ")
c.cd(2)
h2.Draw("COLZ")