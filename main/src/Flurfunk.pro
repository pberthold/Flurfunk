QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Core.cpp \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    Core.h \
    MainWindow.h

FORMS += \
    MainWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Unix 64bit
unix {
    BASS_DIR = $$PWD/../lib/bass/2.4.16/unix_x64
    INCLUDEPATH += $$BASS_DIR/
    LIBS += -L$$$$BASS_DIR -lbass
    DEFINES += "CONFIG_DIR_TYPE=portable"
}

# Windows 64bit
win32:!contains(QT_ARCH, i386) {
    BASS_DIR = $$PWD/../lib/bass/2.4.16/win_x64
    INCLUDEPATH += $$BASS_DIR/
    LIBS += -L$$$$BASS_DIR -lbass
    DEFINES += "CONFIG_DIR=\"QCoreApplication::applicationDirPath()+\\\"/settings.ini\\\"\""
}

# Android armeabi-v7a
android {
    BASS_DIR = $$PWD/../lib/bass/2.4.16/android_armeabi-v7a
    INCLUDEPATH += $$BASS_DIR/
    LIBS += -L$$$$BASS_DIR -lbass
    DEFINES += "CONFIG_DIR=\"QCoreApplication::applicationDirPath()+\\\"/settings.ini\\\"\""

    QT += androidextras
    ANDROID_EXTRA_LIBS += $$BASS_DIR/libbass.so
    DEFINES += "ANDROID=1"

    DISTFILES += \
        android/AndroidManifest.xml \
        android/build.gradle \
        android/res/values/libs.xml

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
}

RC_ICONS = $$PWD/ressource/logo.ico
