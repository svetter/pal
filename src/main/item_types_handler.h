/*
 * Copyright 2023-2024 Simon Vetter
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

/**
 * @file item_types_handler.h
 * 
 * This file defines the ItemTypeMapper class with its item-specific subtypes and the
 * ItemTypesHandler class.
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
#include "src/stats/stats_engine.h"
#include "src/settings/settings.h"

#include <QWidget>
#include <QTableView>



/**
 * A helper class for storing pointers to different kinds of item-specific members, like dialogs,
 * settings, buttons and other UI elements.
 * 
 * An instance of ItemTypeMapper is specific to one PALItemType (like ascent, peak...). One
 * ItemTypeMapper instance is created for each PALItemType and stored in the ItemTypesHandler
 * at program startup.
 * 
 * ItemTypeMapper subtypes should be treates as singletons.
 */
class ItemTypeMapper {
public:
	/** The type of item (like ascent, peak...) that this mapper contains pointers for. */
	const PALItemType		type;
	
	/** The name of the item type (like "ascent", "peak"...). */
	const QString			name;
	
	/** The SQL buffer (base) table containing data of this item type. */
	NormalTable&			baseTable;
	/** The UI buffer (composite) table containing data of this item type. */
	CompositeTable* const	compTable;
	
	/** The item-specific statistics engine for this item type. */
	ItemStatsEngine* const	statsEngine;
	
	/** The tab in the main window corresponding to this item type. */
	QWidget* const			tab;
	/** The table view in the main window showing the composite table of this item type. */
	QTableView* const		tableView;
	/** The scroll area for displaying item-related statistics next to the table in the item's tab. */
	QScrollArea* const		statsScrollArea;
	
	/** The action in the main window menu for creating a new item of this type. */
	QAction* const			newItemAction;
	/** The button in the main window for creating a new item of this type. */
	QPushButton* const		newItemButton;
	
	/** The setting storing the geometry of the dialog for creating and editing items of this type. */
	const Setting<QRect>* const			dialogGeometrySetting;
	/** The setting storing whether the stats panel next to the UI table of this item type is visible. */
	const Setting<bool>* const			showStatsPanelSetting;
	/** The setting storing the splitter sizes for the stats panel next to the UI table of this item type. */
	const Setting<QStringList>* const	statsPanelSplitterSizesSetting;
	/** The setting storing the column widths for the UI table of this item type. */
	ProjectMultiSetting<int>* const		columnWidthsSetting;
	/** The setting storing the column order for the UI table of this item type. */
	ProjectMultiSetting<int>* const		columnOrderSetting;
	/** The setting storing the column hidden states for the UI table of this item type. */
	ProjectMultiSetting<bool>* const	hiddenColumnsSetting;
	/** The setting storing the sorting of the UI table of this item type. */
	ProjectSetting<QString>* const		sortingSetting;

	/** The method opening the dialog for creating a new item of this type. */
	BufferRowIndex	(* const openNewItemDialogAndStoreMethod)		(QWidget*, QMainWindow*, Database*);
	/** The method opening the dialog for duplicating an item of this type. */
	BufferRowIndex	(* const openDuplicateItemDialogAndStoreMethod)	(QWidget*, QMainWindow*, Database*, BufferRowIndex);
	/** The method opening the dialog for editing an item of this type. */
	bool			(* const openEditItemDialogAndStoreMethod)		(QWidget*, QMainWindow*, Database*, BufferRowIndex);
	/** The method opening the dialog for deleting an item of this type. */
	bool			(* const openDeleteItemsDialogAndExecuteMethod)	(QWidget*, QMainWindow*, Database*, QSet<BufferRowIndex>);
	
	
private:
	/** Indicates whether during the current complete runtime of the application, the tab corresponding to the item type was ever opened. */
	bool hasBeenOpenedDuringRuntime;
	/** Indicates whether since the current project was opened, the tab corresponding to the item type was ever opened and the table fully initialized. */
	bool hasBeenOpenedInProject;
	
	
	
public:
	/**
	 * Creates a new ItemTypeMapper instance.
	 * 
	 * @param type									The type of item for this mapper.
	 * @param name									The name of the item type for this mapper.
	 * @param db									The database.
	 * @param baseTable								The SQL buffer (base) table.
	 * @param compTable								The UI buffer (composite) table.
	 * @param tab									The tab in the main window.
	 * @param tableView								The table view in the main window showing the composite table.
	 * @param statsScrollArea						The scroll area for displaying item-related statistics next to the table in the item's tab.
	 * @param newItemAction							The action in the main window menu for creating a new iteme.
	 * @param newItemButton							The button in the main window for creating a new item.
	 * @param columnWidthsSetting					The setting storing the column widths of the UI table.
	 * @param columnOrderSetting					The setting storing the column order of the UI table.
	 * @param hiddenColumnsSetting					The setting storing the column hidden states of the UI table.
	 * @param sortingSetting						The setting storing the sorting of the UI table.
	 * @param dialogGeometrySetting					The setting storing the geometry of the item dialog.
	 * @param showStatsPanelSetting					The setting storing whether the stats panel next to the UI table is visible.
	 * @param statsPanelSplitterSizesSetting		The setting storing the splitter sizes of the stats panel next to the UI table.
	 * @param openNewItemDialogAndStoreMethod		The method opening the dialog for creating a new item.
	 * @param openDuplicateItemDialogAndStoreMethod	The method opening the dialog for duplicating an item.
	 * @param openEditItemDialogAndStoreMethod		The method opening the dialog for editing an item.
	 * @param openDeleteItemsDialogAndExecuteMethod	The method opening the dialog for deleting an item.
	 */
	inline ItemTypeMapper(
		PALItemType					type,
		QString						name,
		Database*					db,
		NormalTable&				baseTable,
		CompositeTable*				compTable,
		QWidget*					tab,
		QTableView*					tableView,
		QScrollArea*				statsScrollArea,
		QAction*					newItemAction,
		QPushButton*				newItemButton,
		ProjectMultiSetting<int>*	columnWidthsSetting,
		ProjectMultiSetting<int>*	columnOrderSetting,
		ProjectMultiSetting<bool>*	hiddenColumnsSetting,
		ProjectSetting<QString>*	sortingSetting,
		const Setting<QRect>*		dialogGeometrySetting,
		const Setting<bool>*		showStatsPanelSetting,
		const Setting<QStringList>*	statsPanelSplitterSizesSetting,
		BufferRowIndex	(* const openNewItemDialogAndStoreMethod)		(QWidget*, QMainWindow*, Database*),
		BufferRowIndex	(* const openDuplicateItemDialogAndStoreMethod)	(QWidget*, QMainWindow*, Database*, BufferRowIndex),
		bool			(* const openEditItemDialogAndStoreMethod)		(QWidget*, QMainWindow*, Database*, BufferRowIndex),
		bool			(* const openDeleteItemsDialogAndExecuteMethod)	(QWidget*, QMainWindow*, Database*, QSet<BufferRowIndex>)
	) :
		type									(type),
		name									(name),
		baseTable								(baseTable),
		compTable								(compTable),
		statsEngine								(new ItemStatsEngine(db, type, baseTable, statsScrollArea->findChild<QVBoxLayout*>())),
		tab										(tab),
		tableView								(tableView),
		statsScrollArea							(statsScrollArea),
		newItemAction							(newItemAction),
		newItemButton							(newItemButton),
		dialogGeometrySetting					(dialogGeometrySetting),
		showStatsPanelSetting					(showStatsPanelSetting),
		statsPanelSplitterSizesSetting			(statsPanelSplitterSizesSetting),
		columnWidthsSetting						(columnWidthsSetting),
		columnOrderSetting						(columnOrderSetting),
		hiddenColumnsSetting					(hiddenColumnsSetting),
		sortingSetting							(sortingSetting),
		openNewItemDialogAndStoreMethod			(openNewItemDialogAndStoreMethod),
		openDuplicateItemDialogAndStoreMethod	(openDuplicateItemDialogAndStoreMethod),
		openEditItemDialogAndStoreMethod		(openEditItemDialogAndStoreMethod),
		openDeleteItemsDialogAndExecuteMethod	(openDeleteItemsDialogAndExecuteMethod),
		hasBeenOpenedDuringRuntime				(false),
		hasBeenOpenedInProject					(false)
	{}
	
	/**
	 * Destroys this ItemTypeMapper instance.
	 */
	inline ~ItemTypeMapper()
	{
		delete compTable;
	}
	
	
	inline bool itemStatsPanelCurrentlySetVisible() const
	{
		return statsScrollArea->isVisibleTo(tab);
	}
	
	/**
	 * Notifies the item type mapper that the tab corresponding to it is curerntly being opened and
	 * fully initialized.
	 */
	inline void openingTab()
	{
		hasBeenOpenedDuringRuntime = true;
		hasBeenOpenedInProject = true;
	}
	
	/**
	 * Whether during the runtime of the application or since the current project was opened, the
	 * tab corresponding to this mapper was ever opened.
	 * 
	 * @param runtimeNotProject	Whether openings of the tab before the current project was opened should be regarded.
	 * @return					True if the tab has been open so far, false otherwise.
	 */
	inline bool tabHasBeenOpened(bool runtimeNotProject) const
	{
		return runtimeNotProject ? hasBeenOpenedDuringRuntime : hasBeenOpenedInProject;
	}
	
	/**
	 * Resets stored bool indicating whether the tab corresponding to this mapper was ever opened
	 * since the current project was opened.
	 */
	inline void resetTabHasBeenOpened()
	{
		hasBeenOpenedInProject = false;
	}
};



/**
 * The arguments passed to the constructor of each ItemTypeMapper subtype.
 * 
 * These arguments are dynamic, i.e. only known at runtime, so they have to be passed to the
 * constructor by code with access to the dynamic elements.
 */
#define TYPE_MAPPER_DYNAMIC_ARG_DECLARATIONS \
	Database*					db, \
	QWidget*					tab, \
	QTableView*					tableView, \
	QScrollArea*				statsScrollArea, \
	QAction*					newItemAction, \
	QPushButton*				newItemButton, \
	ProjectMultiSetting<int>*	columnWidthsSetting, \
	ProjectMultiSetting<int>*	columnOrderSetting, \
	ProjectMultiSetting<bool>*	hiddenColumnsSetting, \
	ProjectSetting<QString>*	sortingSetting

/**
 * The dynamic constructor arguments from all ItemTypeMapper subtypes which are passed on to the
 * ItemTypeMapper constructor.
 */
#define TYPE_MAPPER_DYNAMIC_ARG_NAMES \
	tab, \
	tableView, \
	statsScrollArea, \
	newItemAction, \
	newItemButton, \
	columnWidthsSetting, \
	columnOrderSetting, \
	hiddenColumnsSetting, \
	sortingSetting



/**
 * An ItemTypeMapper for item type Ascent.
 */
class AscentMapper : public ItemTypeMapper {
public:
	/**
	 * Creates a new AscentMapper instance.
	 * 
	 * @param db					The database.
	 * @param tab					The tab in the main window.
	 * @param tableView				The table view in the main window showing the composite table.
	 * @param statsScrollArea		The scroll area for displaying item-related statistics next to the table in the item's tab.
	 * @param newItemAction			The action in the main window menu for creating a new ascent.
	 * @param newItemButton			The button in the main window for creating a new ascent.
	 * @param columnWidthsSetting	The setting storing the column widths of the UI table.
	 * @param columnOrderSetting	The setting storing the column order of the UI table.
	 * @param hiddenColumnsSetting	The setting storing the column hidden states of the UI table.
	 * @param sortingSetting		The setting storing the sorting of the UI table.
	 */
	inline AscentMapper(TYPE_MAPPER_DYNAMIC_ARG_DECLARATIONS) :
		ItemTypeMapper(ItemTypeAscent, "ascent", db,
			db->ascentsTable,
			new CompositeAscentsTable(db, tableView),
			TYPE_MAPPER_DYNAMIC_ARG_NAMES,
			&Settings::ascentDialog_geometry,
			&Settings::ascentsStats_show,
			&Settings::ascentsStats_splitterSizes,
			&openNewAscentDialogAndStore,
			&openDuplicateAscentDialogAndStore,
			&openEditAscentDialogAndStore,
			&openDeleteAscentsDialogAndExecute
		)
	{}
};

/**
 * An ItemTypeMapper for item type Peak.
 */
class PeakMapper : public ItemTypeMapper {
public:
	/**
	 * Creates a new PeakMapper instance.
	 * 
	 * @param db					The database.
	 * @param tab					The tab in the main window.
	 * @param tableView				The table view in the main window showing the composite table.
	 * @param statsScrollArea		The scroll area for displaying item-related statistics next to the table in the item's tab.
	 * @param newItemAction			The action in the main window menu for creating a new ascent.
	 * @param newItemButton			The button in the main window for creating a new ascent.
	 * @param columnWidthsSetting	The setting storing the column widths of the UI table.
	 * @param columnOrderSetting	The setting storing the column order of the UI table.
	 * @param hiddenColumnsSetting	The setting storing the column hidden states of the UI table.
	 * @param sortingSetting		The setting storing the sorting of the UI table.
	 */
	inline PeakMapper(TYPE_MAPPER_DYNAMIC_ARG_DECLARATIONS) :
		ItemTypeMapper(ItemTypePeak, "peak", db,
			db->peaksTable,
			new CompositePeaksTable(db, tableView),
			TYPE_MAPPER_DYNAMIC_ARG_NAMES,
			&Settings::peakDialog_geometry,
			&Settings::peaksStats_show,
			&Settings::peaksStats_splitterSizes,
			&openNewPeakDialogAndStore,
			&openDuplicatePeakDialogAndStore,
			&openEditPeakDialogAndStore,
			&openDeletePeaksDialogAndExecute
		)
	{}
};

/**
 * An ItemTypeMapper for item type Trip.
 */
class TripMapper : public ItemTypeMapper {
public:
	/**
	 * Creates a new TripMapper instance.
	 * 
	 * @param db					The database.
	 * @param tab					The tab in the main window.
	 * @param tableView				The table view in the main window showing the composite table.
	 * @param statsScrollArea		The scroll area for displaying item-related statistics next to the table in the item's tab.
	 * @param newItemAction			The action in the main window menu for creating a new ascent.
	 * @param newItemButton			The button in the main window for creating a new ascent.
	 * @param columnWidthsSetting	The setting storing the column widths of the UI table.
	 * @param columnOrderSetting	The setting storing the column order of the UI table.
	 * @param hiddenColumnsSetting	The setting storing the column hidden states of the UI table.
	 * @param sortingSetting		The setting storing the sorting of the UI table.
	 */
	inline TripMapper(TYPE_MAPPER_DYNAMIC_ARG_DECLARATIONS) :
		ItemTypeMapper(ItemTypeTrip, "trip", db,
			db->tripsTable,
			new CompositeTripsTable(db, tableView),
			TYPE_MAPPER_DYNAMIC_ARG_NAMES,
			&Settings::tripDialog_geometry,
			&Settings::tripsStats_show,
			&Settings::tripsStats_splitterSizes,
			&openNewTripDialogAndStore,
			nullptr,
			&openEditTripDialogAndStore,
			&openDeleteTripsDialogAndExecute
		)
	{}
};

/**
 * An ItemTypeMapper for item type Hiker.
 */
class HikerMapper : public ItemTypeMapper {
public:
	/**
	 * Creates a new HikerMapper instance.
	 * 
	 * @param db					The database.
	 * @param tab					The tab in the main window.
	 * @param tableView				The table view in the main window showing the composite table.
	 * @param statsScrollArea		The scroll area for displaying item-related statistics next to the table in the item's tab.
	 * @param newItemAction			The action in the main window menu for creating a new ascent.
	 * @param newItemButton			The button in the main window for creating a new ascent.
	 * @param columnWidthsSetting	The setting storing the column widths of the UI table.
	 * @param columnOrderSetting	The setting storing the column order of the UI table.
	 * @param hiddenColumnsSetting	The setting storing the column hidden states of the UI table.
	 * @param sortingSetting		The setting storing the sorting of the UI table.
	 */
	inline HikerMapper(TYPE_MAPPER_DYNAMIC_ARG_DECLARATIONS) :
		ItemTypeMapper(ItemTypeHiker, "hiker", db,
			db->hikersTable,
			new CompositeHikersTable(db, tableView),
			TYPE_MAPPER_DYNAMIC_ARG_NAMES,
			&Settings::hikerDialog_geometry,
			&Settings::hikersStats_show,
			&Settings::hikersStats_splitterSizes,
			&openNewHikerDialogAndStore,
			nullptr,
			&openEditHikerDialogAndStore,
			&openDeleteHikersDialogAndExecute
		)
	{}
};

/**
 * An ItemTypeMapper for item type Region.
 */
class RegionMapper : public ItemTypeMapper {
public:
	/**
	 * Creates a new RegionMapper instance.
	 * 
	 * @param db					The database.
	 * @param tab					The tab in the main window.
	 * @param tableView				The table view in the main window showing the composite table.
	 * @param statsScrollArea		The scroll area for displaying item-related statistics next to the table in the item's tab.
	 * @param newItemAction			The action in the main window menu for creating a new ascent.
	 * @param newItemButton			The button in the main window for creating a new ascent.
	 * @param columnWidthsSetting	The setting storing the column widths of the UI table.
	 * @param columnOrderSetting	The setting storing the column order of the UI table.
	 * @param hiddenColumnsSetting	The setting storing the column hidden states of the UI table.
	 * @param sortingSetting		The setting storing the sorting of the UI table.
	 */
	inline RegionMapper(TYPE_MAPPER_DYNAMIC_ARG_DECLARATIONS) :
		ItemTypeMapper(ItemTypeRegion, "region", db,
			db->regionsTable,
			new CompositeRegionsTable(db, tableView),
			TYPE_MAPPER_DYNAMIC_ARG_NAMES,
			&Settings::regionDialog_geometry,
			&Settings::regionsStats_show,
			&Settings::regionsStats_splitterSizes,
			&openNewRegionDialogAndStore,
			nullptr,
			&openEditRegionDialogAndStore,
			&openDeleteRegionsDialogAndExecute
		)
	{}
};

/**
 * An ItemTypeMapper for item type Range.
 */
class RangeMapper : public ItemTypeMapper {
public:
	/**
	 * Creates a new RangeMapper instance.
	 * 
	 * @param db					The database.
	 * @param tab					The tab in the main window.
	 * @param tableView				The table view in the main window showing the composite table.
	 * @param statsScrollArea		The scroll area for displaying item-related statistics next to the table in the item's tab.
	 * @param newItemAction			The action in the main window menu for creating a new ascent.
	 * @param newItemButton			The button in the main window for creating a new ascent.
	 * @param columnWidthsSetting	The setting storing the column widths of the UI table.
	 * @param columnOrderSetting	The setting storing the column order of the UI table.
	 * @param hiddenColumnsSetting	The setting storing the column hidden states of the UI table.
	 * @param sortingSetting		The setting storing the sorting of the UI table.
	 */
	inline RangeMapper(TYPE_MAPPER_DYNAMIC_ARG_DECLARATIONS) :
		ItemTypeMapper(ItemTypeRange, "range", db,
			db->rangesTable,
			new CompositeRangesTable(db, tableView),
			TYPE_MAPPER_DYNAMIC_ARG_NAMES,
			&Settings::rangeDialog_geometry,
			&Settings::rangesStats_show,
			&Settings::rangesStats_splitterSizes,
			&openNewRangeDialogAndStore,
			nullptr,
			&openEditRangeDialogAndStore,
			&openDeleteRangesDialogAndExecute
		)
	{}
};

/**
 * An ItemTypeMapper for item type Country.
 */
class CountryMapper : public ItemTypeMapper {
public:
	/**
	 * Creates a new CountryMapper instance.
	 * 
	 * @param db					The database.
	 * @param tab					The tab in the main window.
	 * @param tableView				The table view in the main window showing the composite table.
	 * @param statsScrollArea		The scroll area for displaying item-related statistics next to the table in the item's tab.
	 * @param newItemAction			The action in the main window menu for creating a new ascent.
	 * @param newItemButton			The button in the main window for creating a new ascent.
	 * @param columnWidthsSetting	The setting storing the column widths of the UI table.
	 * @param columnOrderSetting	The setting storing the column order of the UI table.
	 * @param hiddenColumnsSetting	The setting storing the column hidden states of the UI table.
	 * @param sortingSetting		The setting storing the sorting of the UI table.
	 */
	inline CountryMapper(TYPE_MAPPER_DYNAMIC_ARG_DECLARATIONS) :
		ItemTypeMapper(ItemTypeCountry, "country", db,
			db->countriesTable,
			new CompositeCountriesTable(db, tableView),
			TYPE_MAPPER_DYNAMIC_ARG_NAMES,
			&Settings::countryDialog_geometry,
			&Settings::countriesStats_show,
			&Settings::countriesStats_splitterSizes,
			&openNewCountryDialogAndStore,
			nullptr,
			&openEditCountryDialogAndStore,
			&openDeleteCountriesDialogAndExecute
		)
	{}
};





/**
 * A helper class for storing all ItemTypeMapper instances and generalizing access to and
 * operations on them.
 * 
 * Only one instance of ItemTypesHandler is needed, it should be treated as a singleton.
 * All members and methods are const, so ItemTypesHandler instances can be passed around freely.
 */
class ItemTypesHandler {
public:
	/** The ItemTypeMapper instances for all item types. */
	const QMap<PALItemType, ItemTypeMapper*> mappers;
	
	/** The base table for photos. */
	PhotosTable& photosTable;
	/** The base table for hiker participation in trips. */
	ParticipatedTable& participatedTable;
	
	/**
	 * Creates a new ItemTypesHandler instance.
	 * 
	 * @param ascentMapper		The mapper for item type ascent.
	 * @param peakMapper		The mapper for item type peak.
	 * @param tripMapper		The mapper for item type trip.
	 * @param hikerMapper		The mapper for item type hiker.
	 * @param regionMapper		The mapper for item type region.
	 * @param rangeMapper		The mapper for item type range.
	 * @param countryMapper		The mapper for item type country.
	 * @param photosTable		The database's photos table.
	 * @param participatedTable	The database's hiker participation table.
	 */
	inline ItemTypesHandler(
		AscentMapper*		ascentMapper,
		PeakMapper*			peakMapper,
		TripMapper*			tripMapper,
		HikerMapper*		hikerMapper,
		RegionMapper*		regionMapper,
		RangeMapper*		rangeMapper,
		CountryMapper*		countryMapper,
		PhotosTable&		photosTable,
		ParticipatedTable&	participatedTable
	) :
		mappers({
			{ItemTypeAscent,	ascentMapper},
			{ItemTypePeak,		peakMapper},
			{ItemTypeTrip,		tripMapper},
			{ItemTypeHiker,		hikerMapper},
			{ItemTypeRegion,	regionMapper},
			{ItemTypeRange,		rangeMapper},
			{ItemTypeCountry,	countryMapper},
		}),
		photosTable(photosTable),
		participatedTable(participatedTable)
	{}
	
	/**
	 * Destroys the ItemTypesHandler.
	 */
	inline ~ItemTypesHandler()
	{
		qDeleteAll(mappers);
	}
	
	
	/**
	 * Returns a complete list of ItemTypeMapper instances for every item type.
	 * 
	 * @return	A list of every ItemTypeMapper.
	 */
	inline const QList<ItemTypeMapper*> getAllMappers() const
	{
		QList<ItemTypeMapper*> list = QList<ItemTypeMapper*>();
		for (ItemTypeMapper* mapper : mappers) {
			list += mapper;
		}
		return list;
	}
	
	/**
	 * Returns the ItemTypeMapper instance for the given item type.
	 * 
	 * @param type	The item type.
	 * @return		The ItemTypeMapper instance for the given item type.
	 */
	inline const ItemTypeMapper* get(PALItemType type) const
	{
		return mappers.value(type);
	}
	
	/**
	 * Returns a list of all base tables, including photos and participated tables, in a fixed
	 * order.
	 * 
	 * @return	All base tables in a fixed order.
	 */
	inline QList<Table*> getAllBaseTables() const
	{
		QList<Table*> tables = QList<Table*>();
		for (const ItemTypeMapper* const mapper : mappers) {
			tables += &mapper->baseTable;
		}
		tables.append(&photosTable);
		tables.append(&participatedTable);
		return tables;
	}
	
	/**
	 * Returns the ItemTypeMapper for the item type to which the given table view belongs.
	 * 
	 * Optionally, writes back a bool via pointer to indicate whether the given table view belongs
	 * to a debug table.
	 * 
	 * @return	The ItemTypeMapper for the item type to which the given table view belongs, or nullptr if no table tab is open.
	 */
	inline ItemTypeMapper* getActiveMapper() const
	{
		ItemTypeMapper* matchingMapper = nullptr;
		for (ItemTypeMapper* const mapper : mappers) {
			if (mapper->tab->isVisible()) {
				matchingMapper = mapper;
				break;
			}
		}
		return matchingMapper;
	}
	
	/**
	 * Resets the flags indicating whether the tabs corresponding to the item types were ever opened
	 * since the current project was opened.
	 * 
	 * To be called before a new project is opened.
	 */
	inline void resetTabOpenedFlags() const
	{
		for (ItemTypeMapper* const mapper : mappers) {
			mapper->resetTabHasBeenOpened();
		}
	}
};



#endif // ITEM_TYPES_HANDLER_H
