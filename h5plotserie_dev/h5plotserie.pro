HEADERS     = mainwindow.h \
              dataselection.h \
              curves.h \
              plotarea.h \
              plotdata.h \
              treewidgetitem.h
SOURCES     = mainwindow.cpp \
              dataselection.cpp \
              curves.cpp \
              plotarea.cpp \
              main.cpp

QT += xml

LIBS += -L<<qwt-5.2.1>>/lib -lqwt
INCLUDEPATH += <<qwt-5.2.1>>/include
CONFIG += link_pkgconfig
PKGCONFIG += <<pkgconfig>>/hdf5serie.pc
