QT += core gui
QT += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



SOURCES += \
	src/dialogs/new_range.cpp \
	src/dialogs/new_ascent.cpp \
	src/dialogs/new_peak.cpp \
	src/dialogs/new_trip.cpp \
	src/main/main.cpp \
	src/main/main_window.cpp

HEADERS += \
	src/db/initdb.h \
	src/dialogs/new_ascent.h \
	src/dialogs/new_peak.h \
	src/dialogs/new_range.h \
	src/dialogs/new_trip.h \
	src/main/main_window.h

FORMS += \
	src/ui/main_window.ui \
	src/ui/new_ascent.ui \
	src/ui/new_peak.ui \
	src/ui/new_range.ui \
	src/ui/new_trip.ui



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
