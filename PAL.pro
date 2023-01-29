QT += core gui
QT += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



SOURCES += \
	src/dialogs/addascent.cpp \
	src/main/main.cpp \
	src/main/mainwindow.cpp

HEADERS += \
	src/db/initdb.h \
	src/dialogs/addascent.h \
	src/main/mainwindow.h

FORMS += \
	src/ui/addascent.ui \
	src/ui/mainwindow.ui



TRANSLATIONS += \
	src/translation/PAL_de.ts
	
DISTFILES +=
	src/translation/PAL_de.ts



CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
