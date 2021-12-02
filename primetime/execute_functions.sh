#!/bin/bash

LOGNAME="log"

FILEPATH=../${LOGNAME}/
LOGFILE=${LOGNAME}/${LOGNAME}.txt

echo "Increasing stack size..."
ulimit -s unlimited

function clean {
  mkdir -p ${LOGNAME}
  rm -rf ./${LOGNAME}/${FILEPATH}*
  touch  ./${LOGNAME}/${FILEPATH}/output.txt
  touch  ${LOGFILE}
}

function begin {
  echo "Begin" >> ${LOGFILE}
  echo "  $(date)" >> ${LOGFILE}
}

function end {
  echo "End" >> ${LOGFILE}
  echo "  $(date)" >> ${LOGFILE}
}

function createPrimes {
  echo "Create Primes" >> ${LOGFILE}
  echo "  $(date)" >> ${LOGFILE}

  pushd primebot
  python3 main.py createPrimes ${2} ${FILEPATH}${1}.txt
  popd
}

function execution {
  echo "Execution -> ${2}" >> ${LOGFILE}
  echo "  $(date)" >> ${LOGFILE}
  
  pushd primebot
  python3 main.py executionPrepare ${FILEPATH}${1}.txt
  popd
  
  if [ -z "$4" ]
  then
  
    pushd primeapp
    echo "#define PRIME_ITERATIONS ${3}" > params.h
    make
    popd

    rep=0
    while [ $rep -eq 0 ]
    do
      pushd primeapp
      ./app
      rep=${?}
      cp ${FILEPATH}output.txt ${FILEPATH}${2}.txt
      popd

      if [ $rep -eq 0 ]
      then
        echo "  App Failed. Repeat" >> ${LOGFILE}
      fi
    done

  else

    pushd primeapp
    echo "#define PRIME_ITERATIONS ${4}" > params.h
    make
    popd
    
    cont=0
    while [ $cont -eq 0 ]
    do
      rep=0
      while [ $rep -eq 0 ]
      do
        pushd primeapp
        ./app
        rep=${?}
        cp ${FILEPATH}output.txt ${FILEPATH}${2}.txt
        popd

        if [ $rep -eq 0 ]
        then
          echo "  App Failed. Repeat" >> ${LOGFILE}
        fi
      done

      pushd primebot
      python3 main.py executionCheck ${FILEPATH}${2}.txt ${FILEPATH}${3}.txt
      cont=${?}
      popd
      if [ $cont -eq 0 ]
      then
        echo "  Repeat" >> ${LOGFILE}
        echo "  $(date)" >> ${LOGFILE}
      fi
    done
  fi
}

function filterByEVRate {
  echo "Filter -> ${3}" >> ${LOGFILE}
  echo "  $(date)" >> ${LOGFILE}
  
  pushd primebot
  python3 main.py filterByEVRate ${FILEPATH}${1}.txt ${FILEPATH}${2}.txt ${FILEPATH}${3}.txt ${4} ${5}
  len=$(cat ${FILEPATH}${3}.txt | wc -l)
  popd
  echo "  Len ${len}" >> ${LOGFILE}
}

function filterByEVRateAbsolute {
  echo "Filter -> ${3}" >> ${LOGFILE}
  echo "  $(date)" >> ${LOGFILE}
  
  pushd primebot
  python3 main.py filterByEVRateAbsolute ${FILEPATH}${1}.txt ${FILEPATH}${2}.txt ${FILEPATH}${3}.txt ${4} ${5}
  len=$(cat ${FILEPATH}${3}.txt | wc -l)
  popd
  echo "  Len ${len}" >> ${LOGFILE}
}

function filterByEVCRate {
  echo "Filter -> ${3}" >> ${LOGFILE}
  echo "  $(date)" >> ${LOGFILE}
  
  pushd primebot
  python3 main.py filterByEVCRate ${FILEPATH}${1}.txt ${FILEPATH}${2}.txt ${FILEPATH}${3}.txt ${4} ${5}
  len=$(cat ${FILEPATH}${3}.txt | wc -l)
  popd
  echo "  Len ${len}" >> ${LOGFILE}
}

function filterBySpeed {
  echo "Filter -> ${3}" >> ${LOGFILE}
  echo "  $(date)" >> ${LOGFILE}
  
  pushd primebot
  python3 main.py filterBySpeed ${FILEPATH}${1}.txt ${FILEPATH}${2}.txt ${FILEPATH}${3}.txt ${4}
  len=$(cat ${FILEPATH}${3}.txt | wc -l)
  popd
  echo "  Len ${len}" >> ${LOGFILE}
}

function sortByEVCr {
  echo "Results -> ${3}" >> ${LOGFILE}
  echo "  $(date)" >> ${LOGFILE}
  
  pushd primebot
  python3 main.py sortByEVCr ${FILEPATH}${1}.txt ${FILEPATH}${2}.txt ${FILEPATH}${3}.txt
  len=$(cat ${FILEPATH}${3}.txt | wc -l)
  popd
  echo "  Len ${len}" >> ${LOGFILE}
}

function sortByEVr {
  echo "Results -> ${3}" >> ${LOGFILE}
  echo "  $(date)" >> ${LOGFILE}
  
  pushd primebot
  python3 main.py sortByEVr ${FILEPATH}${1}.txt ${FILEPATH}${2}.txt ${FILEPATH}${3}.txt
  len=$(cat ${FILEPATH}${3}.txt | wc -l)
  popd
  echo "  Len ${len}" >> ${LOGFILE}
}

function mutateResults {
  pushd primebot
  python3 main.py mutation ${FILEPATH}${1}.txt ${FILEPATH}${2}.txt
  popd
}
