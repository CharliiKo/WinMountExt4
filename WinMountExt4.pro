QT       += core gui widgets

CONFIG += c++17

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/mainwindow.h

TRANSLATIONS += \
    translations/winmountext4_zh_CN.ts \
    translations/winmountext4_zh_TW.ts

RESOURCES += \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Add Shell32 for IsUserAnAdmin and ShellExecute
LIBS += -lshell32

DISTFILES += \
    assets/logo.ico
