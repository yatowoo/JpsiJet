#!/usr/bin/env python3

import sys, os, time, json
import argparse

# Command-line arguments
parser = argparse.ArgumentParser(description='QA script for MC with pT hard bins')
parser.add_argument('--sub',help='Subcycle: 16, 17 or 18', default='16')
parser.add_argument('-o', '--output',help='Output ROOT file', default='NewMCQA.root')
parser.add_argument('-p', '--print',help='Output PDF', default='NewMCQA.pdf')
args = parser.parse_args()

import ROOT
import ana_util

fout = ROOT.TFile(args.output,'RECREATE')
c = ROOT.TCanvas('cQA','J/#psi in jets MC production - QA 10%% (LHC%s)' % args.sub, 800, 600)
c.Draw()

ana_util.PrintCover(c,args.print)
SCALE_FACTOR = [2.59e-2, 1.35e-2, 7.41e-3, 2.95e-3, 1.21e-3, 5.67e-4, 2.54e-4, 1.17e-4, 4.71e-5, 5.18e-5]
PT_HARD_BINS = [12, 16, 21, 28, 36, 45, 57, 70, 85, 100, -1]
BINNING_PT = ana_util.BINNING_JET_PT
QA_NAME = ['VtxZ', 'ElePt','EleDCAxy','EleDCAz', 'JetPt', 'JetNtrk', 'TagJetPt', 'TagJetNtrk', 'DieleJetPt', 'DieleJetNtrk', 'JpsiPt', 'JpsiY']
QA_HIST_CONFIG = {
  'VtxZ':{'Logy':False, 'X':[-20,20], 'Y': [0., 0.1], 'Ytitle': '1/N_{ev} dN_{ev}/dZ', 'Legend': [0.1,0.55,0.4,0.9], 'Title':'Event primary vertex Z', 'Sum':False},
  'ElePt':{'Logy':True, 'X':[0,100], 'Y': [1e-10, 10], 'Ytitle': '1/N_{ev} dN_{trk}/dp_{T}', 'Legend': [0.68,0.62,0.9,0.9], 'Title':'Selected track/electron (TPC only) - p_{T}', 'Sum':True},
  'EleDCAxy':{'Logy':True, 'X':[-2,2], 'Y': [1e-4, 100], 'Ytitle': '1/N_{ev} dN_{trk}/dXY', 'Legend': [0.1,0.55,0.4,0.9], 'Title':'Selected track/electron (TPC only) - DCA_{xy}', 'Sum':False},
  'EleDCAz':{'Logy':True, 'X':[-5,5], 'Y': [1e-6, 100], 'Ytitle': '1/N_{ev} dN_{trk}/dZ', 'Legend': [0.1,0.55,0.4,0.9], 'Title':'Selected track/electron (TPC only) - DCA_{z}', 'Sum':False},
  'JetPt':{'Logy':True, 'X':[0,100], 'Y': [1e-8, 10], 'Ytitle': '1/N_{ev} dN_{jet}/dp_{T}', 'Legend': [0.68,0.62,0.9,0.9], 'Title':'Inclusive jet - raw p_{T}', 'Sum':True},
  'JetNtrk':{'Logy':False, 'X':[0,100], 'Y': [0, 12], 'Ytitle': '<N_{trk}>', 'Legend': [0.1,0.6,0.3,0.9], 'Title':'Inclusive jet - N constituents', 'Sum':True},
  'TagJetPt':{'Logy':True, 'X':[0,100], 'Y': [1e-9, 1], 'Ytitle': '1/N_{ev} dN_{jet}/dp_{T}', 'Legend': [0.68,0.62,0.9,0.9], 'Title':'Inclusive jet (updated) - raw p_{T}', 'Sum':True},
  'TagJetNtrk':{'Logy':False, 'X':[0,100], 'Y': [0, 12], 'Ytitle': '<N_{trk}>', 'Legend': [0.1,0.6,0.3,0.9], 'Title':'Inclusive jet (updated) - N constituents', 'Sum':True},
  'DieleJetPt':{'Logy':True, 'X':[0,100], 'Y': [1e-9, 1e-1], 'Ytitle': '1/N_{ev} dN_{jet}/dp_{T}', 'Legend': [0.68,0.62,0.9,0.9], 'Title':'Dielectron tagged jet - raw p_{T}', 'Sum':True},
  'DieleJetNtrk':{'Logy':False, 'X':[0,100], 'Y': [0, 12], 'Ytitle': '<N_{trk}>', 'Legend': [0.1,0.6,0.3,0.9], 'Title':'Dielectron tagged jet - N constituents', 'Sum':True},
  'JpsiPt':{'Logy':True, 'X':[0,100], 'Y': [1e-8, 10], 'Ytitle': '1/N_{ev} dN_{J/#psi}/dp_{T}', 'Legend': [0.68,0.62,0.9,0.9], 'Title':'Generated J/#psi - p_{T}', 'Sum':True},
  'JpsiY':{'Logy':False, 'X':[-2, 2], 'Y': [0, 1], 'Ytitle': '1/N_{ev} dN_{J/#psi}/dY', 'Legend': [0.75,0.5,0.9,0.9], 'Title':'Generated J/#psi - Y', 'Sum':False}
}
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
  QA[i]['ElePt'] = eleP.FindObject('Pt').Rebin(len(BINNING_PT)-1,'hElePt_%d' % i, BINNING_PT)
  QA[i]['EleDCAxy'] = eleP.FindObject('dXY').Clone('hEleDXY_%d' % i)
  QA[i]['EleDCAxy'].GetXaxis().SetRangeUser(-2.,2.)
  QA[i]['EleDCAz'] = eleP.FindObject('dZ').Clone('hEleDZ_%d' % i)
  QA[i]['EleDCAz'].GetXaxis().SetRangeUser(-5.,5.)
  eleN = diele.FindObject('Track_ev1-')
  eleN.SetOwner(True)
  QA[i]['ElePt'].Add(eleN.FindObject('Pt').Rebin(len(BINNING_PT)-1,'', BINNING_PT))
  QA[i]['EleDCAxy'].Add(eleN.FindObject('dXY'))
  QA[i]['EleDCAz'].Add(eleN.FindObject('dZ'))
    # Jet
  jetQA = qa.FindObject('Jet_AKTChargedR040_tracks_pT0150_pt_scheme')
  jetQA.SetOwner(True)
  hs = jetQA.FindObject('jetVars')
  QA[i]['JetPt'] = hs.Projection(0).Rebin(len(BINNING_PT)-1,'hJetPt_%d' % i, BINNING_PT)
  QA[i]['JetNtrk'] = jetQA.FindObject('Ntracks_pT').ProfileX('hJetNtrk_%d' % i).Rebin(len(BINNING_PT)-1,'', BINNING_PT)
  taggedJetQA = qa.FindObject('JpsiJet_AKTChargedR040_tracksWithPair_pT0150_pt_scheme')
  taggedJetQA.SetOwner(True)
  hs = taggedJetQA.FindObject('jetVars')
  QA[i]['TagJetPt'] = hs.Projection(0).Rebin(len(BINNING_PT)-1,'hTagJetPt_%d' % i, BINNING_PT)
  QA[i]['TagJetNtrk'] = taggedJetQA.FindObject('Ntracks_pT').ProfileX('hTagJetNtrk_%d' % i).Rebin(len(BINNING_PT)-1,'', BINNING_PT)
    # Dielectron tagged
  tagQA = qa.FindObject('PairInJet')
  tagQA.SetOwner(True)
  hs = tagQA.FindObject('PairVars')
  QA[i]['DieleJetPt'] = hs.Projection(5).Rebin(len(BINNING_PT)-1,'hDieleJetPt_%d' % i, BINNING_PT)
  QA[i]['DieleJetNtrk'] = tagQA.FindObject('Ntracks_pT').ProfileX('hDieleJetNtrk_%d' % i).Rebin(len(BINNING_PT)-1,'', BINNING_PT)
  # MC J/psi
  jpsiQA = mc.FindObject('JpsiBdecay')
  hs = jpsiQA.FindObject('jpsiVars')
  QA[i]['JpsiPt'] = hs.Projection(0).Rebin(len(BINNING_PT)-1,'hJpsiPt_%d' % i, BINNING_PT)
  QA[i]['JpsiY'] = hs.Projection(1)
  QA[i]['JpsiY'].SetName('hJpsiY_%d' % i)
  # End
  for hist in QA_NAME:
    QA[i][hist].SetDirectory(None)
  qa.Delete()
  mc.Delete()
  f.Close()

fout.cd()
for hist in QA_NAME:
  c.Clear()
  c.SetLogy(False)
  ana_util.COLOR = ana_util.SelectColor()
  ana_util.MARKER = ana_util.SelectMarker()
  # Hist config
  cfg = QA_HIST_CONFIG[hist]
  lgd = ROOT.TLegend(cfg['Legend'][0], cfg['Legend'][1], cfg['Legend'][2], cfg['Legend'][3])
  hSum = QA[0][hist].Clone('hSum'+hist)
  hSum.Reset()
  ana_util.SetColorAndStyle(hSum, ROOT.kRed, ana_util.kRound, 1.5)
  lgd.AddEntry(hSum, 'Sum')
  for i,pTmin in enumerate(PT_HARD_BINS[:-1]):
    color = next(ana_util.COLOR)
    ana_util.SetColorAndStyle(QA[i][hist], color)
    QA[i][hist].Sumw2()
    if(hist.find('Ntrk') < 0):
      QA[i][hist].Scale(1./QA[i]['NEvent'],'width')
    else:
      QA[i][hist].Scale(1.,'width')
    QA[i][hist].SetTitle(QA[i]['Title'])
    hSum.Add(QA[i][hist], SCALE_FACTOR[i])
    if(i == 0):
      QA[i][hist].Draw('PE')
    else:
      QA[i][hist].Draw('same PE')
    lgd.AddEntry(QA[i][hist],QA[i][hist].GetTitle())
  if(cfg['Sum']):
    hSum.Draw('same')
  # Hist config
  lgd.Draw('same')
  c.SetLogy(cfg['Logy'])
  QA[0][hist].SetYTitle(cfg['Ytitle'])
  QA[0][hist].GetXaxis().SetRangeUser(cfg['X'][0], cfg['X'][1])
  QA[0][hist].GetYaxis().SetRangeUser(cfg['Y'][0], cfg['Y'][1])
  QA[0][hist].SetTitle(cfg['Title'])
  c.Modified()
  # Output
  c.Print(args.print, 'Title:' + hist)
  c.Write('c' + hist)

ana_util.PrintCover(c,args.print,'-',isBack=True)
fout.Close()