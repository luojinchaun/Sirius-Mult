INCLUDEPATH += $$PWD \
               $$PWD/IC_Imaging_Control/ \
               $$PWD/Pylon_Camera_Control \
               $$PWD/MV_Camera_Control \
               $$PWD/Virtual_Camera_Control

#******************************************************************************************************************

INCLUDEPATH += $$PWD/IC_Imaging_Control/classlib/include/

CONFIG(debug, debug|release) {
    contains(DEFINES, WIN64) {
        LIBS += -L$$PWD/IC_Imaging_Control/classlib/x64/debug/ -lTIS_UDSHL11d_x64
    } else {
        LIBS += -L$$PWD/IC_Imaging_Control/classlib/win32/debug/ -lTIS_UDSHL11d
    }
} else {
    contains(DEFINES, WIN64) {
        LIBS += -L$$PWD/IC_Imaging_Control/classlib/x64/release/ -lTIS_UDSHL11_x64
    } else {
        LIBS += -L$$PWD/IC_Imaging_Control/classlib/win32/release/ -lTIS_UDSHL11
    }
}

#******************************************************************************************************************

INCLUDEPATH += $$PWD/Pylon_Camera_Control/include/ \
               $$PWD/Pylon_Camera_Control/include/pylon

contains(DEFINES, WIN64) {
    LIBS += -L$$PWD/Pylon_Camera_Control/lib/x64/ -lGCBase_MD_VC141_v3_1_Basler_pylon
    LIBS += -L$$PWD/Pylon_Camera_Control/lib/x64/ -lGenApi_MD_VC141_v3_1_Basler_pylon
    LIBS += -L$$PWD/Pylon_Camera_Control/lib/x64/ -lPylonBase_v6_1
    LIBS += -L$$PWD/Pylon_Camera_Control/lib/x64/ -lPylonC_v6.1
    LIBS += -L$$PWD/Pylon_Camera_Control/lib/x64/ -lPylonGUI_v6_1
    LIBS += -L$$PWD/Pylon_Camera_Control/lib/x64/ -lPylonUtility_v6_1
} else {
    LIBS += -L$$PWD/Pylon_Camera_Control/lib/Win32/ -lGCBase_MD_VC141_v3_1_Basler_pylon
    LIBS += -L$$PWD/Pylon_Camera_Control/lib/Win32/ -lGenApi_MD_VC141_v3_1_Basler_pylon
    LIBS += -L$$PWD/Pylon_Camera_Control/lib/Win32/ -lPylonBase_v6_1
    LIBS += -L$$PWD/Pylon_Camera_Control/lib/Win32/ -lPylonC_v6.1
    LIBS += -L$$PWD/Pylon_Camera_Control/lib/Win32/ -lPylonGUI_v6_1
    LIBS += -L$$PWD/Pylon_Camera_Control/lib/Win32/ -lPylonUtility_v6_1
}

#******************************************************************************************************************

INCLUDEPATH += $$PWD/MV_Camera_Control/include/ \

contains(DEFINES, WIN64) {
    LIBS += -L$$PWD/MV_Camera_Control/lib/win64/ -lMvCameraControl
} else {
    LIBS += -L$$PWD/MV_Camera_Control/lib/Win32/ -lMvCameraControl
}

#******************************************************************************************************************

SOURCES += \
    $$PWD/IC_Imaging_Control/biccameracontrol.cpp \
    $$PWD/MV_Camera_Control/bmvcameracontrol.cpp \
    $$PWD/Pylon_Camera_Control/bbaslercamercontrol.cpp \
    $$PWD/Virtual_Camera_Control/bvrcameracontrol.cpp

HEADERS += \
    $$PWD/IC_Imaging_Control/biccameracontrol.h \
    $$PWD/MV_Camera_Control/bmvcameracontrol.h \
    $$PWD/Pylon_Camera_Control/bbaslercamercontrol.h \
    $$PWD/Virtual_Camera_Control/bvrcameracontrol.h \
    $$PWD/camera.h
