#!/bin/bash -

# Script for running ALICE jobs in local datasets

ALICE_JOB_DIR=$1
ALICE_DATA_DIR=$2
let "no=$3+1"
ALICE_DATA_NO=$(printf "%03d" $no)
ALICE_DATA_FILE=$ALICE_DATA_DIR/AOD_$ALICE_DATA_NO.root
ALICE_OUTPUT_DIR=$ALICE_JOB_DIR/$ALICE_DATA_NO

set -x
mkdir -p $ALICE_OUTPUT_DIR
cp runAnalysis.C */*.C */*.h */*.cxx $ALICE_OUTPUT_DIR/
cd $ALICE_OUTPUT_DIR/
ln -s -f $ALICE_DATA_FILE AliAOD_input.root
export PATH=/data2/ytwu/Software/bin:$PATH
export ALIBUILD_WORK_DIR=/data2/ytwu/Software/ALICE/sw
eval "`alienv shell-helper`"
alienv q --no-refresh | grep -v "latest"
alienv load AliPhysics/vAN-20190522 --no-refresh
alienv list --no-refresh
aliroot -l -b -x -q 'runAnalysis.C(0,0,1,0,0,0)' 1>run.out 2>run.err
alienv unload AliPhysics/vAN-20190522 --no-refresh
alienv list --no-refresh
set +x
