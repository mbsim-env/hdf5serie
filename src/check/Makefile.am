check_PROGRAMS = testlib
check_SCRIPTS = valgrindtestlib.sh \
  testdump.sh \
  valgrindtestdump.sh

TESTS = testlib \
  valgrindtestlib.sh \
  testdump.sh \
  valgrindtestdump.sh

testlib_SOURCES = testlib.cc

testlib_CPPFLAGS = -I..
testlib_LDFLAGS = -L..
testlib_LDADD = -lhdf5serie 
