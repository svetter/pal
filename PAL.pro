QT += core gui
QT += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



SOURCES += \
	src/db/db_model.cpp \
	src/dialogs/new_country.cpp \
	src/dialogs/new_hiker.cpp \
	src/dialogs/new_range.cpp \
	src/dialogs/new_ascent.cpp \
	src/dialogs/new_peak.cpp \
	src/dialogs/new_region.cpp \
	src/dialogs/new_trip.cpp \
	src/main/main.cpp \
	src/main/main_window.cpp

HEADERS += \
	src/data/ascent.h \
	src/data/country.h \
	src/data/hiker.h \
	src/data/peak.h \
	src/data/range.h \
	src/data/region.h \
	src/data/trip.h \
	src/db/db_interface.h \
	src/db/db_model.h \
	src/db/tables/db_tables.h \
	src/dialogs/new_ascent.h \
	src/dialogs/new_country.h \
	src/dialogs/new_hiker.h \
	src/dialogs/new_peak.h \
	src/dialogs/new_range.h \
	src/dialogs/new_region.h \
	src/dialogs/new_trip.h \
	src/main/main_window.h

FORMS += \
	src/ui/main_window.ui \
	src/ui/new_ascent.ui \
	src/ui/new_country.ui \
	src/ui/new_hiker.ui \
	src/ui/new_peak.ui \
	src/ui/new_range.ui \
	src/ui/new_region.ui \
	src/ui/new_trip.ui



TRANSLATIONS += \
	translation/PAL_de.ts
	
DISTFILES +=
	translation/PAL_de.ts
	


RC_ICONS += \
	resources/logo_multisize.ico



CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
