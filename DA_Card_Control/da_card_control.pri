INCLUDEPATH += $$PWD \
               $$PWD/lib/

contains(DEFINES, WIN64) {
    LIBS += -L$$PWD/lib/x64/ -lUSB3000
} else {
    LIBS += -L$$PWD/lib/x86/ -lUSB3000
}

SOURCES += \
    $$PWD/dacardcontrol.cpp

HEADERS += \
    $$PWD/dacardcontrol.h

