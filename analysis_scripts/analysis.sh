#!/bin/bash
#!/bin/sh
echo
echo 'START---------------'
echo 'current dir: ' ${PWD}
cd /afs/cern.ch/work/m/malberti/MTD/TBatFNALApril2019/H4Analysis/
echo 'current dir: ' ${PWD}
source scripts/setup.sh
./bin/TimingAnalysisBars $1
echo 'STOP---------------'
echo
echo

