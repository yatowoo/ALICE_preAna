#!/bin/bash -

# Script for running ALICE jobs in local datasets

ALICE_OUTPUT_DIR=$1
ALICE_DATA_PATTERN=$2
filelist=($(ls $ALICE_DATA_PATTERN))
ALICE_DATA_FILE=${filelist[$3]}
data_dir=$(basename $(dirname $ALICE_DATA_FILE))
ALICE_OUTPUT_DIR=$1/$data_dir

set -x
mkdir -p $ALICE_OUTPUT_DIR
cp *.C *.h *.cxx $ALICE_OUTPUT_DIR/
cd $ALICE_OUTPUT_DIR/
ln -s -f $ALICE_DATA_FILE AliAOD_input.root
export PATH=/data2/ytwu/Software/bin:$PATH
export ALIBUILD_WORK_DIR=/data2/ytwu/Software/ALICE/opt
eval "`alienv shell-helper`"
alienv q --no-refresh | grep -v "latest"
alienv load AliPhysics::vAN-20181208 --no-refresh
alienv list --no-refresh
aliroot -l -b -x -q runAnalysis.C 1>run.out 2>run.err
alienv unload AliPhysics::vAN-20181208 --no-refresh
alienv list --no-refresh
set +x
