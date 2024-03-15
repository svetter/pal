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
 * @file ascent_filter_bar.cpp
 * 
 * This file implements the AscentFilterBar class.
 */

#include "ascent_filter_bar.h"

#include "main_window.h"
#include "src/dialogs/parse_helper.h"
#include "src/data/enum_names.h"

#include <QCalendarWidget>



/**
 * Creates a new ascent filter bar.
 * 
 * @param parent The parent widget.
 */
AscentFilterBar::AscentFilterBar(QWidget* parent) :
	QWidget(parent),
	mainWindow(nullptr),
	db(nullptr),
	compAscents(nullptr),
	selectableRangeIDs(QList<ValidItemID>()),
	selectableHikerIDs(QList<ValidItemID>()),
	temporarilyIgnoreChangeEvents(false)
{
	setupUi(this);
	
	applyFiltersButton->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
	clearFiltersButton->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
	filtersScrollArea->setBackgroundRole(QPalette::Base);
	
	
	connectUI();
}

// INITIAL SETUP

/**
 * Supplies pointers to the main window, the database and the ascent table.
 * 
 * This function is called only once from the MainWindow constructor.
 * It exists so that the AscentFilterBar can be placed in the UI form file, which means that its
 * constructor can't take any custom arguments like these pointers.
 * 
 * @param mainWindow	The main window.
 * @param db			The database.
 * @param compAscents	The ascent table.
 */
void AscentFilterBar::supplyPointers(MainWindow* mainWindow, Database* db, CompositeAscentsTable* compAscents)
{
	this->mainWindow	= mainWindow;
	this->db			= db;
	this->compAscents	= compAscents;
	
	additionalUISetup();
	resetUI();
}


/**
 * Connects interactive UI elements to event handler functions.
 */
void AscentFilterBar::connectUI()
{
	connect(applyFiltersButton,				&QPushButton::clicked,				this,	&AscentFilterBar::handle_applyFilters);
	connect(clearFiltersButton,				&QPushButton::clicked,				this,	&AscentFilterBar::handle_clearFilters);
	
	connect(dateFilterBox,					&QGroupBox::clicked,				this,	&AscentFilterBar::handle_filtersChanged);
	connect(peakHeightFilterBox,			&QGroupBox::clicked,				this,	&AscentFilterBar::handle_filtersChanged);
	connect(volcanoFilterBox,				&QGroupBox::clicked,				this,	&AscentFilterBar::handle_filtersChanged);
	connect(rangeFilterBox,					&QGroupBox::clicked,				this,	&AscentFilterBar::handle_filtersChanged);
	connect(hikeKindFilterBox,				&QGroupBox::clicked,				this,	&AscentFilterBar::handle_filtersChanged);
	connect(difficultyFilterBox,			&QGroupBox::clicked,				this,	&AscentFilterBar::handle_difficultyFilterBoxChanged);
	connect(hikerFilterBox,					&QGroupBox::clicked,				this,	&AscentFilterBar::handle_filtersChanged);
	
	connect(dateFilterMinWidget,			&QDateEdit::dateChanged,			this,	&AscentFilterBar::handle_minDateChanged);
	connect(dateFilterMaxCheckbox,			&QCheckBox::stateChanged,			this,	&AscentFilterBar::handle_filtersChanged);
	connect(dateFilterMaxWidget,			&QDateEdit::dateChanged,			this,	&AscentFilterBar::handle_maxDateChanged);
	connect(peakHeightFilterMinCombo,		&QComboBox::currentIndexChanged,	this,	&AscentFilterBar::handle_minHeightChanged);
	connect(peakHeightFilterMaxCheckbox,	&QCheckBox::stateChanged,			this,	&AscentFilterBar::handle_filtersChanged);
	connect(peakHeightFilterMaxCombo,		&QComboBox::currentIndexChanged,	this,	&AscentFilterBar::handle_maxHeightChanged);
	connect(volcanoFilterYesRadio,			&QRadioButton::clicked,				this,	&AscentFilterBar::handle_filtersChanged);
	connect(volcanoFilterNoRadio,			&QRadioButton::clicked,				this,	&AscentFilterBar::handle_filtersChanged);
	connect(rangeFilterCombo,				&QComboBox::currentIndexChanged,	this,	&AscentFilterBar::handle_filtersChanged);
	connect(hikeKindFilterCombo,			&QComboBox::currentIndexChanged,	this,	&AscentFilterBar::handle_filtersChanged);
	connect(difficultyFilterSystemCombo,	&QComboBox::currentIndexChanged,	this,	&AscentFilterBar::handle_difficultyFilterSystemChanged);
	connect(difficultyFilterGradeCombo,		&QComboBox::currentIndexChanged,	this,	&AscentFilterBar::handle_filtersChanged);
	connect(hikerFilterCombo,				&QComboBox::currentIndexChanged,	this,	&AscentFilterBar::handle_filtersChanged);
}

/**
 * Populates item combo boxes.
 * 
 * In this class, this function is not called from the constructor but when pointers to the main
 * window and the database are supplied via supplyPointers().
 */
void AscentFilterBar::additionalUISetup()
{
	hikeKindFilterCombo->insertItems(0, EnumNames::translateList(EnumNames::hikeKindNames));
	
	QStringList difficultySystemNames = QStringList();
	std::transform(
			EnumNames::difficultyNames.constBegin(),
			EnumNames::difficultyNames.constEnd(),
			std::back_inserter(difficultySystemNames),
			[](QPair<QString, QStringList> qPair){ return qPair.first; }
	);
	difficultyFilterSystemCombo->insertItems(0, EnumNames::translateList(difficultySystemNames));
	
	handle_difficultyFilterSystemChanged();
	
	// Make QGroupBox titles turn gray when disabled (like on other widgets)
	QColor disabledColor = QApplication::palette().color(QPalette::Disabled, QPalette::WindowText);
	QPalette disabledPalette = QPalette();
	disabledPalette.setColor(QPalette::Disabled, QPalette::WindowText, disabledColor);
	dateFilterBox		->setPalette(disabledPalette);
	peakHeightFilterBox	->setPalette(disabledPalette);
	volcanoFilterBox	->setPalette(disabledPalette);
	rangeFilterBox		->setPalette(disabledPalette);
	hikeKindFilterBox	->setPalette(disabledPalette);
	difficultyFilterBox	->setPalette(disabledPalette);
	hikerFilterBox		->setPalette(disabledPalette);
}



// PROJECT SETUP

/**
 * Resets all UI elements to their default state.
 * 
 * Project-specific combo boxes stay populated.
 */
void AscentFilterBar::resetUI()
{
	dateFilterBox		->setChecked(false);
	peakHeightFilterBox	->setChecked(false);
	volcanoFilterBox	->setChecked(false);
	rangeFilterBox		->setChecked(false);
	hikeKindFilterBox	->setChecked(false);
	difficultyFilterBox	->setChecked(false);
	hikerFilterBox		->setChecked(false);
	
	dateFilterMinWidget			->setDate(QDateTime::currentDateTime().date());
	dateFilterMaxWidget			->setDate(QDateTime::currentDateTime().date());
	dateFilterMaxCheckbox		->setChecked(false);
	peakHeightFilterMinCombo	->setCurrentIndex(0);
	peakHeightFilterMaxCombo	->setCurrentIndex(0);
	peakHeightFilterMaxCheckbox	->setChecked(false);
	volcanoFilterYesRadio		->setChecked(true);
	volcanoFilterNoRadio		->setChecked(false);
	rangeFilterCombo			->setCurrentIndex(0);
	hikeKindFilterCombo			->setCurrentIndex(0);
	difficultyFilterSystemCombo	->setCurrentIndex(0);
	difficultyFilterGradeCombo	->setCurrentIndex(0);
	hikerFilterCombo			->setCurrentIndex(0);
}

/**
 * Sets UI up to represent the given set of active filters.
 * 
 * @param filters	The set of active filters to represent in the UI.
 */
void AscentFilterBar::insertFiltersIntoUI(QSet<Filter> filters)
{
	resetUI();
	
	if (filters.isEmpty()) return;
	
	for (const Filter& filter : filters) {
		const CompositeColumn& column = filter.column;
		const QVariant value		= filter.value;
		const bool hasSecond		= filter.hasSecond;
		const QVariant secondValue	= filter.secondValue;
		
		const bool isInt	= value.canConvert<int>()	&& (!hasSecond || secondValue.canConvert<int>());
		const bool isBool	= value.canConvert<bool>()	&& (!hasSecond || secondValue.canConvert<bool>());
		const bool isDate	= value.canConvert<QDate>()	&& (!hasSecond || secondValue.canConvert<QDate>());
		
		if (&column == &compAscents->dateColumn) {
			dateFilterBox->setChecked(true);
			assert(isDate);
			
			dateFilterMaxCheckbox->setChecked(hasSecond);
			QDate date1 = value.toDate();
			if (hasSecond) {
				QDate date2 = secondValue.toDate();
				dateFilterMaxWidget->setDate(date2);
			} else {
				dateFilterMaxWidget->setDate(date1);
			}
			dateFilterMinWidget->setDate(date1);
			continue;
		}
		
		if (&column == &compAscents->peakHeightColumn) {
			peakHeightFilterBox->setChecked(true);
			assert(isInt);
			
			int minHeight = value.toInt();
			int minHeightComboIndex = minHeight == 0 ? 0 : peakHeightFilterMinCombo->findText(QString::number(minHeight));
			peakHeightFilterMinCombo->setCurrentIndex(minHeightComboIndex);
			if (hasSecond) {
				int maxHeight = secondValue.toInt();
				int maxHeightComboIndex = peakHeightFilterMaxCombo->findText(QString::number(maxHeight));
				peakHeightFilterMaxCombo->setCurrentIndex(maxHeightComboIndex);
				peakHeightFilterMaxCheckbox->setChecked(maxHeightComboIndex > minHeightComboIndex);
			} else {
				peakHeightFilterMaxCombo->setCurrentIndex(minHeightComboIndex);
				peakHeightFilterMaxCheckbox->setChecked(false);
			}
			continue;
		}
		
		if (&column == &compAscents->volcanoColumn) {
			volcanoFilterBox->setChecked(true);
			assert(isBool);
			
			bool boolValue = value.toBool();
			volcanoFilterYesRadio->setChecked(boolValue);
			volcanoFilterNoRadio->setChecked(!boolValue);
			continue;
		}
		
		if (&column == &compAscents->rangeIDColumn) {
			rangeFilterBox->setChecked(true);
			
			if (!value.isValid()) {
				rangeFilterCombo->setCurrentIndex(0);
				continue;
			}
			assert(isInt);
			
			ItemID rangeID = value.toInt();
			if (rangeID.isInvalid()) {
				rangeFilterCombo->setCurrentIndex(0);
				continue;
			}
			rangeFilterCombo->setCurrentIndex(selectableRangeIDs.indexOf(rangeID) + 1);	// 0 is None
			continue;
		}
		
		if (&column == &compAscents->hikeKindColumn) {
			hikeKindFilterBox->setChecked(true);
			assert(isInt);
			
			int intValue = value.toInt();
			hikeKindFilterCombo->setCurrentIndex(intValue);
			continue;
		}
		
		if (&column == &compAscents->difficultyColumn) {
			difficultyFilterBox->setChecked(true);
			assert(isInt);
			
			if (hasSecond) {
				int system = value.toInt();
				int grade = secondValue.toInt();
				difficultyFilterSystemCombo->setCurrentIndex(system);
				difficultyFilterGradeCombo->setCurrentIndex(grade);
			}
			continue;
		}
		
		if (&column == &compAscents->hikerIDsColumn) {
			hikerFilterBox->setChecked(true);
			
			if (!value.isValid()) {
				hikerFilterCombo->setCurrentIndex(0);
				continue;
			}
			assert(isInt);
			
			ItemID hikerID = value.toInt();
			if (hikerID.isInvalid()) {
				hikerFilterCombo->setCurrentIndex(0);
				continue;
			}
			hikerFilterCombo->setCurrentIndex(selectableHikerIDs.indexOf(hikerID) + 1);	// 0 is None
			continue;
		}
		
		assert(false);
	}
	
	handle_filtersChanged();
	applyFiltersButton->setEnabled(false);
	clearFiltersButton->setEnabled(true);
}



// UPDATE UI

/**
 * Repopulates range combo box, maintaining selection if possible.
 * 
 * By setting temporarilyIgnoreChangeEvents, the function makes sure that the combo box event
 * handlers which are normally called when the user changes the selection don't fire when called
 * from here. If the selection is different after repopulating, handle_filtersChanged() is called
 * manually.
 */
void AscentFilterBar::updateRangeCombo()
{
	temporarilyIgnoreChangeEvents = true;
	
	ItemID previouslySelectedRangeID = ItemID();
	int rangeComboIndex = rangeFilterCombo->currentIndex();
	if (rangeComboIndex > 0) {
		previouslySelectedRangeID = selectableRangeIDs.at(rangeComboIndex - 1);	// 0 is None
	}
	populateRangeCombo(*db, *rangeFilterCombo, selectableRangeIDs);
	
	int newRangeComboIndex = 0;
	ItemID newlySelectedRangeID = ItemID();
	if (selectableRangeIDs.contains(previouslySelectedRangeID)) {
		newRangeComboIndex = selectableRangeIDs.indexOf(previouslySelectedRangeID) + 1;	// 0 is None
		newlySelectedRangeID = previouslySelectedRangeID;
	}
	rangeFilterCombo->setCurrentIndex(newRangeComboIndex);
	
	temporarilyIgnoreChangeEvents = false;
	
	if (newlySelectedRangeID != previouslySelectedRangeID) {
		handle_filtersChanged();
	}
}

/**
 * Repopulates hiker combo box, maintaining selection if possible.
 * 
 * By setting temporarilyIgnoreChangeEvents, the function makes sure that the combo box event
 * handlers which are normally called when the user changes the selection don't fire when called
 * from here. If the selection is different after repopulating, handle_filtersChanged() is called
 * manually.
 */
void AscentFilterBar::updateHikerCombo()
{
	temporarilyIgnoreChangeEvents = true;
	
	ItemID previouslySelectedHikerID = ItemID();
	int hikerComboIndex = hikerFilterCombo->currentIndex();
	if (hikerComboIndex > 0) {
		previouslySelectedHikerID = selectableHikerIDs.at(hikerComboIndex - 1);	// 0 is None
	}
	populateHikerCombo(*db, *hikerFilterCombo, selectableHikerIDs);
	
	int newHikerComboIndex = 0;
	ItemID newlySelectedHikerID = ItemID();
	if (selectableHikerIDs.contains(previouslySelectedHikerID)) {
		newHikerComboIndex = selectableHikerIDs.indexOf(previouslySelectedHikerID) + 1;	// 0 is None
		newlySelectedHikerID = previouslySelectedHikerID;
	}
	hikerFilterCombo->setCurrentIndex(newHikerComboIndex);
	
	temporarilyIgnoreChangeEvents = false;
	
	if (newlySelectedHikerID != previouslySelectedHikerID) {
		handle_filtersChanged();
	}
}



// UI CHANGE HANDLERS

/**
 * Collective event handler for any changes to filter UI elements.
 * 
 * Sets 'Apply filters' button to enabled if any filter is enabled in the UI and to disabled
 * otherwise.
 * Sets 'Clear filters' button to enabled if any filters are currently applied to the table (Note
 * that this is completely independent of the filter UI).
 */
void AscentFilterBar::handle_filtersChanged()
{
	if (temporarilyIgnoreChangeEvents) return;
	
	bool anyFilterIsSet = dateFilterBox->isChecked()
			|| peakHeightFilterBox->isChecked()
			|| volcanoFilterBox->isChecked()
			|| rangeFilterBox->isChecked()
			|| hikeKindFilterBox->isChecked()
			|| difficultyFilterBox->isChecked()
			|| hikerFilterBox->isChecked();
	applyFiltersButton->setEnabled(anyFilterIsSet);
	
	bool tableCurrentlyFiltered = compAscents->filterIsActive();
	clearFiltersButton->setEnabled(tableCurrentlyFiltered);
	
	if (dateFilterBox->isChecked()) {
		dateFilterMaxWidget->setEnabled(dateFilterMaxCheckbox->checkState());
	}
	if (peakHeightFilterBox->isChecked()) {
		peakHeightFilterMaxCombo->setEnabled(peakHeightFilterMaxCheckbox->checkState());
	}
	
	if (difficultyFilterBox->isChecked() && difficultyFilterSystemCombo->currentIndex() > 0) {
		difficultyFilterGradeCombo->setEnabled(true);
	}
}


/**
 * Event handler for changes to the difficulty filter system combo box.
 * 
 * **Note**: This handler only handles the difficulty filter system combo box, **not** the
 * difficulty *system* combo box next to and functionally connected to it (that is handled in
 * handle_difficultyFilterSystemChanged()).
 */
void AscentFilterBar::handle_difficultyFilterBoxChanged()
{
	if (temporarilyIgnoreChangeEvents) return;
	
	if (difficultyFilterBox->isChecked()) {
		int system = difficultyFilterSystemCombo->currentIndex();
		difficultyFilterGradeCombo->setEnabled(system);
	}
	
	handle_filtersChanged();
}


/**
 * Event handler for changes to the minimum date widget.
 * 
 * Enforces that the minimum date is never later than the maximum date (if set).
 */
void AscentFilterBar::handle_minDateChanged()
{
	if (temporarilyIgnoreChangeEvents) return;
	
	if (!dateFilterMaxCheckbox->checkState()) {
		dateFilterMaxWidget->setDate(dateFilterMinWidget->date());
	}
	else if (dateFilterMinWidget->date() > dateFilterMaxWidget->date()) {
		dateFilterMaxWidget->setDate(dateFilterMinWidget->date());
	}
	
	handle_filtersChanged();
}

/**
 * Event handler for changes to the maximum date widget.
 * 
 * Enforces that the minimum date is never later than the maximum date.
 */
void AscentFilterBar::handle_maxDateChanged()
{
	if (temporarilyIgnoreChangeEvents) return;
	
	if (dateFilterMinWidget->date() > dateFilterMaxWidget->date()) {
		dateFilterMinWidget->setDate(dateFilterMaxWidget->date());
	}
	
	handle_filtersChanged();
}

/**
 * Event handler for changes to the minimum peak height combo box.
 * 
 * Enforces that the minimum peak height is never higher than the maximum peak height (if set).
 */
void AscentFilterBar::handle_minHeightChanged()
{
	if (temporarilyIgnoreChangeEvents) return;
	
	if (!peakHeightFilterMaxCheckbox->checkState()) {
		peakHeightFilterMaxCombo->setCurrentIndex(peakHeightFilterMinCombo->currentIndex());
	}
	else if (peakHeightFilterMinCombo->currentIndex() > peakHeightFilterMaxCombo->currentIndex()) {
		peakHeightFilterMaxCombo->setCurrentIndex(peakHeightFilterMinCombo->currentIndex());
	}
	
	handle_filtersChanged();
}

/**
 * Event handler for changes to the maximum peak height combo box.
 * 
 * Enforces that the minimum peak height is never higher than the maximum peak height.
 */
void AscentFilterBar::handle_maxHeightChanged()
{
	if (temporarilyIgnoreChangeEvents) return;
	
	if (peakHeightFilterMinCombo->currentIndex() > peakHeightFilterMaxCombo->currentIndex()) {
		peakHeightFilterMinCombo->setCurrentIndex(peakHeightFilterMaxCombo->currentIndex());
	}
	
	handle_filtersChanged();
}

/**
 * Event handler for changes to the difficulty filter system combo box.
 * 
 * Repopulates the difficulty grade combo box according to the selected difficulty system.
 */
void AscentFilterBar::handle_difficultyFilterSystemChanged()
{
	if (temporarilyIgnoreChangeEvents) return;
	
	int system = difficultyFilterSystemCombo->currentIndex();
	bool systemSelected = system > 0;
	difficultyFilterGradeCombo->setEnabled(systemSelected && difficultyFilterBox->isChecked());
	
	difficultyFilterGradeCombo->clear();
	if (systemSelected) {
		difficultyFilterGradeCombo->setPlaceholderText(tr("Select grade"));
		QStringList translatedList = EnumNames::translateList(EnumNames::difficultyNames.at(system).second);
		difficultyFilterGradeCombo->insertItems(1, translatedList);
	} else {
		difficultyFilterGradeCombo->setPlaceholderText(tr("None"));
	}
}



// EXECUTE FILTER ACTIONS

/**
 * Event handler for the 'Apply filters' button.
 * 
 * Updates enabled status of 'Apply filters' and 'Clear filters' buttons, collects, saves and
 * applies the filters specified in the UI, and prompts the main window to update the table size
 * information.
 */
void AscentFilterBar::handle_applyFilters()
{
	applyFiltersButton->setEnabled(false);
	clearFiltersButton->setEnabled(true);
	
	QSet<Filter> filters = collectFilters();
	compAscents->applyFilters(filters);
	saveFilters(filters);
	
	mainWindow->currentFiltersChanged();
}

/**
 * Event handler for the 'Clear filters' button.
 * 
 * Sets the 'Clear filters' button to disabled, and performs the steps necessary to clear the
 * filters from the table and update the table size information.
 */
void AscentFilterBar::handle_clearFilters()
{
	compAscents->clearFilters();
	clearSavedFilters();
	
	handle_filtersChanged();	// Potentially enable apply button
	
	mainWindow->currentFiltersChanged();
}



// PARSING FILTERS FROM UI

/**
 * Assembles the set of active filters specified in the UI.
 * 
 * Any filter whose checkbox is enabled is included in the set, and vice versa.
 * 
 * @return	A set representing the active filters currently specified in the UI.
 */
QSet<Filter> AscentFilterBar::collectFilters()
{
	QSet<Filter> filters = QSet<Filter>();
	
	if (dateFilterBox->isChecked()) {
		QDate minDate = dateFilterMinWidget->date();
		if (dateFilterMaxCheckbox->isChecked() && dateFilterMaxWidget->date() > minDate) {
			QDate maxDate = dateFilterMaxWidget->date();
			filters.insert(Filter(compAscents->dateColumn, minDate, maxDate));
		} else {
			filters.insert(Filter(compAscents->dateColumn, minDate));
		}
	}
	
	if (peakHeightFilterBox->isChecked()) {
		QString minHeightString = peakHeightFilterMinCombo->currentText();
		bool conversionOk = true;
		int minHeight = (minHeightString == "<1000") ? 0 : minHeightString.toInt(&conversionOk);
		assert(conversionOk);
		int maxHeight = peakHeightFilterMaxCombo->currentText().toInt(&conversionOk);
		assert(conversionOk);
		filters.insert(Filter(compAscents->peakHeightColumn, minHeight, maxHeight));
	}
	
	if (volcanoFilterBox->isChecked()) {
		bool value = !volcanoFilterNoRadio->isChecked();
		filters.insert(Filter(compAscents->volcanoColumn, value));
	}
	
	if (rangeFilterBox->isChecked()) {
		ItemID rangeID = parseItemCombo(*rangeFilterCombo, selectableRangeIDs);
		filters.insert(Filter(compAscents->rangeIDColumn, rangeID.asQVariant()));
	}
	
	if (hikeKindFilterBox->isChecked()) {
		int value = parseEnumCombo(*hikeKindFilterCombo, true);
		filters.insert(Filter(compAscents->hikeKindColumn, value));
	}
	
	if (difficultyFilterBox->isChecked()) {
		int system	= parseEnumCombo(*difficultyFilterSystemCombo, true);
		int grade	= parseEnumCombo(*difficultyFilterGradeCombo, true);
		if (system <= 0) {
			system = 0;
			grade = 0;
		}
		filters.insert(Filter(compAscents->difficultyColumn, system, grade));
	}
	
	if (hikerFilterBox->isChecked()) {
		ItemID hikerID = parseItemCombo(*hikerFilterCombo, selectableHikerIDs);
		filters.insert(Filter(compAscents->hikerIDsColumn, hikerID.asQVariant()));
	}
	
	return filters;
}



// SAVING FILTERS

/**
 * Clears all filters previously saved in the project settings.
 */
void AscentFilterBar::clearSavedFilters()
{
	db->projectSettings.ascentFilters_date				.clear(*this);
	db->projectSettings.ascentFilters_maxDate			.clear(*this);
	db->projectSettings.ascentFilters_peakHeight		.clear(*this);
	db->projectSettings.ascentFilters_maxPeakHeight		.clear(*this);
	db->projectSettings.ascentFilters_volcano			.clear(*this);
	db->projectSettings.ascentFilters_range				.clear(*this);
	db->projectSettings.ascentFilters_hikeKind			.clear(*this);
	db->projectSettings.ascentFilters_difficultySystem	.clear(*this);
	db->projectSettings.ascentFilters_difficultyGrade	.clear(*this);
	db->projectSettings.ascentFilters_hiker				.clear(*this);
}

/**
 * Saves the given set of filters to the project settings.
 * 
 * Any filter not included in the set will be cleared from the project settings.
 * 
 * @param filters	The set of filters to save.
 */
void AscentFilterBar::saveFilters(const QSet<Filter> filters)
{
	clearSavedFilters();
	
	for (const Filter& filter : filters) {
		const CompositeColumn& column = filter.column;
		const QVariant value = filter.value;
		const bool hasSecond = filter.hasSecond;
		const QVariant secondValue = filter.secondValue;
		
		const bool isInt	= value.canConvert<int>()	&& (!hasSecond || secondValue.canConvert<int>());
		const bool isBool	= value.canConvert<bool>()	&& (!hasSecond || secondValue.canConvert<bool>());
		const bool isDate	= value.canConvert<QDate>()	&& (!hasSecond || secondValue.canConvert<QDate>());
		
		if (&column == &compAscents->dateColumn) {
			assert(isDate);
			db->projectSettings.ascentFilters_date.set(*this, value);
			if (hasSecond) db->projectSettings.ascentFilters_maxDate.set(*this, secondValue);
			continue;
		}
		
		if (&column == &compAscents->peakHeightColumn) {
			assert(isInt);
			db->projectSettings.ascentFilters_peakHeight.set(*this, value);
			if (hasSecond) db->projectSettings.ascentFilters_maxPeakHeight.set(*this, secondValue);
			continue;
		}
		
		if (&column == &compAscents->volcanoColumn) {
			assert(isBool);
			db->projectSettings.ascentFilters_volcano.set(*this, value);
			continue;
		}
		
		if (&column == &compAscents->rangeIDColumn) {
			ItemID rangeID = value.toInt();
			db->projectSettings.ascentFilters_range.set(*this, rangeID.isValid() ? rangeID.asQVariant() : -1);
			continue;
		}
		
		if (&column == &compAscents->hikeKindColumn) {
			assert(isInt);
			db->projectSettings.ascentFilters_hikeKind.set(*this, value);
			continue;
		}
		
		if (&column == &compAscents->difficultyColumn) {
			assert(isInt);
			db->projectSettings.ascentFilters_difficultySystem.set(*this, value);
			db->projectSettings.ascentFilters_difficultyGrade.set(*this, secondValue);
			continue;
		}
		
		if (&column == &compAscents->hikerIDsColumn) {
			ItemID hikerID = value.toInt();
			db->projectSettings.ascentFilters_hiker.set(*this, hikerID.isValid() ? hikerID.asQVariant() : -1);
			continue;
		}
		
		assert(false);
	}
}



// RETRIEVING FILTERS FROM PROJECT SETTINGS

/**
 * Retrieves the set of filters saved in the project settings.
 * 
 * Filters not set in the project settings will not be included in the set.
 * 
 * @return	A set of filters representing the ones saved in the project settings.
 */
QSet<Filter> AscentFilterBar::parseFiltersFromProjectSettings()
{
	QSet<Filter> filters = QSet<Filter>();
	
	ProjectSettings& settings = db->projectSettings;
	
	if (settings.ascentFilters_date.present()) {
		QDate date1 = settings.ascentFilters_date.get();
		if (settings.ascentFilters_maxDate.present()) {
			QDate date2 = settings.ascentFilters_maxDate.get();
			filters.insert(Filter(compAscents->dateColumn, date1, date2));
		} else {
			filters.insert(Filter(compAscents->dateColumn, date1));
		}
	}
	
	if (settings.ascentFilters_peakHeight.present()) {
		int minHeight = settings.ascentFilters_peakHeight.get();
		assert(settings.ascentFilters_maxPeakHeight.present());
		int maxHeight = settings.ascentFilters_maxPeakHeight.get();
		filters.insert(Filter(compAscents->peakHeightColumn, minHeight, maxHeight));
	}
	
	if (settings.ascentFilters_volcano.present()) {
		bool volcano = settings.ascentFilters_volcano.get();
		filters.insert(Filter(compAscents->volcanoColumn, volcano));
	}
	
	if (settings.ascentFilters_range.present()) {
		ItemID rangeID = settings.ascentFilters_range.get();
		filters.insert(Filter(compAscents->rangeIDColumn, rangeID.asQVariant()));
	}
	
	if (settings.ascentFilters_hikeKind.present()) {
		int hikeKind = settings.ascentFilters_hikeKind.get();
		filters.insert(Filter(compAscents->hikeKindColumn, hikeKind));
	}
	
	if (settings.ascentFilters_difficultySystem.present()) {
		assert(settings.ascentFilters_difficultyGrade.present());
		int difficultySystem = settings.ascentFilters_difficultySystem.get();
		int difficultyGrade = settings.ascentFilters_difficultyGrade.get();
		filters.insert(Filter(compAscents->difficultyColumn, difficultySystem, difficultyGrade));
	}
	
	if (settings.ascentFilters_hiker.present()) {
		ItemID hikerID = settings.ascentFilters_hiker.get();
		filters.insert(Filter(compAscents->hikerIDsColumn, hikerID.asQVariant()));
	}
	
	return filters;
}
