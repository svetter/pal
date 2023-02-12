QT += core gui
QT += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



SOURCES += \
	src/data/ascent.cpp \
	src/data/country.cpp \
	src/data/hiker.cpp \
	src/data/names.cpp \
	src/data/peak.cpp \
	src/data/range.cpp \
	src/data/region.cpp \
	src/data/trip.cpp \
	src/db/db_error.cpp \
	src/db/db_interface.cpp \
	src/db/db_model.cpp \
	src/db/tables/ascents_table.cpp \
	src/db/tables/countries_table.cpp \
	src/db/tables/hikers_table.cpp \
	src/db/tables/participated_table.cpp \
	src/db/tables/peaks_table.cpp \
	src/db/tables/photos_table.cpp \
	src/db/tables/ranges_table.cpp \
	src/db/tables/regions_table.cpp \
	src/db/tables/trips_table.cpp \
	src/dialogs/add_hiker_dialog.cpp \
	src/dialogs/country_dialog.cpp \
	src/dialogs/hiker_dialog.cpp \
	src/dialogs/new_or_edit_dialog.cpp \
	src/dialogs/parse_helper.cpp \
	src/dialogs/range_dialog.cpp \
	src/dialogs/ascent_dialog.cpp \
	src/dialogs/peak_dialog.cpp \
	src/dialogs/region_dialog.cpp \
	src/dialogs/trip_dialog.cpp \
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
	src/db/db_error.h \
	src/db/db_interface.h \
	src/db/db_model.h \
	src/db/test_db.h \
	src/db/tables/ascents_table.h \
	src/db/tables/countries_table.h \
	src/db/tables/hikers_table.h \
	src/db/tables/participated_table.h \
	src/db/tables/peaks_table.h \
	src/db/tables/photos_table.h \
	src/db/tables/ranges_table.h \
	src/db/tables/regionss_table.h \
	src/db/tables/trips_table.h \
	src/dialogs/add_hiker_dialog.h \
	src/dialogs/ascent_dialog.h \
	src/dialogs/country_dialog.h \
	src/dialogs/hiker_dialog.h \
	src/dialogs/new_or_edit_dialog.h \
	src/dialogs/peak_dialog.h \
	src/dialogs/range_dialog.h \
	src/dialogs/region_dialog.h \
	src/dialogs/trip_dialog.h \
	src/dialogs/parse_helper.h \
	src/main/main_window.h

FORMS += \
	src/ui/main_window.ui \
	src/ui/add_hiker_dialog.ui \
	src/ui/ascent_dialog.ui \
	src/ui/country_dialog.ui \
	src/ui/hiker_dialog.ui \
	src/ui/peak_dialog.ui \
	src/ui/range_dialog.ui \
	src/ui/region_dialog.ui \
	src/ui/trip_dialog.ui



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
