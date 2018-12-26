#!/bin/bash -

# Script for running ALICE jobs in local datasets

ALICE_OUTPUT_DIR=$1
ALICE_DATA_DIR=$2
ALICE_DATA_NO=$(printf "%03d" $3)
ALICE_DATA_FILE=$2/AliAOD_$ALICE_DATA_NO.root
ALICE_OUTPUT_DIR=$1/$ALICE_DATA_NO

source /data2/ytwu/.bashrc
mkdir -p $ALICE_OUTPUT_DIR
cp runAnlaysis.C AddTaskJPSIFilter_pp.C ConfigJpsi_cj_pp.C $ALICE_OUTPUT_DIR/
cd $ALICE_OUTPUT_DIR/
ln -s $ALICE_DATA_FILE AliAOD_input.root
cdali
aliroot -l -b -x -q runAnlaysis.C 1>run.out 2>run.err