#include "ascent_filter_bar.h"

#include "main_window.h"
#include "src/dialogs/parse_helper.h"

#include <QCalendarWidget>



AscentFilterBar::AscentFilterBar(QWidget* parent) :
		QWidget(parent),
		mainWindow(nullptr),
		db(nullptr),
		compAscents(nullptr)
{
	setupUi(this);
	
	connectUI();
}

// INITIAL SETUP

void AscentFilterBar::supplyPointers(MainWindow* mainWindow, Database* db, CompositeAscentsTable* compAscents)
{
	this->mainWindow	= mainWindow;
	this->db			= db;
	this->compAscents	= compAscents;
	
	setupUI();
	resetUI();
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

void AscentFilterBar::setupUI()
{
	rangeFilterCombo->setModel(db->rangesTable);
	rangeFilterCombo->setRootModelIndex(db->rangesTable->getNullableRootModelIndex());
	rangeFilterCombo->setModelColumn(db->rangesTable->nameColumn->getIndex());
	
	hikeKindFilterCombo->insertItems(0, Ascent::hikeKindNames);
	
	QStringList difficultySystemNames = QStringList();
	std::transform(
			Ascent::difficultyNames.constBegin(),
			Ascent::difficultyNames.constEnd(),
			std::back_inserter(difficultySystemNames),
			[](QPair<QString, QStringList> qPair){ return qPair.first; }
	);
	difficultyFilterSystemCombo->insertItems(0, difficultySystemNames);
	handle_difficultyFilterSystemChanged();
	
	hikerFilterCombo->setModel(db->hikersTable);
	hikerFilterCombo->setRootModelIndex(db->hikersTable->getNullableRootModelIndex());
	hikerFilterCombo->setModelColumn(db->hikersTable->nameColumn->getIndex());
}



// PROJECT SETUP

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

void AscentFilterBar::insertFiltersIntoUI(QSet<Filter> filters)
{
	resetUI();
	
	if (filters.isEmpty()) return;
	
	for (const Filter& filter : filters) {
		const CompositeColumn* const column = filter.column;
		const QVariant value		= filter.value;
		const bool hasSecond		= filter.hasSecond;
		const QVariant secondValue	= filter.secondValue;
		
		const bool isInt	= value.canConvert<int>()	&& (!hasSecond || secondValue.canConvert<int>());
		const bool isBool	= value.canConvert<bool>()	&& (!hasSecond || secondValue.canConvert<bool>());
		const bool isDate	= value.canConvert<QDate>()	&& (!hasSecond || secondValue.canConvert<QDate>());
		
		if (column == compAscents->dateColumn) {
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
		
		if (column == compAscents->peakHeightColumn) {
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
		
		if (column == compAscents->volcanoColumn) {
			volcanoFilterBox->setChecked(true);
			assert(isBool);
			
			bool boolValue = value.toBool();
			volcanoFilterYesRadio->setChecked(boolValue);
			volcanoFilterNoRadio->setChecked(!boolValue);
			continue;
		}
		
		if (column == compAscents->rangeIDColumn) {
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
			int bufferRowIndex = db->rangesTable->getBufferIndexForPrimaryKey(rangeID.forceValid());
			rangeFilterCombo->setCurrentIndex(bufferRowIndex + 1);	// 0 is None
			continue;
		}
		
		if (column == compAscents->hikeKindColumn) {
			hikeKindFilterBox->setChecked(true);
			assert(isInt);
			
			int intValue = value.toInt();
			hikeKindFilterCombo->setCurrentIndex(intValue);
			continue;
		}
		
		if (column == compAscents->difficultyColumn) {
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
		
		if (column == compAscents->hikerIDsColumn) {
			hikerFilterBox->setChecked(true);
			
			if (!value.isValid()) {
				hikerFilterCombo->setCurrentIndex(0);
				continue;
			}
			assert(isInt);
			
			ItemID hikerID = value.toInt();
			if (hikerID.isInvalid()) {
				hikerFilterCombo->setCurrentIndex(0);
			} else {
				int bufferRowIndex = db->hikersTable->getBufferIndexForPrimaryKey(hikerID.forceValid());
				hikerFilterCombo->setCurrentIndex(bufferRowIndex + 1);	// 0 is None
			}
			continue;
		}
		
		assert(false);
	}
	
	handle_filtersChanged();
	applyFiltersButton->setEnabled(false);
	clearFiltersButton->setEnabled(true);
}



// UI CHANGE HANDLERS

void AscentFilterBar::handle_filtersChanged()
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
	
	if (difficultyFilterBox->isChecked() && difficultyFilterSystemCombo->currentIndex() > 0) {
		difficultyFilterGradeCombo->setEnabled(true);
	}
}


void AscentFilterBar::handle_difficultyFilterBoxChanged()
{
	if (!difficultyFilterBox->isChecked()) return;
	
	int system = difficultyFilterSystemCombo->currentIndex();
	difficultyFilterGradeCombo->setEnabled(system);
	
	handle_filtersChanged();
}


void AscentFilterBar::handle_minDateChanged()
{
	if (!dateFilterMaxCheckbox->checkState()) {
		dateFilterMaxWidget->setDate(dateFilterMinWidget->date());
	}
	else if (dateFilterMinWidget->date() > dateFilterMaxWidget->date()) {
		dateFilterMaxWidget->setDate(dateFilterMinWidget->date());
	}
	
	handle_filtersChanged();
}

void AscentFilterBar::handle_maxDateChanged()
{
	if (dateFilterMinWidget->date() > dateFilterMaxWidget->date()) {
		dateFilterMinWidget->setDate(dateFilterMaxWidget->date());
	}
	
	handle_filtersChanged();
}

void AscentFilterBar::handle_minHeightChanged()
{
	if (!peakHeightFilterMaxCheckbox->checkState()) {
		peakHeightFilterMaxCombo->setCurrentIndex(peakHeightFilterMinCombo->currentIndex());
	}
	else if (peakHeightFilterMinCombo->currentIndex() > peakHeightFilterMaxCombo->currentIndex()) {
		peakHeightFilterMaxCombo->setCurrentIndex(peakHeightFilterMinCombo->currentIndex());
	}
	
	handle_filtersChanged();
}

void AscentFilterBar::handle_maxHeightChanged()
{
	if (peakHeightFilterMinCombo->currentIndex() > peakHeightFilterMaxCombo->currentIndex()) {
		peakHeightFilterMinCombo->setCurrentIndex(peakHeightFilterMaxCombo->currentIndex());
	}
	
	handle_filtersChanged();
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



// EXECUTE FILTER ACTIONS

void AscentFilterBar::handle_applyFilters()
{
	applyFiltersButton->setEnabled(false);
	clearFiltersButton->setEnabled(true);
	
	QSet<Filter> filters = collectFilters();
	compAscents->applyFilters(filters);
	saveFilters(filters);
	
	mainWindow->updateTableSize();
}

void AscentFilterBar::handle_clearFilters()
{
	clearFiltersButton->setEnabled(false);
	handle_filtersChanged();	// Potentially enable apply button
	
	compAscents->clearFilters();
	clearSavedFilters();
	
	mainWindow->updateTableSize();
}



// PARSING FILTERS FROM UI

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
		ItemID rangeID = parseIDCombo(rangeFilterCombo);
		filters.insert(Filter(compAscents->rangeIDColumn, rangeID.asQVariant()));
	}
	
	if (hikeKindFilterBox->isChecked()) {
		int value = parseEnumCombo(hikeKindFilterCombo);
		filters.insert(Filter(compAscents->hikeKindColumn, value));
	}
	
	if (difficultyFilterBox->isChecked()) {
		int system	= parseEnumCombo(difficultyFilterSystemCombo);
		int grade	= parseEnumCombo(difficultyFilterGradeCombo);
		if (system <= 0) {
			system = 0;
			grade = 0;
		}
		filters.insert(Filter(compAscents->difficultyColumn, system, grade));
	}
	
	if (hikerFilterBox->isChecked()) {
		ItemID hikerID = parseIDCombo(hikerFilterCombo);
		filters.insert(Filter(compAscents->hikerIDsColumn, hikerID.asQVariant()));
	}
	
	return filters;
}



// SAVING FILTERS

void AscentFilterBar::clearSavedFilters()
{
	db->projectSettings->dateFilter			->setBothToNull(this);
	db->projectSettings->peakHeightFilter	->setBothToNull(this);
	db->projectSettings->volcanoFilter		->setBothToNull(this);
	db->projectSettings->rangeFilter		->setBothToNull(this);
	db->projectSettings->hikeKindFilter		->setBothToNull(this);
	db->projectSettings->difficultyFilter	->setBothToNull(this);
	db->projectSettings->hikerFilter		->setBothToNull(this);
}

void AscentFilterBar::saveFilters(const QSet<Filter> filters)
{
	clearSavedFilters();
	
	for (const Filter& filter : filters) {
		const CompositeColumn* column = filter.column;
		const QVariant value = filter.value;
		const bool hasSecond = filter.hasSecond;
		const QVariant secondValue = filter.secondValue;
		
		const bool isInt	= value.canConvert<int>()	&& (!hasSecond || secondValue.canConvert<int>());
		const bool isBool	= value.canConvert<bool>()	&& (!hasSecond || secondValue.canConvert<bool>());
		const bool isDate	= value.canConvert<QDate>()	&& (!hasSecond || secondValue.canConvert<QDate>());
		
		if (column == compAscents->dateColumn) {
			assert(isDate);
			db->projectSettings->dateFilter->set(this, value);
			if (hasSecond) db->projectSettings->dateFilter->setSecond(this, secondValue);
			continue;
		}
		
		if (column == compAscents->peakHeightColumn) {
			assert(isInt);
			db->projectSettings->peakHeightFilter->set(this, value);
			if (hasSecond) db->projectSettings->peakHeightFilter->setSecond(this, secondValue);
			continue;
		}
		
		if (column == compAscents->volcanoColumn) {
			assert(isBool);
			db->projectSettings->volcanoFilter->set(this, value);
			continue;
		}
		
		if (column == compAscents->rangeIDColumn) {
			ItemID rangeID = value.toInt();
			db->projectSettings->rangeFilter->set(this, rangeID.isValid() ? rangeID.asQVariant() : -1);
			continue;
		}
		
		if (column == compAscents->hikeKindColumn) {
			assert(isInt);
			db->projectSettings->hikeKindFilter->set(this, value);
			continue;
		}
		
		if (column == compAscents->difficultyColumn) {
			assert(isInt);
			db->projectSettings->difficultyFilter->set(this, value);
			db->projectSettings->difficultyFilter->setSecond(this, secondValue);
			continue;
		}
		
		if (column == compAscents->hikerIDsColumn) {
			ItemID hikerID = value.toInt();
			db->projectSettings->hikerFilter->set(this, hikerID.isValid() ? hikerID.asQVariant() : -1);
			continue;
		}
		
		assert(false);
	}
}



// RETRIEVING FILTERS FROM PROJECT SETTINGS

QSet<Filter> AscentFilterBar::parseFiltersFromProjectSettings()
{
	QSet<Filter> filters = QSet<Filter>();
	
	ProjectSettings* settings = db->projectSettings;
	
	if (settings->dateFilter->isNotNull()) {
		QDate date1 = settings->dateFilter->get();
		if (settings->dateFilter->secondIsNotNull()) {
			QDate date2 = settings->dateFilter->getSecond();
			filters.insert(Filter(compAscents->dateColumn, date1, date2));
		} else {
			filters.insert(Filter(compAscents->dateColumn, date1));
		}
	}
	
	if (settings->peakHeightFilter->isNotNull()) {
		int minHeight = settings->peakHeightFilter->get();
		assert(settings->peakHeightFilter->secondIsNotNull());
		int maxHeight = settings->peakHeightFilter->getSecond();
		filters.insert(Filter(compAscents->peakHeightColumn, minHeight, maxHeight));
	}
	
	if (settings->volcanoFilter->isNotNull()) {
		bool volcano = settings->volcanoFilter->get();
		filters.insert(Filter(compAscents->volcanoColumn, volcano));
	}
	
	if (settings->rangeFilter->isNotNull()) {
		ItemID rangeID = settings->rangeFilter->get();
		filters.insert(Filter(compAscents->rangeIDColumn, rangeID.asQVariant()));
	}
	
	if (settings->hikeKindFilter->isNotNull()) {
		int hikeKind = settings->hikeKindFilter->get();
		filters.insert(Filter(compAscents->hikeKindColumn, hikeKind));
	}
	
	if (settings->difficultyFilter->isNotNull()) {
		assert(settings->difficultyFilter->secondIsNotNull());
		int difficultySystem = settings->difficultyFilter->get();
		int difficultyGrade = settings->difficultyFilter->getSecond();
		filters.insert(Filter(compAscents->difficultyColumn, difficultySystem, difficultyGrade));
	}
	
	if (settings->hikerFilter->isNotNull()) {
		ItemID hikerID = settings->hikerFilter->get();
		filters.insert(Filter(compAscents->hikerIDsColumn, hikerID.asQVariant()));
	}
	
	return filters;
}