#include "ascent_filter_bar.h"

#include "main_window.h"
#include "src/dialogs/parse_helper.h"

#include <QCalendarWidget>



AscentFilterBar::AscentFilterBar(QWidget* parent) :
		QWidget(parent)
{
	setupUi(this);
	
	connectUI();
	
	setupUI();
	updateUI();
}



// INITIAL SETUP

void AscentFilterBar::supplyPointers(MainWindow* mainWindow, Database* db, CompositeAscentsTable* compAscents)
{
	this->mainWindow = mainWindow;
	this->db = db;
	this-> compAscents = compAscents;
}

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
	connect(dateFilterMinWidget,			&QDateEdit::dateChanged,			this,	&AscentFilterBar::handle_filtersChanged);
	connect(dateFilterMaxCheckbox,			&QCheckBox::stateChanged,			this,	&AscentFilterBar::handle_filtersChanged);
	connect(dateFilterMaxWidget,			&QDateEdit::dateChanged,			this,	&AscentFilterBar::handle_filtersChanged);
	connect(peakHeightFilterMinCombo,		&QComboBox::currentIndexChanged,	this,	&AscentFilterBar::handle_filtersChanged);
	connect(peakHeightFilterMaxCheckbox,	&QCheckBox::stateChanged,			this,	&AscentFilterBar::handle_filtersChanged);
	connect(peakHeightFilterMaxCombo,		&QComboBox::currentIndexChanged,	this,	&AscentFilterBar::handle_filtersChanged);
	connect(volcanoFilterYesRadio,			&QRadioButton::clicked,				this,	&AscentFilterBar::handle_filtersChanged);
	connect(volcanoFilterNoRadio,			&QRadioButton::clicked,				this,	&AscentFilterBar::handle_filtersChanged);
	connect(rangeFilterCombo,				&QComboBox::currentIndexChanged,	this,	&AscentFilterBar::handle_filtersChanged);
	connect(hikeKindFilterCombo,			&QComboBox::currentIndexChanged,	this,	&AscentFilterBar::handle_filtersChanged);
	connect(difficultyFilterSystemCombo,	&QComboBox::currentIndexChanged,	this,	&AscentFilterBar::handle_difficultyFilterSystemChanged);
	connect(difficultyFilterGradeCombo,		&QComboBox::currentIndexChanged,	this,	&AscentFilterBar::handle_filtersChanged);
	connect(hikerFilterCombo,				&QComboBox::currentIndexChanged,	this,	&AscentFilterBar::handle_filtersChanged);
}

void AscentFilterBar::setupUI()
{
	rangeFilterCombo->setModel(db->rangesTable);
	rangeFilterCombo->setRootModelIndex(db->rangesTable->getNullableRootModelIndex());
	rangeFilterCombo->setModelColumn(db->rangesTable->nameColumn->getIndex());
	
	hikeKindFilterCombo->insertItems(1, Ascent::hikeKindNames);
	
	QStringList difficultySystemNames = QStringList();
	std::transform(
			Ascent::difficultyNames.constBegin(),
			Ascent::difficultyNames.constEnd(),
			std::back_inserter(difficultySystemNames),
			[](QPair<QString, QStringList> qPair){ return qPair.first; }
	);
	difficultyFilterSystemCombo->insertItems(1, difficultySystemNames);
	handle_difficultyFilterSystemChanged();
	
	hikerFilterCombo->setModel(db->hikersTable);
	hikerFilterCombo->setRootModelIndex(db->hikersTable->getNullableRootModelIndex());
	hikerFilterCombo->setModelColumn(db->hikersTable->nameColumn->getIndex());
}



// PROJECT SETUP

void AscentFilterBar::insertFiltersIntoUI(QSet<Filter> filters)
{
	resetUI();
	
	for (const Filter& filter : filters) {
		const CompositeColumn* const column = filter.column;
		const QVariant value		= filter.value;
		const bool hasSecond		= filter.hasSecond;
		const QVariant secondValue	= filter.secondValue;
		
		const bool isInt	= value.canConvert<int>();
		const bool isBool	= value.canConvert<bool>();
		const bool isDate	= value.canConvert<QDate>();
		
		if (column == compAscents->dateColumn) {
			assert(isDate);
			QDate date = value.toDate();
			dateFilterMinWidget->setDate(date);
			if (hasSecond) date = secondValue.toDate();
			dateFilterMaxWidget->setDate(date);
			dateFilterMaxCheckbox->setChecked(hasSecond);	
			dateFilterBox->setChecked(true);
			continue;
		}
		
		if (column == compAscents->peakHeightColumn) {
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
			peakHeightFilterBox->setChecked(true);
			continue;
		}
		
		if (column == compAscents->volcanoColumn) {
			assert(isBool);
			bool boolValue = value.toBool();
			volcanoFilterYesRadio->setChecked(boolValue);
			volcanoFilterNoRadio->setChecked(!boolValue);
			volcanoFilterBox->setChecked(true);
			continue;
		}
		
		if (column == compAscents->rangeColumn) {
			assert(isInt);
			ValidItemID rangeID = value.toInt();
			int bufferRowIndex = db->rangesTable->getBufferIndexForPrimaryKey(rangeID);
			rangeFilterCombo->setCurrentIndex(bufferRowIndex);
			rangeFilterBox->setChecked(true);
			continue;
		}
		
		if (column == compAscents->hikeKindColumn) {
			assert(isInt);
			int intValue = value.toInt();
			hikeKindFilterCombo->setCurrentIndex(intValue);
			hikeKindFilterBox->setChecked(true);
			continue;
		}
		
		if (column == compAscents->difficultyColumn) {
			assert(isInt);
			if (hasSecond) {
				int system = value.toInt();
				int grade = secondValue.toInt();
				difficultyFilterSystemCombo->setCurrentIndex(system);
				difficultyFilterGradeCombo->setCurrentIndex(grade);
			}
			difficultyFilterBox->setChecked(true);
			continue;
		}
		
		if (column == compAscents->hikerIDsColumn) {
			assert(isInt);
			ValidItemID hikerID = value.toInt();
			int bufferRowIndex = db->hikersTable->getBufferIndexForPrimaryKey(hikerID);
			hikerFilterCombo->setCurrentIndex(bufferRowIndex);
			hikerFilterBox->setChecked(true);
			continue;
		}
		
		assert(false);
	}
	
	updateUI();
}



// UI UPDATES

void AscentFilterBar::updateUI()
{
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
	
	if (!dateFilterMaxCheckbox->checkState()) {
		dateFilterMaxWidget->setDate(dateFilterMinWidget->date());
	}
	else if (dateFilterMinWidget->date() > dateFilterMaxWidget->date()) {
		if (dateFilterMaxWidget->hasFocus() || dateFilterMaxWidget->calendarWidget()->hasFocus()) {
			dateFilterMinWidget->setDate(dateFilterMaxWidget->date());
		} else {
			dateFilterMaxWidget->setDate(dateFilterMinWidget->date());
		}
	}
	
	if (!peakHeightFilterMaxCheckbox->checkState()) {
		peakHeightFilterMaxCombo->setCurrentIndex(peakHeightFilterMinCombo->currentIndex());
	}
	else if (peakHeightFilterMinCombo->currentIndex() > peakHeightFilterMaxCombo->currentIndex()) {
		if (peakHeightFilterMaxCombo->hasFocus()) {
			peakHeightFilterMinCombo->setCurrentIndex(peakHeightFilterMaxCombo->currentIndex());
		} else {
			peakHeightFilterMaxCombo->setCurrentIndex(peakHeightFilterMinCombo->currentIndex());
		}
	}
	
	if (difficultyFilterBox->isChecked() && difficultyFilterSystemCombo->currentIndex() > 0) {
		difficultyFilterGradeCombo->setEnabled(true);
	}
}



// FILTER EVENT HANDLERS

void AscentFilterBar::handle_filtersChanged()
{
	updateUI();
}

void AscentFilterBar::handle_difficultyFilterBoxChanged()
{
	if (!difficultyFilterBox->isChecked()) return;
	
	int system = difficultyFilterSystemCombo->currentIndex();
	bool systemSelected = system > 0;
	difficultyFilterGradeCombo->setEnabled(systemSelected);
	
	updateUI();
}

void AscentFilterBar::handle_difficultyFilterSystemChanged()
{
	int system = difficultyFilterSystemCombo->currentIndex();
	bool systemSelected = system > 0;
	difficultyFilterGradeCombo->setEnabled(systemSelected && difficultyFilterBox->isChecked());
	
	difficultyFilterGradeCombo->clear();
	if (systemSelected) {
		difficultyFilterGradeCombo->setPlaceholderText(tr("Select grade"));
		difficultyFilterGradeCombo->insertItems(1, Ascent::difficultyNames.at(system).second);
	} else {
		difficultyFilterGradeCombo->setPlaceholderText(tr("None"));
	}
}

void AscentFilterBar::handle_applyFilters()
{
	applyFiltersButton->setEnabled(false);
	clearFiltersButton->setEnabled(true);
	
	compAscents->applyFilters(collectAndSaveFilters());
	
	mainWindow->updateTableSize();
}

void AscentFilterBar::handle_clearFilters()
{
	clearFiltersButton->setEnabled(false);
	updateUI();	// Potentially enable apply button
	
	compAscents->clearFilters();
	clearSavedFilters();
	
	mainWindow->updateTableSize();
}



// GENERAL HELPERS

QSet<Filter> AscentFilterBar::collectAndSaveFilters()
{
	QSet<Filter> filters = QSet<Filter>();
	
	if (dateFilterBox->isChecked()) {
		QDate minDate = dateFilterMinWidget->date();
		db->projectSettings->dateFilter->set(this, minDate);
		if (dateFilterMaxCheckbox->isChecked() && dateFilterMaxWidget->date() > minDate) {
			QDate maxDate = dateFilterMaxWidget->date();
			filters.insert(Filter(compAscents->dateColumn, minDate, maxDate));
			db->projectSettings->dateFilter->setSecond(this, maxDate);
		} else {
			filters.insert(Filter(compAscents->dateColumn, minDate));
			db->projectSettings->dateFilter->setSecondToNull(this);
		}
	} else {
		db->projectSettings->dateFilter->setBothToNull(this);
	}
	
	if (peakHeightFilterBox->isChecked()) {
		QString minHeightString = peakHeightFilterMinCombo->currentText();
		bool conversionOk = true;
		int minHeight = (minHeightString == "<1000") ? 0 : minHeightString.toInt(&conversionOk);
		assert(conversionOk);
		int maxHeight = peakHeightFilterMaxCombo->currentText().toInt(&conversionOk);
		assert(conversionOk);
		filters.insert(Filter(compAscents->peakHeightColumn, minHeight, maxHeight));
		db->projectSettings->peakHeightFilter->set(this, minHeight);
		db->projectSettings->peakHeightFilter->setSecond(this, maxHeight);
	} else {
		db->projectSettings->peakHeightFilter->setBothToNull(this);
	}
	
	if (volcanoFilterBox->isChecked()) {
		bool value = !volcanoFilterNoRadio->isChecked();
		filters.insert(Filter(compAscents->volcanoColumn, value));
		db->projectSettings->volcanoFilter->set(this, value);
		db->projectSettings->volcanoFilter->setSecondToNull(this);
	} else {
		db->projectSettings->volcanoFilter->setBothToNull(this);
	}
	
	if (rangeFilterBox->isChecked()) {
		ItemID rangeID = parseIDCombo(rangeFilterCombo);
		filters.insert(Filter(compAscents->rangeIDColumn, rangeID.asQVariant()));
		db->projectSettings->rangeFilter->set(this, rangeID.asQVariant());
		db->projectSettings->rangeFilter->setSecondToNull(this);
	} else {
		db->projectSettings->rangeFilter->setBothToNull(this);
	}
	
	if (hikeKindFilterBox->isChecked()) {
		int value = parseEnumCombo(hikeKindFilterCombo);
		filters.insert(Filter(compAscents->hikeKindColumn, value));
		db->projectSettings->hikeKindFilter->set(this, value);
		db->projectSettings->hikeKindFilter->setSecondToNull(this);
	} else {
		db->projectSettings->hikeKindFilter->setBothToNull(this);
	}
	
	if (difficultyFilterBox->isChecked()) {
		int system	= parseEnumCombo(difficultyFilterSystemCombo);
		int grade	= parseEnumCombo(difficultyFilterGradeCombo);
		if (system <= 0) {
			system = 0;
			grade = 0;
		}
		filters.insert(Filter(compAscents->difficultyColumn, system, grade));
		db->projectSettings->difficultyFilter->set(this, system);
		db->projectSettings->difficultyFilter->setSecond(this, grade);
	} else {
		db->projectSettings->difficultyFilter->setBothToNull(this);
	}
	
	if (hikerFilterBox->isChecked()) {
		ItemID hikerID = parseIDCombo(hikerFilterCombo);
		filters.insert(Filter(compAscents->hikerIDsColumn, hikerID.asQVariant()));
		db->projectSettings->hikerFilter->set(this, hikerID.asQVariant());
		db->projectSettings->hikerFilter->setSecondToNull(this);
	} else {
		db->projectSettings->hikerFilter->setBothToNull(this);
	}
	
	return filters;
}

void AscentFilterBar::clearSavedFilters()
{
	db->projectSettings->dateFilter			->setBothToNull(this);
	db->projectSettings->peakHeightFilter	->setBothToNull(this);
	db->projectSettings->volcanoFilter		->setBothToNull(this);
	db->projectSettings->rangeFilter			->setBothToNull(this);
	db->projectSettings->hikeKindFilter		->setBothToNull(this);
	db->projectSettings->difficultyFilter	->setBothToNull(this);
	db->projectSettings->hikerFilter			->setBothToNull(this);
}

void AscentFilterBar::resetUI()
{
	dateFilterBox		->setChecked(false);
	peakHeightFilterBox	->setChecked(false);
	volcanoFilterBox	->setChecked(false);
	rangeFilterBox		->setChecked(false);
	hikeKindFilterBox	->setChecked(false);
	difficultyFilterBox	->setChecked(false);
	hikerFilterBox		->setChecked(false);
	
	dateFilterMinWidget->setDate(QDate());
	dateFilterMaxWidget->setDate(QDate());
	dateFilterMaxCheckbox->setChecked(false);
	peakHeightFilterMinCombo->setCurrentIndex(0);
	peakHeightFilterMaxCombo->setCurrentIndex(0);
	peakHeightFilterMaxCombo->setCurrentIndex(0);
	peakHeightFilterMaxCheckbox->setChecked(false);
	volcanoFilterYesRadio->setChecked(true);
	volcanoFilterNoRadio->setChecked(false);
	rangeFilterCombo->setCurrentIndex(0);
	hikeKindFilterCombo->setCurrentIndex(0);
	difficultyFilterSystemCombo->setCurrentIndex(0);
	difficultyFilterGradeCombo->setCurrentIndex(0);
	hikerFilterCombo->setCurrentIndex(0);
}
