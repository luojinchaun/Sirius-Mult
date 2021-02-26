QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11


CONFIG += force_debug_info

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    blog.cpp \
    btablewidget.cpp \
    dialog/camera_aim_dialog.cpp \
    dialog/camera_calibration_dialog.cpp \
    dialog/camera_setting_dialog.cpp \
    dialog/customer_engineer_dialog.cpp \
    dialog/our_engineer_dialog.cpp \
    dialog/shifter_calibration_dialog.cpp \
    dialog/shifter_setting_dialog.cpp \
    dialog/unit_setting_dialog.cpp \
    globalfun.cpp \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp

HEADERS += \
    blog.h \
    btablewidget.h \
    classlib/include/algorithmMain.h \
    classlib/include/algorithmMain.h \
    classlib/include/baseFunc.h \
    classlib/include/baseFunc.h \
    classlib/include/edgedetector.h \
    classlib/include/edgedetector.h \
    classlib/include/filter.h \
    classlib/include/filter.h \
    classlib/include/psf.h \
    classlib/include/psf.h \
    classlib/include/psi.h \
    classlib/include/psi.h \
    classlib/include/pztcalibrate.h \
    classlib/include/pztcalibrate.h \
    classlib/include/result.h \
    classlib/include/result.h \
    classlib/include/splice.h \
    classlib/include/unwrap.h \
    classlib/include/unwrap.h \
    classlib/include/zernike.h \
    classlib/include/zernike.h \
    dialog/camera_aim_dialog.h \
    dialog/camera_calibration_dialog.h \
    dialog/camera_setting_dialog.h \
    dialog/customer_engineer_dialog.h \
    dialog/our_engineer_dialog.h \
    dialog/shifter_calibration_dialog.h \
    dialog/shifter_setting_dialog.h \
    dialog/unit_setting_dialog.h \
    globalfun.h \
    mainwindow.h \
    qcustomplot.h

FORMS += \
    dialog/camera_aim_dialog.ui \
    dialog/camera_calibration_dialog.ui \
    dialog/camera_setting_dialog.ui \
    dialog/customer_engineer_dialog.ui \
    dialog/our_engineer_dialog.ui \
    dialog/shifter_calibration_dialog.ui \
    dialog/shifter_setting_dialog.ui \
    dialog/unit_setting_dialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#******************************************************************************************************************

TARGET = Sirius-Mult

DESTDIR = ./

CONFIG += debug_and_release
QT += axcontainer

QT += charts printsupport

RC_ICONS = Image_mask_recognition.ico

LIBS += -luser32

include("../Draw_Figure/draw_figure.pri")
include("../Camera/camera.pri")
include("../DA_Card_Control/da_card_control.pri")
include("../Registry_control/registry_control.pri")

RESOURCES += \
    image.qrc \
    source.qrc

INCLUDEPATH += $$PWD \
INCLUDEPATH += $$PWD/classlib/include/

CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/classlib/win64/debug/ -lAlgorithmLib_64
} else {
    LIBS += -L$$PWD/classlib/win64/release/ -lAlgorithmLib_64
}
