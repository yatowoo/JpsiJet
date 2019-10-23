#!/usr/bin/env python3

# Detector response matrix - 4D
# jet pT bins [10, 15, 25], [20,25,35,]

import ROOT
import ana_util
import ana_phys

f = ROOT.TFile('../output/MCrequest/AnaMC_JpsiBJets.root')

mc = f.JpsiJetAnalysis.Get('MChistos')

jpsi = mc.FindObject('JpsiBdecay')

response = jpsi.FindObject('Jet_DetResponse')
# THnSparse - z_det, z_gen, pTjet_det, pTjet_gen, dZ, dpTjet, dpTJ/psi

ana_util.ALICEStyle()
ROOT.gStyle.SetPalette(ROOT.kInvertedDarkBodyRadiator)

c = ROOT.TCanvas('cRM','Detector response matrix - 4D',1600,1600)
c.Draw()
ana_util.SetPadMargin(c,0,0,0,0)
PAD_RM = ROOT.TPad("padBody","Body of response matrix", 0.02, 0.02, 0.8, 0.85)
ana_util.SetPadMargin(PAD_RM,0.1,0.,0.,0.1)
PAD_RM.Divide(3,3,0,0)
PAD_RM.Draw()
JET_PT_BINS = [10,15,35,100]
RM = [[1,2,3],[4,5,6],[7,8,9]]
PAD_INDEX = [7,4,1,8,5,2,9,6,3]
PAVE = [[1,2,3],[4,5,6],[7,8,9]]
response.GetAxis(0).SetRangeUser(0,1)
response.GetAxis(1).SetRangeUser(0,1)

# pT bin


for i in range(3):
  response.GetAxis(2).SetRangeUser(JET_PT_BINS[i],JET_PT_BINS[i+1])
  SUM = None
  for j in range(3):
    response.GetAxis(3).SetRangeUser(JET_PT_BINS[j],JET_PT_BINS[j+1])
    RM[i][j] = response.Projection(1,0)
    RM[i][j].SetName('RM_%d_%d' % (i, j))
    if(SUM is None):
      SUM = RM[i][j].Clone('hSum_%d' % i)
    else:
      SUM.Add(RM[i][j])
  for j in range(3):
    NX = SUM.GetNbinsX()
    NY = SUM.GetNbinsY()
    for ix in range(1,NX+1):
      SumY = SUM.Integral(ix, ix, 1, NY)
      if(SumY == 0):
        continue
      for iy in range(1, NY+1):
        val = RM[i][j].GetBinContent(ix, iy) / SumY
        RM[i][j].SetBinContent(ix, iy, val)
    PAD_RM.cd(PAD_INDEX[3*i+j])
    RM[i][j].SetTitle('')
    RM[i][j].SetMaximum(1.0)
    RM[i][j].SetMinimum(0.0)
    if(3*i + j != 0):
      RM[i][j].SetXTitle('')
      RM[i][j].GetXaxis().SetTitleSize(0.)
      RM[i][j].GetXaxis().SetLabelSize(0.)
      RM[i][j].SetYTitle('')
      RM[i][j].GetYaxis().SetTitleSize(0.)
      RM[i][j].GetYaxis().SetLabelSize(0.)
    RM[i][j].Draw('COLZ')
    PAVE[i][j] = ROOT.TPaveText(0.15,0.8,0.4,0.95,'brNDC')
    PAVE[i][j].SetName('TxtCuts_%d_%d' % (i,j))
    PAVE[i][j].SetFillColor(0)
    if(i==2):
      PAVE[i][j].AddText('p_{T,det} > %d GeV' % JET_PT_BINS[i])
    else:
      PAVE[i][j].AddText('%d < p_{T,det} < %d GeV' % (JET_PT_BINS[i],JET_PT_BINS[i+1]))
    if(j==2):
      PAVE[i][j].AddText('p_{T,gen} > %d GeV' % JET_PT_BINS[j])
    else:
      PAVE[i][j].AddText('%d < p_{T,gen} < %d GeV' % (JET_PT_BINS[j],JET_PT_BINS[j+1]))
    #PAVE[i][j].Draw('same')

c.SaveAs('RM_Full.root')
c.SaveAs('RM_Full.pdf')

f.Close()