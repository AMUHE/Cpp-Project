QT += core gui widgets network websockets sql texttospeech

CONFIG += c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = SmartAccessWelcome

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    apps/terminal/main.cpp \
    apps/terminal/main_window.cpp \
    modules/camera/stereo_camera.cpp \
    modules/vision/face_recognizer.cpp \
    modules/access-server/access_server.cpp \
    modules/access-control/access_policy.cpp \
    modules/config/app_config.cpp \
    modules/persistence/access_event_store.cpp \
    modules/device/door_controller.cpp \
    modules/speech/speech_announcer.cpp \
    modules/logging/json_logger.cpp

HEADERS += \
    apps/terminal/main_window.h \
    modules/camera/include/saw/camera/stereo_camera.h \
    modules/vision/include/saw/vision/face_recognizer.h \
    modules/access-server/include/saw/server/access_server.h \
    modules/access-control/include/saw/access/access_policy.h \
    modules/config/include/saw/config/app_config.h \
    modules/persistence/include/saw/persistence/access_event_store.h \
    modules/device/include/saw/device/door_controller.h \
    modules/speech/include/saw/speech/speech_announcer.h \
    modules/logging/include/saw/logging/json_logger.h

FORMS += \
    apps/terminal/main_window.ui

INCLUDEPATH += \
    modules/camera/include \
    modules/vision/include \
    modules/access-server/include \
    modules/access-control/include \
    modules/config/include \
    modules/persistence/include \
    modules/device/include \
    modules/speech/include \
    modules/logging/include

# Resolve OpenCV without committing a developer-machine path. Precedence:
# qmake argument -> config/opencv.local.pri -> environment variable.
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
    contains(QT_ARCH, x86_64) {
        error("The detected OpenCV build is 32-bit. Select Desktop Qt 5.14.2 MinGW 32-bit, or set OPENCV_ROOT to a 64-bit MinGW OpenCV build.")
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
        -lopencv_face345 \
        -lopencv_objdetect345 \
        -lopencv_videoio345 \
        -lopencv_imgcodecs345 \
        -lopencv_imgproc345 \
        -lopencv_core345

    # Make Qt Creator runs find the OpenCV DLLs without changing system PATH.
    QMAKE_POST_LINK += $$quote(cmd /c xcopy /D /Y "$$shell_path($$OPENCV_BIN_DIR)\\*.dll" "$$shell_path($$TARGET_BIN_DIR)\\" ^>nul) $$escape_expand(\n\t)
}

DESTDIR = $$OUT_PWD/bin
OBJECTS_DIR = $$OUT_PWD/obj
MOC_DIR = $$OUT_PWD/moc
RCC_DIR = $$OUT_PWD/rcc
UI_DIR = $$OUT_PWD/ui
