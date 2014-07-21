#! /bin/sh

valgrind --help >& /dev/null
if [ $? -ne 0 ]; then
  echo "WARNING!"
  echo "valgrind not found!"
  echo "Skipping valgrind test of testdump"
  exit 0
else
  if test -x ../dump/.libs/lt-h5dumpserie; then
#    valgrind ---error-exitcode=200 --num-callers=150 --leak-check=full --show-reachable=yes ../dump/.libs/lt-h5lsserie testcompound.h5/serie1d || exit
#    valgrind --error-exitcode=200 --num-callers=150 --leak-check=full --show-reachable=yes ../dump/.libs/lt-h5dumpserie testcompound.h5/serie1d || exit
    valgrind --error-exitcode=200 --num-callers=150 --leak-check=full --show-reachable=yes ../dump/.libs/lt-h5lsserie test2d.h5 || exit
    valgrind --error-exitcode=200 --num-callers=150 --leak-check=full --show-reachable=yes ../dump/.libs/lt-h5dumpserie test2d.h5/timeserie || exit
  else
#    valgrind --error-exitcode=200 --num-callers=150 --leak-check=full --show-reachable=yes ../dump/h5lsserie testcompound.h5/serie1d || exit
#    valgrind --error-exitcode=200 --num-callers=150 --leak-check=full --show-reachable=yes ../dump/h5dumpserie testcompound.h5/serie1d || exit
    valgrind --error-exitcode=200 --num-callers=150 --leak-check=full --show-reachable=yes ../dump/h5lsserie test2d.h5 || exit
    valgrind --error-exitcode=200 --num-callers=150 --leak-check=full --show-reachable=yes ../dump/h5dumpserie test2d.h5/timeserie || exit
  fi
fi
