#! /bin/bash

valgrind --help >& /dev/null
if [ $? -ne 0 ]; then
  echo "WARNING!"
  echo "valgrind not found!"
  echo "Skipping valgrind test of testdump"
  exit 0
else
  if test -x ../dump/.libs/lt-h5dumpserie; then
    valgrind --error-exitcode=200 --num-callers=150 --leak-check=full --show-reachable=yes ../dump/.libs/lt-h5lsserie -d -l test2d.h5 || exit
    valgrind --error-exitcode=200 --num-callers=150 --leak-check=full --show-reachable=yes ../dump/.libs/lt-h5dumpserie test2d.h5/timeserie || exit
  elif test -x ../dump/h5lsserie; then
    valgrind --error-exitcode=200 --num-callers=150 --leak-check=full --show-reachable=yes ../dump/h5lsserie -d -l test2d.h5 || exit
    valgrind --error-exitcode=200 --num-callers=150 --leak-check=full --show-reachable=yes ../dump/h5dumpserie test2d.h5/timeserie || exit
  elif test -x ../dump/.libs/h5lsserie; then
    valgrind --error-exitcode=200 --num-callers=150 --leak-check=full --show-reachable=yes ../dump/.libs/h5lsserie -d -l test2d.h5 || exit
    valgrind --error-exitcode=200 --num-callers=150 --leak-check=full --show-reachable=yes ../dump/.libs/h5dumpserie test2d.h5/timeserie || exit
  else
    echo "Unknown install/build stage"
    exit 1
  fi
fi
