noinst_PROGRAMS = read write

read_SOURCES = read.cc

write_SOURCES = write.cc

read_CPPFLAGS = -I../.. -I..
read_LDFLAGS = -L../.. -L..
read_LDADD = -lhdf5serie -lhdf5rereadmetadata

write_CPPFLAGS = -I../..
write_LDFLAGS = -L../..
write_LDADD = -lhdf5serie
