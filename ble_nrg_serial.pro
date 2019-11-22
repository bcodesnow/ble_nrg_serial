QT += quick quickwidgets qml bluetooth quickcontrols2 network networkauth multimedia
CONFIG += c++11
QMAKE_CXXFLAGS_WARN_ON += -Wno-reorder
#QMAKE_CXXFLAGS += "-Wno-old-style-cast"

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS


# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    adapterinfo.cpp \
    devicecontroller.cpp \
    devicefinder.cpp \
    bluetoothbaseclass.cpp \
    devicehelper.cpp \
    deviceinfo.cpp \
    deviceinterface.cpp \
    graphpainter.cpp \
    linuxterminalinterface.cpp \
    networkmanager.cpp \
    paintdata.cpp \
    qmllistadapter.cpp \
    terminaltoqmlb.cpp \
    connectionhandler.cpp \
    logfilehandler.cpp \
    catchcontroller.cpp \
    timesynchandler.cpp

RESOURCES += qml.qrc \
    common.qrc \
    images.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


HEADERS += \
    adapterinfo.h \
    devicecontroller.h \
    devicefinder.h \
    bluetoothbaseclass.h \
    deviceinfo.h \
    deviceinterface.h \
    graphpainter.h \
    linuxterminalinterface.h \
    mci_catch_detection.h \
    networkmanager.h \
    paintdata.h \
    qmllistadapter.h \
    terminaltoqmlb.h \
    connectionhandler.h \
    ble_uart.h \
    logfilehandler.h \
    catchcontroller.h \
    ble_uart_types.h \
    timesynchandler.h

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/res/values/libs.xml \
    preTrash

contains(ANDROID_TARGET_ARCH,) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}

versionAtLeast(QT_VERSION, "5.14.0") {
    ANDROID_EXTRA_LIBS += \
        $$PWD/arm/libcrypto_1_1.so \
        $$PWD/arm/libssl_1_1.so \
        $$PWD/arm64/libcrypto_1_1.so \
        $$PWD/arm64/libssl_1_1.so \
        $$PWD/x86/libcrypto_1_1.so \
        $$PWD/x86/libssl_1_1.so \
        $$PWD/x86_64/libcrypto_1_1.so \
        $$PWD/x86_64/libssl_1_1.so
}
else {
    equals(ANDROID_TARGET_ARCH,armeabi-v7a) {
        ANDROID_EXTRA_LIBS += \
            $$PWD/arm/libcrypto_1_1.so \
            $$PWD/arm/libssl_1_1.so
    }

    equals(ANDROID_TARGET_ARCH,arm64-v8a) {
        ANDROID_EXTRA_LIBS += \
            $$PWD/arm64/libcrypto_1_1.so \
            $$PWD/arm64/libssl_1_1.so
    }

    equals(ANDROID_TARGET_ARCH,x86) {
        ANDROID_EXTRA_LIBS += \
            $$PWD/x86/libcrypto_1_1.so \
            $$PWD/x86/libssl_1_1.so
    }

    equals(ANDROID_TARGET_ARCH,x86_64) {
        ANDROID_EXTRA_LIBS += \
            $$PWD/x86_64/libcrypto_1_1.so \
            $$PWD/x86_64/libssl_1_1.so
    }
}
