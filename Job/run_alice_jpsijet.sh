#!/bin/bash -

# Script for running ALICE jobs in local datasets

ALICE_JOB_DIR=$1
ALICE_DATA_NO=$2
ALICE_OUTPUT_DIR=$ALICE_JOB_DIR/$ALICE_DATA_NO
ALICE_SW=AliPhysics/DQ-JpsiJet-1

set -x
mkdir -p $ALICE_OUTPUT_DIR
cp Job/CheckOutput.py Job/JpsiFilterOutput.log $ALICE_OUTPUT_DIR/
cd $ALICE_OUTPUT_DIR/
export PATH=/data2/ytwu/Software/bin:$PATH
export ALIBUILD_WORK_DIR=/data2/ytwu/Software/ALICE/sw
eval "`alienv shell-helper`"
alienv q --no-refresh | grep "AliPhysics" | sort | uniq | grep -v "latest"
alienv load $ALICE_SW --no-refresh
alienv list --no-refresh
./CheckOutput.py JpsiFilterOutput.log $ALICE_DATA_NO
alienv unload $ALICE_SW --no-refresh
set +x
