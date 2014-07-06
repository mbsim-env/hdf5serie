if COND_WIN32
else
  MAYBE_VALGRIND_TESTS = valgrindtestlib.sh valgrindtestdump.sh
endif

check_PROGRAMS = testlib
check_SCRIPTS = testdump.sh $(MAYBE_VALGRIND_TESTS)

TESTS = testlib testdump.sh $(MAYBE_VALGRIND_TESTS)

testlib_SOURCES = testlib.cc

testlib_CPPFLAGS = -I..
testlib_LDFLAGS = -L..
testlib_LDADD = ../libhdf5serie.la
