
TARGET = StreamParser
TEMPLATE = lib

CONFIG += staticlib

SOURCES += ast.cpp \
           objectnode.cpp \
           platformnode.cpp \
           streamnode.cpp \
    valuenode.cpp \
    bundlenode.cpp \
    propertynode.cpp \
    namenode.cpp \
    functionnode.cpp

HEADERS += ast.h \
           objectnode.h \
           platformnode.h \
           streamnode.h \
    valuenode.h \
    bundlenode.h \
    propertynode.h \
    namenode.h \
    functionnode.h

BISONSOURCES = lang_stream.y
FLEXSOURCES = lang_stream.l

include(../config.pri)
include(parser.pri)

win32 {
LIBS += -L${FLEX_LIB_PATH}
}
LIBS += -lfl