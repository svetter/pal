/*
 * Copyright 2023 Simon Vetter
 * 
 * This file is part of PeakAscentLogger.
 * 
 * PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with PeakAscentLogger.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ITEM_TYPES_HANDLER_H
#define ITEM_TYPES_HANDLER_H

#include "src/comp_tables/comp_ascents_table.h"
#include "src/comp_tables/comp_countries_table.h"
#include "src/comp_tables/comp_hikers_table.h"
#include "src/comp_tables/comp_peaks_table.h"
#include "src/comp_tables/comp_ranges_table.h"
#include "src/comp_tables/comp_regions_table.h"
#include "src/comp_tables/comp_trips_table.h"
#include "src/comp_tables/composite_table.h"
#include "src/dialogs/ascent_dialog.h"
#include "src/dialogs/country_dialog.h"
#include "src/dialogs/hiker_dialog.h"
#include "src/dialogs/peak_dialog.h"
#include "src/dialogs/range_dialog.h"
#include "src/dialogs/region_dialog.h"
#include "src/dialogs/trip_dialog.h"
#include "src/main/settings.h"

#include <QWidget>
#include <QTableView>



enum PALItemType {
	ItemTypeAscent,
	ItemTypePeak,
	ItemTypeTrip,
	ItemTypeHiker,
	ItemTypeRegion,
	ItemTypeRange,
	ItemTypeCountry
};



class ItemTypeMapper {
public:
	const PALItemType		type;
	
	const QString			name;
	
	NormalTable* const		baseTable;
	CompositeTable* const	compTable;
	
	QWidget* const			tab;
	QTableView* const		tableView;
	QTableView* const		debugTableView;
	
	QAction* const			newItemAction;
	QPushButton* const		newItemButton;
	
	const Setting<QRect>* const			dialogGeometrySetting;
	const Setting<QStringList>* const	columnWidthsSetting;
	const Setting<QStringList>* const	sortingSetting;
	
	BufferRowIndex	(* const openNewItemDialogAndStoreMethod)		(QWidget*, Database*);
	BufferRowIndex	(* const openDuplicateItemDialogAndStoreMethod)	(QWidget*, Database*, BufferRowIndex);
	void			(* const openEditItemDialogAndStoreMethod)		(QWidget*, Database*, BufferRowIndex);
	void			(* const openDeleteItemDialogAndStoreMethod)	(QWidget*, Database*, BufferRowIndex);
	
	
	
	inline ItemTypeMapper(
			PALItemType			type,
			QString				name,
			NormalTable*		baseTable,
			CompositeTable*		compTable,
			QWidget*			tab,
			QTableView*			tableView,
			QTableView*			debugTableView,
			QAction* const		newItemAction,
			QPushButton* const	newItemButton,
			const Setting<QRect>*		dialogGeometrySetting,
			const Setting<QStringList>*	columnWidthsSetting,
			const Setting<QStringList>*	sortingSetting,
			BufferRowIndex	(* const openNewItemDialogAndStoreMethod)		(QWidget*, Database*),
			BufferRowIndex	(* const openDuplicateItemDialogAndStoreMethod)	(QWidget*, Database*, BufferRowIndex),
			void			(* const openEditItemDialogAndStoreMethod)		(QWidget*, Database*, BufferRowIndex),
			void			(* const openDeleteItemDialogAndStoreMethod)	(QWidget*, Database*, BufferRowIndex)
			) :
			type									(type),
			name									(name),
			baseTable								(baseTable),
			compTable								(compTable),
			tab										(tab),
			tableView								(tableView),
			debugTableView							(debugTableView),
			newItemAction							(newItemAction),
			newItemButton							(newItemButton),
			dialogGeometrySetting					(dialogGeometrySetting),
			columnWidthsSetting						(columnWidthsSetting),
			sortingSetting							(sortingSetting),
			openNewItemDialogAndStoreMethod			(openNewItemDialogAndStoreMethod),
			openDuplicateItemDialogAndStoreMethod	(openDuplicateItemDialogAndStoreMethod),
			openEditItemDialogAndStoreMethod		(openEditItemDialogAndStoreMethod),
			openDeleteItemDialogAndStoreMethod		(openDeleteItemDialogAndStoreMethod)
	{}
	
	inline ~ItemTypeMapper()
	{
		delete compTable;
	}
};



#define TYPE_MAPPER_DYNAMIC_ARG_DECLARATIONS \
	Database*			db, \
	QWidget*			tab, \
	QTableView*			tableView, \
	QTableView*			debugTableView, \
	QAction* const		newItemAction, \
	QPushButton* const	newItemButton

#define TYPE_MAPPER_DYNAMIC_ARG_NAMES \
	tab, \
	tableView, \
	debugTableView, \
	newItemAction, \
	newItemButton



class AscentTypeMapper : public ItemTypeMapper {
public:
	inline AscentTypeMapper(TYPE_MAPPER_DYNAMIC_ARG_DECLARATIONS) :
			ItemTypeMapper(ItemTypeAscent, "ascent", db->ascentsTable, new CompositeAscentsTable(db, tableView), TYPE_MAPPER_DYNAMIC_ARG_NAMES,
				&Settings::ascentDialog_geometry,
				&Settings::mainWindow_columnWidths_ascentsTable,
				&Settings::mainWindow_sorting_ascentsTable,
				&openNewAscentDialogAndStore,
				&openDuplicateAscentDialogAndStore,
				&openEditAscentDialogAndStore,
				&openDeleteAscentDialogAndExecute
			)
	{}
};

class PeakTypeMapper : public ItemTypeMapper {
public:
	inline PeakTypeMapper(TYPE_MAPPER_DYNAMIC_ARG_DECLARATIONS) :
			ItemTypeMapper(ItemTypePeak, "peak", db->peaksTable, new CompositePeaksTable(db, tableView), TYPE_MAPPER_DYNAMIC_ARG_NAMES,
				&Settings::peakDialog_geometry,
				&Settings::mainWindow_columnWidths_peaksTable,
				&Settings::mainWindow_sorting_peaksTable,
				&openNewPeakDialogAndStore,
				&openDuplicatePeakDialogAndStore,
				&openEditPeakDialogAndStore,
				&openDeletePeakDialogAndExecute
			)
	{}
};

class TripTypeMapper : public ItemTypeMapper {
public:
	inline TripTypeMapper(TYPE_MAPPER_DYNAMIC_ARG_DECLARATIONS) :
			ItemTypeMapper(ItemTypeTrip, "trip", db->tripsTable, new CompositeTripsTable(db, tableView), TYPE_MAPPER_DYNAMIC_ARG_NAMES,
				&Settings::tripDialog_geometry,
				&Settings::mainWindow_columnWidths_tripsTable,
				&Settings::mainWindow_sorting_tripsTable,
				&openNewTripDialogAndStore,
				nullptr,
				&openEditTripDialogAndStore,
				&openDeleteTripDialogAndExecute
			)
	{}
};

class HikerTypeMapper : public ItemTypeMapper {
public:
	inline HikerTypeMapper(TYPE_MAPPER_DYNAMIC_ARG_DECLARATIONS) :
			ItemTypeMapper(ItemTypeHiker, "hiker", db->hikersTable, new CompositeHikersTable(db, tableView), TYPE_MAPPER_DYNAMIC_ARG_NAMES,
				&Settings::hikerDialog_geometry,
				&Settings::mainWindow_columnWidths_hikersTable,
				&Settings::mainWindow_sorting_hikersTable,
				&openNewHikerDialogAndStore,
				nullptr,
				&openEditHikerDialogAndStore,
				&openDeleteHikerDialogAndExecute
			)
	{}
};

class RegionTypeMapper : public ItemTypeMapper {
public:
	inline RegionTypeMapper(TYPE_MAPPER_DYNAMIC_ARG_DECLARATIONS) :
			ItemTypeMapper(ItemTypeRegion, "region", db->regionsTable, new CompositeRegionsTable(db, tableView), TYPE_MAPPER_DYNAMIC_ARG_NAMES,
				&Settings::regionDialog_geometry,
				&Settings::mainWindow_columnWidths_regionsTable,
				&Settings::mainWindow_sorting_regionsTable,
				&openNewRegionDialogAndStore,
				nullptr,
				&openEditRegionDialogAndStore,
				&openDeleteRegionDialogAndExecute
			)
	{}
};

class RangeTypeMapper : public ItemTypeMapper {
public:
	inline RangeTypeMapper(TYPE_MAPPER_DYNAMIC_ARG_DECLARATIONS) :
			ItemTypeMapper(ItemTypeRange, "range", db->rangesTable, new CompositeRangesTable(db, tableView), TYPE_MAPPER_DYNAMIC_ARG_NAMES,
				&Settings::rangeDialog_geometry,
				&Settings::mainWindow_columnWidths_rangesTable,
				&Settings::mainWindow_sorting_rangesTable,
				&openNewRangeDialogAndStore,
				nullptr,
				&openEditRangeDialogAndStore,
				&openDeleteRangeDialogAndExecute
			)
	{}
};

class CountryTypeMapper : public ItemTypeMapper {
public:
	inline CountryTypeMapper(TYPE_MAPPER_DYNAMIC_ARG_DECLARATIONS) :
			ItemTypeMapper(ItemTypeCountry, "country", db->countriesTable, new CompositeCountriesTable(db, tableView), TYPE_MAPPER_DYNAMIC_ARG_NAMES,
				&Settings::countryDialog_geometry,
				&Settings::mainWindow_columnWidths_countriesTable,
				&Settings::mainWindow_sorting_countriesTable,
				&openNewCountryDialogAndStore,
				nullptr,
				&openEditCountryDialogAndStore,
				&openDeleteCountryDialogAndExecute
			)
	{}
};





class ItemTypesHandler {
public:
	const bool showDebugTableViews;
	const QMap<PALItemType, const ItemTypeMapper*> mappers;
	
	inline ItemTypesHandler(bool showDebugTableViews,
			const AscentTypeMapper*		ascentMapper,
			const PeakTypeMapper*		peakMapper,
			const TripTypeMapper*		tripMapper,
			const HikerTypeMapper*		hikerMapper,
			const RegionTypeMapper*		regionMapper,
			const RangeTypeMapper*		rangeMapper,
			const CountryTypeMapper*	countryMapper
			) :
			showDebugTableViews(showDebugTableViews),
			mappers({
				{ItemTypeAscent,	ascentMapper},
				{ItemTypePeak,		peakMapper},
				{ItemTypeTrip,		tripMapper},
				{ItemTypeHiker,		hikerMapper},
				{ItemTypeRegion,	regionMapper},
				{ItemTypeRange,		rangeMapper},
				{ItemTypeCountry,	countryMapper},
			})
	{}
	
	inline const ItemTypeMapper* get(PALItemType type) const
	{
		return mappers.value(type);
	}
	
	inline void forEach(std::function<void (const ItemTypeMapper&)> lambda) const
	{
		for (const ItemTypeMapper* mapper : mappers) {
			lambda(*mapper);
		}
	}
	
	inline bool forMatchingTableView(QTableView* tableView, std::function<void (const ItemTypeMapper&, bool)> lambda) const
	{
		const ItemTypeMapper* matchingMapper = nullptr;
		bool debugTable = false;
		for (const ItemTypeMapper* mapper : mappers) {
			if (mapper->tableView == tableView) matchingMapper = mapper;
			if (mapper->debugTableView == tableView) {
				matchingMapper = mapper;
				debugTable = true;
			}
		}
		if (!matchingMapper) return false;
		
		lambda(*matchingMapper, debugTable);
		return true;
	}
};



#endif // ITEM_TYPES_HANDLER_H
