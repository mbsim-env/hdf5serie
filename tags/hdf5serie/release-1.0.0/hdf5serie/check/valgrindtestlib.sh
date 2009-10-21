#! /bin/sh

valgrind --help >& /dev/null
if [ $? -ne 0 ]; then
  echo "WARNING!"
  echo "valgrind not found!"
  echo "Skipping valgrind test of testlib"
  exit 0
else
  if test -x .libs/lt-testlib; then
    valgrind --error-exitcode=1 .libs/lt-testlib
  else
    valgrind --error-exitcode=1 testlib
  fi
fi
