#!/bin/bash - 

# Script for aliensh

function list_jobs(){
  ps -l 20000 -A -b | grep -v "$USER -"
}

function list_subjobs(){
  masterjob $1 -printid | grep "Subjobs in DONE" | cut -d':' -f3 | /usr/bin/tr -d ',)'
}

function count_inputfiles(){
  ps -jdl $1 | grep "LF.*AliAOD.root" | wc
}

function resub_all(){
  MID_START=$1
  ERR_TYPE=$2
  for mid in $(list_jobs | awk '$2 >= "'$MID_START'" {print $2}');
  do
    echo $mid
    masterjob $mid -status $ERR_TYPE resubmit;
  done
}

function list_runs(){
  for run in $(ls $1/OutputAOD/ | grep 000);
  do
    echo $1/OutputAOD/$run
  done

}

function list_subjobs(){
  for run in $(ls $1/OutputAOD/ | grep 000);
  do
    run_path=$1/OutputAOD/$run
    for subjob in $(ls $run_path -F | grep /);
    do
      echo $run_path/$subjob
    done
  done
}

function clean_workdir(){
  # List runs
  for run in $(ls $1/OutputAOD/ | grep 000);
  do
    run_path=$1/OutputAOD/$run
    echo "[+] Run " $run " Found"
    # Delete extra outputs
    EXTRA_OUTPUT=(log_archive.zip root_archive.zip AliAOD.root stderr stdout)
    for output in ${EXTRA_OUTPUT[@]};
    do
      echo "[-] Delete extra outputs: " $run_path/$output
      rm $run_path/$output
    done
    # Remove output of subjobs - after MERGE
    for subjob in $(ls $run_path -F | grep /);
    do
      echo "[-] Remove subjob outputs: " $run_path/$subjob
      rmdir $run_path/$subjob
    done
  done  
}
