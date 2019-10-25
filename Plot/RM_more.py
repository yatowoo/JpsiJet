#!/usr/bin/env python3

import ROOT
f = ROOT.TFile('RM_Full.root')
f.cRM.Draw()
hRM = f.cRM.FindObject('RM_0_0').Clone('hRM')
c = ROOT.TCanvas('c1','Temp')
c.Draw()
hRM.Draw('COLZ')
plt = hRM.GetListOfFunctions().FindObject('palette')
f.cRM.cd()
plt.SetX1NDC(0.8)
plt.SetY1NDC(0.05)
plt.SetX2NDC(0.85)
plt.SetY2NDC(0.88)
plt.Draw()

f.cRM.SaveAs('JpsiJet_SIMUL_BJetJpsi_RM_pp13TeV.eps')
f.cRM.SaveAs('JpsiJet_SIMUL_BJetJpsi_RM_pp13TeV.root')
f.cRM.SaveAs('JpsiJet_SIMUL_BJetJpsi_RM_pp13TeV.pdf')
f.cRM.SaveAs('JpsiJet_SIMUL_BJetJpsi_RM_pp13TeV.png')
f.cRM.SaveAs('tmp.C')
