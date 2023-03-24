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
	Ascent,
	Peak,
	Trip,
	Hiker,
	Region,
	Range,
	Country
};



class ItemTypeMapper {
public:
	const PALItemType		type;
	
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
	
	int  (* const openNewItemDialogAndStoreMethod)			(QWidget*, Database*);
	int  (* const openDuplicateItemDialogAndStoreMethod)	(QWidget*, Database*, int);
	void (* const openEditItemDialogAndStoreMethod)			(QWidget*, Database*, int);
	void (* const openDeleteItemDialogAndStoreMethod)		(QWidget*, Database*, int);
	
	
	
	inline ItemTypeMapper(
			PALItemType			type,
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
			int  (* const openNewItemDialogAndStoreMethod)			(QWidget*, Database*),
			int  (* const openDuplicateItemDialogAndStoreMethod)	(QWidget*, Database*, int),
			void (* const openEditItemDialogAndStoreMethod)			(QWidget*, Database*, int),
			void (* const openDeleteItemDialogAndStoreMethod)		(QWidget*, Database*, int)
			) :
			type									(type),
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
			ItemTypeMapper(Ascent, db->ascentsTable, new CompositeAscentsTable(db), TYPE_MAPPER_DYNAMIC_ARG_NAMES,
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
			ItemTypeMapper(Peak, db->peaksTable, new CompositePeaksTable(db), TYPE_MAPPER_DYNAMIC_ARG_NAMES,
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
			ItemTypeMapper(Trip, db->tripsTable, new CompositeTripsTable(db), TYPE_MAPPER_DYNAMIC_ARG_NAMES,
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
			ItemTypeMapper(Hiker, db->hikersTable, new CompositeHikersTable(db), TYPE_MAPPER_DYNAMIC_ARG_NAMES,
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
			ItemTypeMapper(Region, db->regionsTable, new CompositeRegionsTable(db), TYPE_MAPPER_DYNAMIC_ARG_NAMES,
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
			ItemTypeMapper(Range, db->rangesTable, new CompositeRangesTable(db), TYPE_MAPPER_DYNAMIC_ARG_NAMES,
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
			ItemTypeMapper(Country, db->countriesTable, new CompositeCountriesTable(db), TYPE_MAPPER_DYNAMIC_ARG_NAMES,
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
	const QMap<PALItemType, const ItemTypeMapper*> mappers;
	
	inline ItemTypesHandler(
			const AscentTypeMapper*		ascentMapper,
			const PeakTypeMapper*		peakMapper,
			const TripTypeMapper*		tripMapper,
			const HikerTypeMapper*		hikerMapper,
			const RegionTypeMapper*		regionMapper,
			const RangeTypeMapper*		rangeMapper,
			const CountryTypeMapper*	countryMapper
			) :
			mappers({
				{Ascent,	ascentMapper},
				{Peak,		peakMapper},
				{Trip,		tripMapper},
				{Hiker,		hikerMapper},
				{Region,	regionMapper},
				{Range,		rangeMapper},
				{Country,	countryMapper},
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
