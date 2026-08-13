QT += core gui widgets network websockets

CONFIG += c++17
CONFIG -= app_bundle
win32-g++ {
    QMAKE_CFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
    QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
}
TEMPLATE = app
TARGET = CameraSimulationDemo

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    apps/camera-demo/main.cpp \
    apps/camera-demo/simulation_window.cpp \
    modules/camera/stereo_camera.cpp \
    modules/access-server/access_server.cpp \
    modules/device/door_controller.cpp

HEADERS += \
    apps/camera-demo/simulation_window.h \
    modules/camera/include/saw/camera/stereo_camera.h \
    modules/access-server/include/saw/server/access_server.h \
    modules/device/include/saw/device/door_controller.h

INCLUDEPATH += \
    apps/camera-demo \
    modules/camera/include \
    modules/access-server/include \
    modules/device/include

isEmpty(OPENCV_ROOT) {
    exists($$PWD/config/opencv.local.pri) {
        include($$PWD/config/opencv.local.pri)
    }
}
isEmpty(OPENCV_ROOT): OPENCV_ROOT = $$(OPENCV_ROOT)

win32 {
    isEmpty(OPENCV_ROOT) {
        error("OpenCV is not configured. Copy config/opencv.local.pri.example to config/opencv.local.pri and set OPENCV_ROOT, then run qmake again.")
    }

    OPENCV_INCLUDE = $$OPENCV_ROOT/include
    OPENCV_LIB_DIR = $$OPENCV_ROOT/x86/mingw/lib
    OPENCV_BIN_DIR = $$OPENCV_ROOT/x86/mingw/bin
    TARGET_BIN_DIR = $$absolute_path(bin, $$OUT_PWD)

    !exists($$OPENCV_INCLUDE/opencv2/core.hpp) {
        error("OpenCV headers were not found under OPENCV_ROOT: $$OPENCV_ROOT")
    }

    INCLUDEPATH += $$OPENCV_INCLUDE
    LIBS += -L$$OPENCV_LIB_DIR \
        -lopencv_videoio345 \
        -lopencv_imgproc345 \
        -lopencv_core345

    QMAKE_POST_LINK += $$quote(cmd /c xcopy /D /Y "$$shell_path($$OPENCV_BIN_DIR)\\*.dll" "$$shell_path($$TARGET_BIN_DIR)\\" ^>nul) $$escape_expand(\n\t)
}

DESTDIR = $$OUT_PWD/bin
OBJECTS_DIR = $$OUT_PWD/obj
MOC_DIR = $$OUT_PWD/moc
RCC_DIR = $$OUT_PWD/rcc
