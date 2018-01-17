if COND_WIN32
else
  MAYBE_VALGRIND_TESTS = valgrindtestlib.sh valgrindtestdump.sh
endif

check_PROGRAMS = testlib

TEST_EXTENSIONS=.sh
TESTS = testlib.sh testdump.sh $(MAYBE_VALGRIND_TESTS)
testdump.log: testlib.log
valgrindtestlib.log: testlib.log
valgrindtestdump.log: valgrindtestlib.log

testlib_SOURCES = testlib.cc

testlib_CPPFLAGS = -I$(top_srcdir) $(FMATVEC_CFLAGS)
testlib_LDFLAGS = -L..
testlib_LDADD = ../libhdf5serie.la -l@BOOST_SYSTEM_LIB@
