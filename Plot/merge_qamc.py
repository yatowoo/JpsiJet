#!/usr/bin/env python3

import sys, os, time, json
import argparse

# Command-line arguments
parser = argparse.ArgumentParser(description='QA script for MC with pT hard bins')
parser.add_argument('--sub',help='Subcycle: 16, 17 or 18', default='16')
args = parser.parse_args()

import ROOT
import ana_util

PT_HARD_BINS = [12, 16, 21, 28, 36, 45, 57, 70, 85, 100, -1]
QA_NAME = ['VtxZ', 'ElePt','EleDCAxy','EleDCAz', 'JetPt', 'JetNtrk', 'TagJetPt', 'TagJetNtrk']
QA = list(range(len(PT_HARD_BINS) - 1))
for i,pTmin in enumerate(PT_HARD_BINS[:-1]):
  QA[i] = {}
for i,pTmin in enumerate(PT_HARD_BINS[:-1]):
  # Init
  if(PT_HARD_BINS[i+1] == -1):
    QA[i]['Title'] = "p_{T} hard bin > %d GeV/c" % PT_HARD_BINS[i]
  else:
    QA[i]['Title'] = "p_{T} hard bin: %d - %d GeV/c" % (PT_HARD_BINS[i], PT_HARD_BINS[i+1])
  f = ROOT.TFile('../output/MCrequest/NewMC/JpsiJetMC_NewQA%s_bin%d_191002/AnalysisResults.root' % (args.sub, i+1))
  if(f.IsOpen()):
    print('>>> Processing ' + f.GetName())
    print('[-] Config : ' + QA[i]['Title'])
  else:
    exit(1)
  qa = f.JpsiJetAnalysis.Get('QAhistos_MC')
  qa.SetOwner(True)
  mc = f.JpsiJetAnalysis.Get('MChistos')
  mc.SetOwner(True)
  # QA
    # Event
  diele = qa.FindObject('Dielectron')
  diele.SetOwner(True)
  evQA = diele.FindObject('Event')
  evQA.SetOwner(True)
  QA[i]['VtxZ'] = evQA.FindObject('VtxZ').Clone('hVtxZ_%d' % i)
  QA[i]['NEvent'] = QA[i]['VtxZ'].GetEntries()
  print('[-] N event : %1.3e' % QA[i]['NEvent'])
    # Track / Electron
  eleP = diele.FindObject('Track_ev1+')
  eleP.SetOwner(True)
  QA[i]['ElePt'] = eleP.FindObject('Pt').Rebin(len(ana_util.BINNING_JPSI_PT)-1,'hElePt_%d' % i, ana_util.BINNING_JPSI_PT)
  QA[i]['EleDCAxy'] = eleP.FindObject('dXY').Clone('hEleDXY_%d' % i)
  QA[i]['EleDCAxy'].GetXaxis().SetRangeUser(-2.,2.)
  QA[i]['EleDCAz'] = eleP.FindObject('dZ').Clone('hEleDZ_%d' % i)
  QA[i]['EleDCAz'].GetXaxis().SetRangeUser(-5.,5.)
  eleN = diele.FindObject('Track_ev1-')
  eleN.SetOwner(True)
  QA[i]['ElePt'].Add(eleN.FindObject('Pt').Rebin(len(ana_util.BINNING_JPSI_PT)-1,'', ana_util.BINNING_JPSI_PT))
  QA[i]['EleDCAxy'].Add(eleN.FindObject('dXY'))
  QA[i]['EleDCAz'].Add(eleN.FindObject('dZ'))
    # Jet
  jetQA = qa.FindObject('Jet_AKTChargedR040_tracks_pT0150_pt_scheme')
  jetQA.SetOwner(True)
  hs = jetQA.FindObject('jetVars')
  QA[i]['JetPt'] = hs.Projection(0).Rebin(len(ana_util.BINNING_JET_PT)-1,'hJetPt_%d' % i, ana_util.BINNING_JET_PT)
  QA[i]['JetNtrk'] = jetQA.FindObject('Ntracks_pT').ProfileX('hJetNtrk_%d' % i).Rebin(len(ana_util.BINNING_JET_PT)-1,'', ana_util.BINNING_JET_PT)
  taggedJetQA = qa.FindObject('JpsiJet_AKTChargedR040_tracksWithPair_pT0150_pt_scheme')
  taggedJetQA.SetOwner(True)
  hs = taggedJetQA.FindObject('jetVars')
  QA[i]['TagJetPt'] = hs.Projection(0).Rebin(len(ana_util.BINNING_JET_PT)-1,'hTagJetPt_%d' % i, ana_util.BINNING_JET_PT)
  QA[i]['TagJetNtrk'] = taggedJetQA.FindObject('Ntracks_pT').ProfileX('hTagJetNtrk_%d' % i).Rebin(len(ana_util.BINNING_JET_PT)-1,'', ana_util.BINNING_JET_PT)
  for hist in QA_NAME:
    QA[i][hist].SetDirectory(None)
  # End
  qa.Delete()
  mc.Delete()
  f.Close()

print(QA)

c = ROOT.TCanvas('cQA','MC QA', 800, 600)
c.Draw()
for hist in QA_NAME:
  c.Clear()
  if(hist.find('Pt') > -1):
    c.SetLogy()
  histMin = 0
  histMax = 0
  for i,pTmin in enumerate(PT_HARD_BINS[:-1]):
    color = next(ana_util.COLOR)
    ana_util.SetColorAndStyle(QA[i][hist], color)
    QA[i][hist].Scale(1/QA[i]['NEvent'],'width')
    QA[i][hist].SetTitle(QA[i]['Title'])
    # Y Range
    hMin = QA[i][hist].GetMinimum()
    if(histMin > hMin):
      histMin = hMin
    hMax = QA[i][hist].GetMinimum()
    if(histMax < hMax):
      histMax = hMax
    if(i == 0):
      QA[i][hist].Draw('PE')
    else:
      QA[i][hist].Draw('same PE')
  c.BuildLegend()
  QA[0][hist].SetTitle(hist)
  QA[0][hist].GetYaxis().SetRangeUser(histMin, histMax)
  c.SaveAs('MCQA' + args.sub + '_' + hist + '.pdf')