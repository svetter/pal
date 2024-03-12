# Copyright 2023-2024 Simon Vetter
# 
# This file is part of PeakAscentLogger.
# 
# PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
# of the GNU General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
# 
# PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along with PeakAscentLogger.
# If not, see <https://www.gnu.org/licenses/>.



VERSION = 1.3.0
DEFINES += APP_VERSION_MAJOR=1
DEFINES += APP_VERSION_MINOR=3
DEFINES += APP_VERSION_PATCH=0

DEFINES += APP_COPYRIGHT='"\\\"2023-2024 Simon Vetter\\\""'
DEFINES += CODE_LINK='"\\\"https://github.com/svetter/pal\\\""'



QT += core gui
QT += sql
QT += svg svgwidgets
QT += charts

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000	# disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17
CONFIG += lrelease embed_translations



test_conf {
	TARGET = PAL_test
	QT += testlib
	CONFIG += qt warn_on depend_includepath testcase

	SOURCES += \
		src/test/startup_test.cpp
} else {
	SOURCES += src/main/main.cpp
}



HEADERS += \
	src/comp_tables/breadcrumbs.h \
	src/comp_tables/comp_ascents_table.h \
	src/comp_tables/comp_countries_table.h \
	src/comp_tables/comp_hikers_table.h \
	src/comp_tables/comp_peaks_table.h \
	src/comp_tables/comp_ranges_table.h \
	src/comp_tables/comp_regions_table.h \
	src/comp_tables/comp_table_listener.h \
	src/comp_tables/comp_trips_table.h \
	src/comp_tables/composite_column.h \
	src/comp_tables/composite_table.h \
	src/comp_tables/filter.h \
	src/comp_tables/fold_composite_column.h \
	src/data/ascent.h \
	src/data/country.h \
	src/data/enum_names.h \
	src/data/hiker.h \
	src/data/item_id.h \
	src/data/item_types.h \
	src/data/peak.h \
	src/data/photo.h \
	src/data/range.h \
	src/data/region.h \
	src/data/trip.h \
	src/db/associative_table.h \
	src/db/column.h \
	src/db/database.h \
	src/db/db_error.h \
	src/db/db_upgrade.h \
	src/db/table_listener.h \
	src/db/normal_table.h \
	src/db/row_index.h \
	src/db/table.h \
	src/db/table_buffer.h \
	src/db/tables/ascents_table.h \
	src/db/tables/countries_table.h \
	src/db/tables/hikers_table.h \
	src/db/tables/participated_table.h \
	src/db/tables/peaks_table.h \
	src/db/tables/photos_table.h \
	src/db/tables/ranges_table.h \
	src/db/tables/regions_table.h \
	src/db/tables/settings_table.h \
	src/db/tables/trips_table.h \
	src/dialogs/add_hiker_dialog.h \
	src/dialogs/ascent_dialog.h \
	src/dialogs/country_dialog.h \
	src/dialogs/hiker_dialog.h \
	src/dialogs/item_dialog.h \
	src/dialogs/list/hiker_list.h \
	src/dialogs/list/photo_list.h \
	src/dialogs/peak_dialog.h \
	src/dialogs/range_dialog.h \
	src/dialogs/region_dialog.h \
	src/dialogs/trip_dialog.h \
	src/dialogs/parse_helper.h \
	src/main/about_window.h \
	src/main/ascent_filter_bar.h \
	src/main/helpers.h \
	src/main/item_types_handler.h \
	src/main/main_window.h \
	src/settings/project_settings.h \
	src/settings/project_settings_window.h \
	src/settings/settings.h \
	src/settings/settings_window.h \
	src/stats/chart.h \
	src/stats/stats_engine.h \
	src/stats/stats_listeners.h \
	src/tools/export_decls.h \
	src/tools/export_dialog.h \
	src/tools/export_thread.h \
	src/tools/export_writer.h \
	src/tools/photo_relocation_thread.h \
	src/tools/relocate_photos_dialog.h \
	src/viewer/ascent_viewer.h \
	src/viewer/file_drop_frame.h \
	src/viewer/icon_group_box.h \
	src/viewer/scalable_image_label.h

SOURCES += \
	src/comp_tables/breadcrumbs.cpp \
	src/comp_tables/comp_table_listener.cpp \
	src/comp_tables/composite_column.cpp \
	src/comp_tables/composite_table.cpp \
	src/comp_tables/fold_composite_column.cpp \
	src/data/ascent.cpp \
	src/data/country.cpp \
	src/data/enum_names.cpp \
	src/data/hiker.cpp \
	src/data/item_id.cpp \
	src/data/peak.cpp \
	src/data/photo.cpp \
	src/data/range.cpp \
	src/data/region.cpp \
	src/data/trip.cpp \
	src/db/associative_table.cpp \
	src/db/column.cpp \
	src/db/database.cpp \
	src/db/db_error.cpp \
	src/db/db_upgrade.cpp \
	src/db/normal_table.cpp \
	src/db/row_index.cpp \
	src/db/table.cpp \
	src/db/table_buffer.cpp \
	src/db/tables/ascents_table.cpp \
	src/db/tables/countries_table.cpp \
	src/db/tables/hikers_table.cpp \
	src/db/tables/participated_table.cpp \
	src/db/tables/peaks_table.cpp \
	src/db/tables/photos_table.cpp \
	src/db/tables/ranges_table.cpp \
	src/db/tables/regions_table.cpp \
	src/db/tables/settings_table.cpp \
	src/db/tables/trips_table.cpp \
	src/dialogs/add_hiker_dialog.cpp \
	src/dialogs/country_dialog.cpp \
	src/dialogs/hiker_dialog.cpp \
	src/dialogs/item_dialog.cpp \
	src/dialogs/list/hiker_list.cpp \
	src/dialogs/list/photo_list.cpp \
	src/dialogs/parse_helper.cpp \
	src/dialogs/range_dialog.cpp \
	src/dialogs/ascent_dialog.cpp \
	src/dialogs/peak_dialog.cpp \
	src/dialogs/region_dialog.cpp \
	src/dialogs/trip_dialog.cpp \
	src/main/about_window.cpp \
	src/main/ascent_filter_bar.cpp \
	src/main/helpers.cpp \
	src/main/main_window.cpp \
	src/settings/project_settings.cpp \
	src/settings/project_settings_window.cpp \
	src/settings/settings.cpp \
	src/settings/settings_window.cpp \
	src/stats/chart.cpp \
	src/stats/stats_engine.cpp \
	src/stats/stats_listeners.cpp \
	src/tools/export_dialog.cpp \
	src/tools/export_thread.cpp \
	src/tools/export_writer.cpp \
	src/tools/photo_relocation_thread.cpp \
	src/tools/relocate_photos_dialog.cpp \
	src/viewer/ascent_viewer.cpp \
	src/viewer/scalable_image_label.cpp

FORMS += \
	src/ui/about_window.ui \
	src/ui/ascent_filter_bar.ui \
	src/ui/ascent_viewer.ui \
	src/ui/export_dialog.ui \
	src/ui/main_window.ui \
	src/ui/add_hiker_dialog.ui \
	src/ui/ascent_dialog.ui \
	src/ui/country_dialog.ui \
	src/ui/hiker_dialog.ui \
	src/ui/peak_dialog.ui \
	src/ui/project_settings_window.ui \
	src/ui/range_dialog.ui \
	src/ui/region_dialog.ui \
	src/ui/relocate_photos_dialog.ui \
	src/ui/settings_window.ui \
	src/ui/trip_dialog.ui



resources.files = \
	resources/icons/logo.svg \
	resources/icons/ascent.svg \
	resources/icons/peak.svg \
	resources/icons/trip.svg \
	resources/icons/hiker.svg \
	resources/icons/region.svg \
	resources/icons/range.svg \
	resources/icons/country.svg \
	resources/icons/ascent_viewer.svg \
	resources/icons/png/logo_128.png \
	resources/icons/ico/logo_peak_multisize_square.ico \
	resources/icons/ico/ascent_multisize_square.ico \
	resources/icons/ico/trip_multisize_square.ico \
	resources/icons/ico/hiker_multisize_square.ico \
	resources/icons/ico/region_multisize_square.ico \
	resources/icons/ico/range_multisize_square.ico \
	resources/icons/ico/country_multisize_square.ico \
	resources/icons/ico/ascent_viewer_multisize_square.ico \
	resources/icons/ico/statistics_multisize_square.ico \
	resources/gpl/gpl-v3-logo.svg
resources.base = resources
resources.prefix = /

RESOURCES = resources



RC_ICONS = resources/icons/ico/logo_peak_multisize_square.ico



TRANSLATIONS += \
	translation/en.ts \
	translation/de.ts

DISTFILES += \
	translation/en.ts \
	translation/de.ts
