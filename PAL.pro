QT += core gui
QT += sql
QT += svg svgwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000	# disables all the APIs deprecated before Qt 6.0.0



SOURCES += \
	src/data/ascent.cpp \
	src/data/country.cpp \
	src/data/hiker.cpp \
	src/data/item_id.cpp \
	src/data/names.cpp \
	src/data/peak.cpp \
	src/data/photo.cpp \
	src/data/range.cpp \
	src/data/region.cpp \
	src/data/trip.cpp \
	src/db/associative_table.cpp \
	src/db/column.cpp \
	src/db/database.cpp \
	src/db/db_error.cpp \
	src/db/normal_table.cpp \
	src/db/table.cpp \
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
	src/dialogs/list/hiker_list.cpp \
	src/dialogs/list/photo_list.cpp \
	src/dialogs/new_or_edit_dialog.cpp \
	src/dialogs/parse_helper.cpp \
	src/dialogs/range_dialog.cpp \
	src/dialogs/ascent_dialog.cpp \
	src/dialogs/peak_dialog.cpp \
	src/dialogs/region_dialog.cpp \
	src/dialogs/trip_dialog.cpp \
	src/main/about_window.cpp \
	src/main/main.cpp \
	src/main/main_window.cpp

HEADERS += \
	src/data/ascent.h \
	src/data/country.h \
	src/data/hiker.h \
	src/data/item_id.h \
	src/data/peak.h \
	src/data/photo.h \
	src/data/range.h \
	src/data/region.h \
	src/data/trip.h \
	src/db/associative_table.h \
	src/db/column.h \
	src/db/database.h \
	src/db/db_error.h \
	src/db/normal_table.h \
	src/db/table.h \
	src/db/test_db.h \
	src/db/tables/ascents_table.h \
	src/db/tables/countries_table.h \
	src/db/tables/hikers_table.h \
	src/db/tables/participated_table.h \
	src/db/tables/peaks_table.h \
	src/db/tables/photos_table.h \
	src/db/tables/ranges_table.h \
	src/db/tables/regions_table.h \
	src/db/tables/trips_table.h \
	src/dialogs/add_hiker_dialog.h \
	src/dialogs/ascent_dialog.h \
	src/dialogs/country_dialog.h \
	src/dialogs/hiker_dialog.h \
	src/dialogs/list/hiker_list.h \
	src/dialogs/list/photo_list.h \
	src/dialogs/new_or_edit_dialog.h \
	src/dialogs/peak_dialog.h \
	src/dialogs/range_dialog.h \
	src/dialogs/region_dialog.h \
	src/dialogs/trip_dialog.h \
	src/dialogs/parse_helper.h \
	src/main/about_window.h \
	src/main/main_window.h \
	src/main/settings.h

FORMS += \
	src/ui/about_window.ui \
	src/ui/main_window.ui \
	src/ui/add_hiker_dialog.ui \
	src/ui/ascent_dialog.ui \
	src/ui/country_dialog.ui \
	src/ui/hiker_dialog.ui \
	src/ui/peak_dialog.ui \
	src/ui/range_dialog.ui \
	src/ui/region_dialog.ui \
	src/ui/trip_dialog.ui



DEFINES += APP_VERSION='"\\\"0.1\\\""'
DEFINES += APP_COPYRIGHT='"\\\"2023 Simon Vetter\\\""'
DEFINES += CODE_LINK='"\\\"https://github.com/svetter/pal\\\""'



TRANSLATIONS += \
	translation/PAL_de.ts

DISTFILES += \
	translation/PAL_de.ts



RC_ICONS = resources/logo/logo_multisize_square.ico



resources.files = \
	resources/logo/logo_path.svg \
	resources/logo/logo_128.png \
	resources/logo/logo_multisize_square.ico \
	resources/logo/gpl-v3-logo.svg
resources.base = resources
resources.prefix = /

RESOURCES = resources



CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
