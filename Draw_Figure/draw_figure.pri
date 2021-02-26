INCLUDEPATH += $$PWD \
               $$PWD/OpenCV_msvc2017_64/include

SOURCES += \
    $$PWD/bqgraphicsitem.cpp \
    $$PWD/bqgraphicsscene.cpp \
    $$PWD/bqgraphicsview.cpp \

HEADERS += \
    $$PWD/bqgraphicsitem.h \
    $$PWD/bqgraphicsscene.h \
    $$PWD/bqgraphicsview.h \

windows {
    CONFIG(debug, debug|release) {
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_calib3d420d
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_core420d
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_dnn420d
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_features2d420d
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_flann420d
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_gapi420d
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_highgui420d
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_imgcodecs420d
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_imgproc420d
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_ml420d
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_objdetect420d
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_photo420d
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_stitching420d
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_video420d
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_videoio420d
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_world420d
    } else {
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_calib3d420
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_core420
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_dnn420
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_features2d420
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_flann420
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_gapi420
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_highgui420
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_imgcodecs420
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_imgproc420
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_ml420
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_objdetect420
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_photo420
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_stitching420
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_video420
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_videoio420
        LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_world420
    }
}
