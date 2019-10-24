#!/usr/bin/env python3

import sys, os, json, argparse

parser = argparse.ArgumentParser(description='Check AOD files')
parser.add_argument('filelist',help='Path list for reading', default="Periods.txt")
parser.add_argument('no',help='Number of processing', type=int, default=0)
args = parser.parse_args()

import ROOT

with open(args.filelist) as f:
  path = f.readlines()[args.no].strip()

f = ROOT.TFile(path + "/AliAOD.Dielectron.root")

print(path.split('/')[-2] + '\t' + repr(f.aodTree.GetEntries()))

f.Close()

