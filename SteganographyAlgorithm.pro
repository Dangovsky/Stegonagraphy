QT += core
QT -= gui

CONFIG += c++11

TARGET = SteganographyAlgorithm
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    steganography.cpp

HEADERS += \
    steganography.h

INCLUDEPATH += D:\opencv-build\install\include

LIBS += -LD:\\opencv-build\\bin \
    libopencv_core340 \
    libopencv_highgui340 \
    libopencv_imgproc340 \
    libopencv_imgcodecs340 \
    libopencv_features2d340 \
    libopencv_calib3d340 \

DISTFILES += \
    test.tif \
    test.jpg
