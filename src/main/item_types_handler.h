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

#include "src/comp_tables/comp_item_tables/comp_ascents_table.h"
#include "src/comp_tables/comp_item_tables/comp_countries_table.h"
#include "src/comp_tables/comp_item_tables/comp_hikers_table.h"
#include "src/comp_tables/comp_item_tables/comp_peaks_table.h"
#include "src/comp_tables/comp_item_tables/comp_ranges_table.h"
#include "src/comp_tables/comp_item_tables/comp_regions_table.h"
#include "src/comp_tables/comp_item_tables/comp_trips_table.h"
#include "src/comp_tables/composite_table.h"
#include "src/dialogs/ascent_dialog.h"
#include "src/dialogs/country_dialog.h"
#include "src/dialogs/hiker_dialog.h"
#include "src/dialogs/peak_dialog.h"
#include "src/dialogs/range_dialog.h"
#include "src/dialogs/region_dialog.h"
#include "src/dialogs/trip_dialog.h"
#include "src/main/filter_bar.h"
#include "src/stats/stats_engine.h"
#include "src/settings/settings.h"

#include <QWidget>
#include <QTableView>



/**
 * A helper struct for passing pointers to item-specific UI elements to the constructor of
 * ItemTypeMapper subtypes.
 */
struct TypeMapperPointers {
	/** The item type's tab in the main window. */
	QWidget*		tab;
	/** The item type's table view in the main window. */
	QTableView*		tableView;
	/** The item type's filter bar above the table in the main window. */
	FilterBar*		filterBar;
	/** The item type's stats scroll area in the main window. */
	QScrollArea*	statsScrollArea;
	/** The item type's action in the main window's menu bar for creating a new item. */
	QAction*		newItemAction;
	/** The item type's button in the main window for creating a new item. */
	QPushButton*	newItemButton;
};



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
	const PALItemType	type;
	
	/** The name of the item type (like "ascent", "peak"...). */
	const QString		name;
	
	/** The SQL buffer (base) table containing data of this item type. */
	NormalTable&		baseTable;
	/** The UI buffer (composite) table containing data of this item type. */
	CompositeTable&		compTable;
	
	/** The item-specific statistics engine for this item type. */
	ItemStatsEngine&	statsEngine;
	
	/** The tab in the main window corresponding to this item type. */
	QWidget&			tab;
	/** The table view in the main window showing the composite table of this item type. */
	QTableView&			tableView;
	/** The widget for table filters above the table in the item's tab. */
	FilterBar&			filterBar;
	/** The scroll area for displaying item-related statistics next to the table in the item's tab. */
	QScrollArea&		statsScrollArea;
	
	/** The action in the main window menu for creating a new item of this type. */
	QAction&			newItemAction;
	/** The button in the main window for creating a new item of this type. */
	QPushButton&		newItemButton;
	
	/** The setting storing the geometry of the dialog for creating and editing items of this type. */
	const Setting<QRect>&		dialogGeometrySetting;
	/** The setting storing whether the stats panel next to the UI table of this item type is visible. */
	const Setting<bool>&		showStatsPanelSetting;
	/** The setting storing the splitter sizes for the stats panel next to the UI table of this item type. */
	const Setting<QStringList>&	statsPanelSplitterSizesSetting;
	/** The setting storing the column widths for the UI table of this item type. */
	ProjectMultiSetting<int>&	columnWidthsSetting;
	/** The setting storing the column order for the UI table of this item type. */
	ProjectMultiSetting<int>&	columnOrderSetting;
	/** The setting storing the column hidden states for the UI table of this item type. */
	ProjectMultiSetting<bool>&	hiddenColumnsSetting;
	/** The setting storing the sorting of the UI table of this item type. */
	ProjectSetting<QString>&	sortingSetting;
	/** The setting storing whether the filter bar above the UI table of this item type is visible. */
	ProjectSetting<bool>&		showFilterBarSetting;
	/** The setting storing the filters for the UI table of this item type. */
	ProjectSetting<QString>&	filtersSetting;

	/** The method opening the dialog for creating a new item of this type. */
	void (* const openNewItemDialogAndStoreMethod)			(QWidget&, QMainWindow&, Database&, std::function<void (BufferRowIndex)>);
	/** The method opening the dialog for duplicating an item of this type. */
	void (* const openDuplicateItemDialogAndStoreMethod)	(QWidget&, QMainWindow&, Database&, BufferRowIndex, std::function<void (BufferRowIndex)>);
	/** The method opening the dialog for editing an item of this type. */
	void (* const openEditItemDialogAndStoreMethod)			(QWidget&, QMainWindow&, Database&, BufferRowIndex, std::function<void (bool)>);
	/** The method opening the dialog for editing multiple items of this type. */
	void (* const openMultiEditItemsDialogAndStoreMethod)	(QWidget&, QMainWindow&, Database&, const QSet<BufferRowIndex>&, BufferRowIndex, std::function<void (bool)>);
	/** The method opening the dialog for deleting an item of this type. */
	bool (* const openDeleteItemsDialogAndExecuteMethod)	(QWidget&, QMainWindow&, Database&, const QSet<BufferRowIndex>&);
	
	
private:
	/** Indicates whether during the current complete runtime of the application, the tab corresponding to the item type was ever opened. */
	bool hasBeenOpenedDuringRuntime;
	/** Indicates whether since the current project was opened, the tab corresponding to the item type was ever opened and the table fully initialized. */
	bool hasBeenOpenedInProject;
	
	
	
public:
	/**
	 * Creates a new ItemTypeMapper instance.
	 *
	 * @param db										The database.
	 * @param type										The type of item for this mapper.
	 * @param name										The name of the item type for this mapper.
	 * @param baseTable									The SQL buffer (base) table.
	 * @param compTable									The UI buffer (composite) table.
	 * @param pointerSupply								A struct containing pointers to item-specific UI elements.
	 * @param columnWidthsSetting						The setting storing the column widths of the UI table.
	 * @param columnOrderSetting						The setting storing the column order of the UI table.
	 * @param hiddenColumnsSetting						The setting storing the column hidden states of the UI table.
	 * @param sortingSetting							The setting storing the sorting of the UI table.
	 * @param showFilterBarSetting						The setting storing whether the filter bar above the UI table is visible.
	 * @param filtersSetting							The setting storing all filters for the UI table.
	 * @param dialogGeometrySetting						The setting storing the geometry of the item dialog.
	 * @param showStatsPanelSetting						The setting storing whether the stats panel next to the UI table is visible.
	 * @param statsPanelSplitterSizesSetting			The setting storing the splitter sizes of the stats panel next to the UI table.
	 * @param openNewItemDialogAndStoreMethod			The method opening the dialog for creating a new item.
	 * @param openDuplicateItemDialogAndStoreMethod		The method opening the dialog for duplicating an item.
	 * @param openEditItemDialogAndStoreMethod			The method opening the dialog for editing a single item.
	 * @param openMultiEditItemsDialogAndStoreMethod	The method opening the dialog for editing multiple items.
	 * @param openDeleteItemsDialogAndExecuteMethod		The method opening the dialog for deleting an item.
	 */
	inline ItemTypeMapper(
		Database&					db,
		PALItemType					type,
		QString						name,
		NormalTable&				baseTable,
		CompositeTable*				compTable,
		TypeMapperPointers&&		pointerSupply,
		ProjectMultiSetting<int>&	columnWidthsSetting,
		ProjectMultiSetting<int>&	columnOrderSetting,
		ProjectMultiSetting<bool>&	hiddenColumnsSetting,
		ProjectSetting<QString>&	sortingSetting,
		ProjectSetting<bool>&		showFilterBarSetting,
		ProjectSetting<QString>&	filtersSetting,
		const Setting<QRect>&		dialogGeometrySetting,
		const Setting<bool>&		showStatsPanelSetting,
		const Setting<QStringList>&	statsPanelSplitterSizesSetting,
		void (* const openNewItemDialogAndStoreMethod)			(QWidget&, QMainWindow&, Database&, std::function<void (BufferRowIndex)>),
		void (* const openDuplicateItemDialogAndStoreMethod)	(QWidget&, QMainWindow&, Database&, BufferRowIndex, std::function<void (BufferRowIndex)>),
		void (* const openEditItemDialogAndStoreMethod)			(QWidget&, QMainWindow&, Database&, BufferRowIndex, std::function<void (bool)>),
		void (* const openMultiEditItemsDialogAndStoreMethod)	(QWidget&, QMainWindow&, Database&, const QSet<BufferRowIndex>&, BufferRowIndex, std::function<void (bool)>),
		bool (* const openDeleteItemsDialogAndExecuteMethod)	(QWidget&, QMainWindow&, Database&, const QSet<BufferRowIndex>&)
	) :
		type									(type),
		name									(name),
		baseTable								(baseTable),
		compTable								(*compTable),
		statsEngine								(* new ItemStatsEngine(db, type, baseTable, pointerSupply.statsScrollArea->findChild<QVBoxLayout*>())),
		tab										(*pointerSupply.tab),
		tableView								(*pointerSupply.tableView),
		filterBar								(*pointerSupply.filterBar),
		statsScrollArea							(*pointerSupply.statsScrollArea),
		newItemAction							(*pointerSupply.newItemAction),
		newItemButton							(*pointerSupply.newItemButton),
		dialogGeometrySetting					(dialogGeometrySetting),
		showStatsPanelSetting					(showStatsPanelSetting),
		statsPanelSplitterSizesSetting			(statsPanelSplitterSizesSetting),
		columnWidthsSetting						(columnWidthsSetting),
		columnOrderSetting						(columnOrderSetting),
		hiddenColumnsSetting					(hiddenColumnsSetting),
		sortingSetting							(sortingSetting),
		showFilterBarSetting					(showFilterBarSetting),
		filtersSetting							(filtersSetting),
		openNewItemDialogAndStoreMethod			(openNewItemDialogAndStoreMethod),
		openDuplicateItemDialogAndStoreMethod	(openDuplicateItemDialogAndStoreMethod),
		openEditItemDialogAndStoreMethod		(openEditItemDialogAndStoreMethod),
		openMultiEditItemsDialogAndStoreMethod	(openMultiEditItemsDialogAndStoreMethod),
		openDeleteItemsDialogAndExecuteMethod	(openDeleteItemsDialogAndExecuteMethod),
		hasBeenOpenedDuringRuntime				(false),
		hasBeenOpenedInProject					(false)
	{}
	
	/**
	 * Destroys this ItemTypeMapper instance.
	 */
	inline ~ItemTypeMapper()
	{
		delete &compTable;
		delete &statsEngine;
	}
	
	
	inline bool itemStatsPanelCurrentlySetVisible() const
	{
		return statsScrollArea.isVisibleTo(&tab);
	}
	
	inline bool filterBarCurrentlySetVisible() const
	{
		return filterBar.isVisibleTo(&tab);
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
 * A helper class for storing all ItemTypeMapper instances and generalizing access to and
 * operations on them.
 * 
 * Only one instance of ItemTypesHandler is needed, it should be treated as a singleton.
 * All members and methods are const, so ItemTypesHandler instances can be passed around freely.
 */
class ItemTypesHandler {
public:
	/** The ItemTypeMapper instance for the ascent item type. */
	ItemTypeMapper ascentMapper;
	/** The ItemTypeMapper instance for the peak item type. */
	ItemTypeMapper peakMapper;
	/** The ItemTypeMapper instance for the trip item type. */
	ItemTypeMapper tripMapper;
	/** The ItemTypeMapper instance for the hiker item type. */
	ItemTypeMapper hikerMapper;
	/** The ItemTypeMapper instance for the region item type. */
	ItemTypeMapper regionMapper;
	/** The ItemTypeMapper instance for the range item type. */
	ItemTypeMapper rangeMapper;
	/** The ItemTypeMapper instance for the country item type. */
	ItemTypeMapper countryMapper;
	
	/** The ItemTypeMapper instances for all item types. */
	const QMap<PALItemType, ItemTypeMapper*> mappers;
	
	/** The base table for photos. */
	PhotosTable& photosTable;
	/** The base table for hiker participation in trips. */
	ParticipatedTable& participatedTable;
	
	/**
	 * Creates the ItemTypesHandler instance.
	 * 
	 * @param db			The database.
	 * @param uiPointerMap	A map of pointer supply structs for each item type.
	 */
	inline ItemTypesHandler(Database& db, QMap<PALItemType, TypeMapperPointers>&& uiPointerMap) :
		//										type				name		baseTable			compTable					(db, tableView									  )		pointerSupply							columnWidthsSetting								columnOrderSetting								hiddenColumnsSetting								sortingSetting								showFilterBarSetting								filtersSetting								dialogGeometrySetting				showStatsPanelSetting			statsPanelSplitterSizesSetting			openNewItemDialogAndStoreMethod	openDuplicateItemDialogAndStoreMethod	openEditItemDialogAndStoreMethod	openMultiEditItemsDialogAndStoreMethod,	openDeleteItemsDialogAndExecuteMethod
		ascentMapper	(ItemTypeMapper	(db,	ItemTypeAscent,		"ascent",	db.ascentsTable,	new CompositeAscentsTable	(db, uiPointerMap.value(ItemTypeAscent)	.tableView),	uiPointerMap.value(ItemTypeAscent),		db.projectSettings.columnWidths_ascentsTable,	db.projectSettings.columnOrder_ascentsTable,	db.projectSettings.hiddenColumns_ascentsTable,		db.projectSettings.sorting_ascentsTable,	db.projectSettings.showFilterBar_ascentsTable,		db.projectSettings.filters_ascentsTable,	Settings::ascentDialog_geometry,	Settings::ascentsStats_show,	Settings::ascentsStats_splitterSizes,	&openNewAscentDialogAndStore,	&openDuplicateAscentDialogAndStore,		&openEditAscentDialogAndStore,		&openMultiEditAscentsDialogAndStore,	&openDeleteAscentsDialogAndExecute)),
		peakMapper		(ItemTypeMapper	(db,	ItemTypePeak,		"peak",		db.peaksTable,		new CompositePeaksTable		(db, uiPointerMap.value(ItemTypePeak)	.tableView),	uiPointerMap.value(ItemTypePeak),		db.projectSettings.columnWidths_peaksTable,		db.projectSettings.columnOrder_peaksTable,		db.projectSettings.hiddenColumns_peaksTable,		db.projectSettings.sorting_peaksTable,		db.projectSettings.showFilterBar_peaksTable,		db.projectSettings.filters_peaksTable,		Settings::peakDialog_geometry,		Settings::peaksStats_show,		Settings::peaksStats_splitterSizes,		&openNewPeakDialogAndStore,		&openDuplicatePeakDialogAndStore,		&openEditPeakDialogAndStore,		&openMultiEditPeaksDialogAndStore,		&openDeletePeaksDialogAndExecute)),
		tripMapper		(ItemTypeMapper	(db,	ItemTypeTrip,		"trip",		db.tripsTable,		new CompositeTripsTable		(db, uiPointerMap.value(ItemTypeTrip)	.tableView),	uiPointerMap.value(ItemTypeTrip),		db.projectSettings.columnWidths_tripsTable,		db.projectSettings.columnOrder_tripsTable,		db.projectSettings.hiddenColumns_tripsTable,		db.projectSettings.sorting_tripsTable,		db.projectSettings.showFilterBar_tripsTable,		db.projectSettings.filters_tripsTable,		Settings::tripDialog_geometry,		Settings::tripsStats_show,		Settings::tripsStats_splitterSizes,		&openNewTripDialogAndStore,		nullptr /* no duplication */,			&openEditTripDialogAndStore,		&openMultiEditTripsDialogAndStore,		&openDeleteTripsDialogAndExecute)),
		hikerMapper		(ItemTypeMapper	(db,	ItemTypeHiker,		"hiker",	db.hikersTable,		new CompositeHikersTable	(db, uiPointerMap.value(ItemTypeHiker)	.tableView),	uiPointerMap.value(ItemTypeHiker),		db.projectSettings.columnWidths_hikersTable,	db.projectSettings.columnOrder_hikersTable,		db.projectSettings.hiddenColumns_hikersTable,		db.projectSettings.sorting_hikersTable,		db.projectSettings.showFilterBar_hikersTable,		db.projectSettings.filters_hikersTable,		Settings::hikerDialog_geometry,		Settings::hikersStats_show,		Settings::hikersStats_splitterSizes,	&openNewHikerDialogAndStore,	nullptr /* no duplication */,			&openEditHikerDialogAndStore,		&openMultiEditHikersDialogAndStore,		&openDeleteHikersDialogAndExecute)),
		regionMapper	(ItemTypeMapper	(db,	ItemTypeRegion,		"region",	db.regionsTable,	new CompositeRegionsTable	(db, uiPointerMap.value(ItemTypeRegion)	.tableView),	uiPointerMap.value(ItemTypeRegion),		db.projectSettings.columnWidths_regionsTable,	db.projectSettings.columnOrder_regionsTable,	db.projectSettings.hiddenColumns_regionsTable,		db.projectSettings.sorting_regionsTable,	db.projectSettings.showFilterBar_regionsTable,		db.projectSettings.filters_regionsTable,	Settings::regionDialog_geometry,	Settings::regionsStats_show,	Settings::regionsStats_splitterSizes,	&openNewRegionDialogAndStore,	nullptr /* no duplication */,			&openEditRegionDialogAndStore,		&openMultiEditRegionsDialogAndStore,	&openDeleteRegionsDialogAndExecute)),
		rangeMapper		(ItemTypeMapper	(db,	ItemTypeRange,		"range",	db.rangesTable,		new CompositeRangesTable	(db, uiPointerMap.value(ItemTypeRange)	.tableView),	uiPointerMap.value(ItemTypeRange),		db.projectSettings.columnWidths_rangesTable,	db.projectSettings.columnOrder_rangesTable,		db.projectSettings.hiddenColumns_rangesTable,		db.projectSettings.sorting_rangesTable,		db.projectSettings.showFilterBar_rangesTable,		db.projectSettings.filters_rangesTable,		Settings::rangeDialog_geometry,		Settings::rangesStats_show,		Settings::rangesStats_splitterSizes,	&openNewRangeDialogAndStore,	nullptr /* no duplication */,			&openEditRangeDialogAndStore,		&openMultiEditRangesDialogAndStore,		&openDeleteRangesDialogAndExecute)),
		countryMapper	(ItemTypeMapper	(db,	ItemTypeCountry,	"country",	db.countriesTable,	new CompositeCountriesTable	(db, uiPointerMap.value(ItemTypeCountry).tableView),	uiPointerMap.value(ItemTypeCountry),	db.projectSettings.columnWidths_countriesTable,	db.projectSettings.columnOrder_countriesTable,	db.projectSettings.hiddenColumns_countriesTable,	db.projectSettings.sorting_countriesTable,	db.projectSettings.showFilterBar_countriesTable,	db.projectSettings.filters_countriesTable,	Settings::countryDialog_geometry,	Settings::countriesStats_show,	Settings::countriesStats_splitterSizes,	&openNewCountryDialogAndStore,	nullptr /* no duplication */,			&openEditCountryDialogAndStore,		&openMultiEditCountriesDialogAndStore,	&openDeleteCountriesDialogAndExecute)),
		mappers({
			{ItemTypeAscent,	(&ascentMapper)},
			{ItemTypePeak,		(&peakMapper)},
			{ItemTypeTrip,		(&tripMapper)},
			{ItemTypeHiker,		(&hikerMapper)},
			{ItemTypeRegion,	(&regionMapper)},
			{ItemTypeRange,		(&rangeMapper)},
			{ItemTypeCountry,	(&countryMapper)}
		}),
		photosTable(db.photosTable),
		participatedTable(db.participatedTable)
	{}
	
	/**
	 * Destroys the ItemTypesHandler.
	 */
	inline ~ItemTypesHandler()
	{}
	
	
	/**
	 * Returns a complete list of ItemTypeMapper instances for every item type.
	 * 
	 * @return	A list of every ItemTypeMapper.
	 */
	inline const QList<ItemTypeMapper*> getAllMappers() const
	{
		QList<ItemTypeMapper*> list = QList<ItemTypeMapper*>();
		for (ItemTypeMapper* const mapper : mappers) {
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
	inline const ItemTypeMapper& get(PALItemType type) const
	{
		ItemTypeMapper* mapper = mappers.value(type);
		assert(mapper);
		return *mapper;
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
	 * Returns the ItemTypeMapper which belongs to the currently active tab in the main window, or
	 * nullptr if no tab is active.
	 * 
	 * @return	The ItemTypeMapper for the currently active tab, or nullptr if no tab is active.
	 */
	inline ItemTypeMapper* getActiveMapperOrNull() const
	{
		ItemTypeMapper* matchingMapper = nullptr;
		for (ItemTypeMapper* const mapper : mappers) {
			if (mapper->tab.isVisible()) {
				matchingMapper = mapper;
				break;
			}
		}
		return matchingMapper;
	}
	
	/**
	 * Returns the ItemTypeMapper which belongs to the currently active tab in the main window.
	 * 
	 * @pre An item tab must currently be active.
	 * 
	 * @return	The ItemTypeMapper for the currently active tab.
	 */
	inline ItemTypeMapper& getActiveMapper() const
	{
		ItemTypeMapper* mapper = getActiveMapperOrNull();
		assert(mapper);
		return *mapper;
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
