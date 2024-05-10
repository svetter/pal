#include "filter_wizard.h"

#include "src/comp_tables/fold_composite_column.h"
#include "src/filters/filter_widgets/filter_box.h"
#include "src/filters/bool_filter.h"
#include "src/filters/date_filter.h"
#include "src/filters/dual_enum_filter.h"
#include "src/filters/enum_filter.h"
#include "src/filters/id_filter.h"
#include "src/filters/int_filter.h"
#include "src/filters/string_filter.h"
#include "src/filters/time_filter.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>



FilterWizardColumnPage::FilterWizardColumnPage(QWidget* parent, const CompositeTable& tableToFilter, const QTableView& tableView) :
	QWizardPage(parent),
	tableToFilter(tableToFilter),
	tableView(tableView),
	columnListWidget(new QListWidget(this)),
	columnList(QList<const CompositeColumn*>())
{
	QVBoxLayout* const layout = new QVBoxLayout();
	setLayout(layout);
	setTitle(tr("Choose column"));
	setSubTitle(tr("Choose the column to use for the new filter."));
	
	columnListWidget->setAutoFillBackground(true);
	columnListWidget->setBackgroundRole(QPalette::Base);
	layout->addWidget(columnListWidget);
	
	connect(columnListWidget, &QListWidget::itemSelectionChanged, this, &FilterWizardColumnPage::completeChanged);
}



const CompositeColumn* FilterWizardColumnPage::getSelectedColumn() const
{
	if (columnListWidget->currentRow() < 0) return nullptr;
	if (columnList.isEmpty()) return nullptr;
	return columnList.at(columnListWidget->currentRow());
}



void FilterWizardColumnPage::initializePage()
{
	columnList.clear();
	columnListWidget->clear();
	
	for (int visualIndex = 0; visualIndex < tableToFilter.getNumberOfNormalColumns(); visualIndex++) {
		const int logicalIndex = tableView.horizontalHeader()->logicalIndex(visualIndex);
		const CompositeColumn& column = tableToFilter.getColumnAt(logicalIndex);
		const bool hidden = tableToFilter.isColumnHidden(column);
		
		QListWidgetItem* item = new QListWidgetItem(column.uiName, columnListWidget);
		if (hidden) {
			item->setForeground(QColorConstants::DarkGray);
			item->setToolTip(tr("This column is currently hidden"));
		}
		
		columnList.append(&column);
		columnListWidget->addItem(item);
	}
	columnListWidget->setCurrentRow(-1);
}

bool FilterWizardColumnPage::isComplete() const
{
	return columnListWidget->currentRow() >= 0;
}

int FilterWizardColumnPage::nextId() const
{
	const CompositeColumn* const column = getSelectedColumn();
	if (!column) return Page_Column;
	return FilterWizard::columnEligibleForProxyIDMode(*column) ? Page_Mode : Page_Settings;
}





FilterWizardModePage::FilterWizardModePage(QWidget* parent, const CompositeTable& tableToFilter, const FilterWizardColumnPage& columnPage) :
	QWizardPage(parent),
	tableToFilter(tableToFilter),
	columnPage(columnPage),
	filterIdentityRadio(new QRadioButton(this)),
	filterStringRadio(new QRadioButton(this)),
	previewBox(new QGroupBox(this)),
	previewLayout(new QHBoxLayout()),
	idFilterBox(nullptr),
	stringFilterBox(nullptr)
{
	QVBoxLayout* const layout = new QVBoxLayout();
	setLayout(layout);
	setTitle(tr("Choose filter mode"));
	setSubTitle(tr("The selected column can either be filtered for a single item selected from a complete list, or the list of items can be searched as a string."));
	
	layout->addWidget(filterIdentityRadio);
	layout->addWidget(filterStringRadio);
	
	layout->addSpacing(20);
	
	previewBox->setTitle(tr("Preview"));
	previewBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	previewLayout->setContentsMargins(5, 5, 10, 5);
	previewLayout->setSpacing(15);
	previewBox->setLayout(previewLayout);
	layout->addWidget(previewBox);
	
	connect(filterIdentityRadio,	&QRadioButton::clicked, this, &FilterWizardModePage::handle_radiosChanged);
	connect(filterStringRadio,		&QRadioButton::clicked, this, &FilterWizardModePage::handle_radiosChanged);
}



bool FilterWizardModePage::isProxyIDModeSelected() const
{
	const CompositeColumn* const column = columnPage.getSelectedColumn();
	assert(column);
	return FilterWizard::columnEligibleForProxyIDMode(*column) && filterIdentityRadio->isChecked();
}



void FilterWizardModePage::handle_radiosChanged()
{
	const bool identity	= filterIdentityRadio	->isChecked();
	const bool string	= filterStringRadio		->isChecked();
	idFilterBox		->setEnabled(!string);
	stringFilterBox	->setEnabled(!identity);
	
	Q_EMIT completeChanged();
}



void FilterWizardModePage::initializePage()
{
	const CompositeColumn* const compColumn = columnPage.getSelectedColumn();
	const Column* contentColumn = nullptr;
	if (compColumn->type == Reference) {
		contentColumn = &((ReferenceCompositeColumn*) compColumn)->contentColumn;
	} else if (compColumn->type == ListStringFold || compColumn->type == HikerListFold) {
		contentColumn = ((ListStringFoldCompositeColumn*) compColumn)->contentColumn;
	}
	assert(contentColumn);
	
	filterIdentityRadio->setText(tr("Filter by selectable single items from %1 table").arg(contentColumn->table.uiName));
	filterStringRadio->setText(tr("Filter by searching through textual list representing all items"));
	
	if (idFilterBox)		previewLayout->removeWidget(idFilterBox);
	if (stringFilterBox)	previewLayout->removeWidget(stringFilterBox);
	delete idFilterBox;
	delete stringFilterBox;
	IDFilter* const		idFilter		= new IDFilter		(tableToFilter, *compColumn, tr("Identity Filter"));
	StringFilter* const	stringFilter	= new StringFilter	(tableToFilter, *compColumn, tr("String Filter"));
	idFilterBox		= idFilter		->createFilterBox(this);
	stringFilterBox	= stringFilter	->createFilterBox(this);
	previewLayout->addWidget(idFilterBox);
	previewLayout->addWidget(stringFilterBox);
	
	handle_radiosChanged();
}

bool FilterWizardModePage::isComplete() const
{
	return filterIdentityRadio->isChecked() || filterStringRadio->isChecked();
}

int FilterWizardModePage::nextId() const
{
	return Page_Settings;
}





FilterWizardSettingsPage::FilterWizardSettingsPage(QWidget* parent, const CompositeTable& tableToFilter, const FilterWizardColumnPage& columnPage, const FilterWizardModePage& modePage) :
	QWizardPage(parent),
	tableToFilter(tableToFilter),
	columnPage(columnPage),
	modePage(modePage),
	nameEdit(new QLineEdit(this)),
	intSettingsHLine(new QFrame(this)),
	exactValueRadiobutton(new QRadioButton(this)),
	classesRadiobutton(new QRadioButton(this)),
	intClassesGroupBox(new QGroupBox(this)),
	intClassIncrementLabel(new QLabel(this)),
	intClassIncrementSpinner(new QSpinBox(this)),
	intClassMinLabel(new QLabel(this)),
	intClassMinSpinner(new QSpinBox(this)),
	intClassMaxLabel(new QLabel(this)),
	intClassMaxSpinner(new QSpinBox(this)),
	intClassPreview(new QListWidget(this))
{
	QVBoxLayout* const layout = new QVBoxLayout();
	setLayout(layout);
	setTitle(tr("Customize"));
	setSubTitle(tr("Choose a name for the new filter, and choose additional settings."));
	
	nameEdit->setPlaceholderText(tr("Name the new filter"));
	layout->addWidget(nameEdit);
	
	intSettingsHLine->setFrameShape(QFrame::HLine);
	intSettingsHLine->setFrameShadow(QFrame::Sunken);
	layout->addSpacing(10);
	layout->addWidget(intSettingsHLine);
	layout->addSpacing(10);
	
	exactValueRadiobutton->setText(tr("Filter by exact value"));
	layout->addWidget(exactValueRadiobutton);
	
	classesRadiobutton->setText(tr("Filter by classes:"));
	layout->addWidget(classesRadiobutton);
	
	QGridLayout* intClassesSpinnersLayout = new QGridLayout();
	QHBoxLayout* intClassesLayout = new QHBoxLayout();
	intClassesGroupBox->setLayout(intClassesLayout);
	
	// Increment label
	intClassIncrementLabel->setText(tr("Difference between steps:"));
	intClassesSpinnersLayout->addWidget(intClassIncrementLabel, 0, 0);
	// Increment widget
	intClassIncrementSpinner->setMinimum(1);
	intClassIncrementSpinner->setMaximum(1000000);
	intClassIncrementSpinner->setValue(1000);
	handle_intClassIncrementChanged();
	intClassesSpinnersLayout->addWidget(intClassIncrementSpinner, 0, 1);
	// Min label
	intClassMinLabel->setText(tr("Minimum of lowest class:"));
	intClassesSpinnersLayout->addWidget(intClassMinLabel, 1, 0);
	// Min widget
	intClassMinSpinner->setMinimum(0);
	intClassMinSpinner->setValue(0);
	handle_intClassMinChanged();
	intClassesSpinnersLayout->addWidget(intClassMinSpinner, 1, 1);
	// Max label
	intClassMaxLabel->setText(tr("Ceiling of highest class:"));
	intClassesSpinnersLayout->addWidget(intClassMaxLabel, 2, 0);
	// Max widget
	intClassMaxSpinner->setMaximum(9999999);
	intClassMaxSpinner->setValue(9000);
	handle_intClassMaxChanged();
	intClassesSpinnersLayout->addWidget(intClassMaxSpinner, 2, 1);
	
	intClassesLayout->addLayout(intClassesSpinnersLayout);
	// Preview
	QVBoxLayout* intClassPreviewLayout = new QVBoxLayout();
	intClassPreviewLayout->addWidget(new QLabel(tr("Preview of the classes:")));
	intClassPreview->setAutoFillBackground(true);
	intClassPreview->setBackgroundRole(QPalette::Base);
	intClassPreviewLayout->addWidget(intClassPreview);
	intClassesLayout->addLayout(intClassPreviewLayout);
	
	layout->addWidget(intClassesGroupBox);
	handle_intFilterModeSelectionChanged();
	
	
	connect(exactValueRadiobutton,		&QRadioButton::clicked,		this,	&FilterWizardSettingsPage::handle_intFilterModeSelectionChanged);
	connect(classesRadiobutton,			&QRadioButton::clicked,		this,	&FilterWizardSettingsPage::handle_intFilterModeSelectionChanged);
	connect(intClassIncrementSpinner,	&QSpinBox::valueChanged,	this,	&FilterWizardSettingsPage::handle_intClassIncrementChanged);
	connect(intClassMinSpinner,			&QSpinBox::valueChanged,	this,	&FilterWizardSettingsPage::handle_intClassMinChanged);
	connect(intClassMaxSpinner,			&QSpinBox::valueChanged,	this,	&FilterWizardSettingsPage::handle_intClassMaxChanged);
	
	connect(nameEdit,					&QLineEdit::textChanged,	this,	&FilterWizardSettingsPage::completeChanged);
	connect(exactValueRadiobutton,		&QRadioButton::clicked,		this,	&FilterWizardSettingsPage::completeChanged);
	connect(classesRadiobutton,			&QRadioButton::clicked,		this,	&FilterWizardSettingsPage::completeChanged);
}



QString FilterWizardSettingsPage::getName() const
{
	return nameEdit->text();
}

QList<int> FilterWizardSettingsPage::getIntSettings() const
{
	if (columnPage.getSelectedColumn()->contentType != Integer) return {};
	if (!classesRadiobutton->isChecked()) return {};
	
	return {
		intClassIncrementSpinner->value(),
		intClassMinSpinner->value(),
		intClassMaxSpinner->value()
	};
}



void FilterWizardSettingsPage::handle_intFilterModeSelectionChanged()
{
	const bool useExact		= exactValueRadiobutton->isChecked();
	intClassIncrementSpinner->setEnabled(!useExact);
	intClassMinSpinner		->setEnabled(!useExact);
	intClassMaxSpinner		->setEnabled(!useExact);
	intClassPreview			->setEnabled(!useExact);
}

void FilterWizardSettingsPage::handle_intClassIncrementChanged()
{
	intClassMinSpinner->setSingleStep(intClassIncrementSpinner->value());
	intClassMaxSpinner->setSingleStep(intClassIncrementSpinner->value());
	updateIntClassPreview();
}

void FilterWizardSettingsPage::handle_intClassMinChanged()
{
	intClassMaxSpinner->setMinimum(intClassMinSpinner->value());
	updateIntClassPreview();
}

void FilterWizardSettingsPage::handle_intClassMaxChanged()
{
	intClassMinSpinner->setMaximum(intClassMaxSpinner->value());
	updateIntClassPreview();
}

void FilterWizardSettingsPage::updateIntClassPreview()
{
	const int classIncrement	= intClassIncrementSpinner->value();
	const int classMinValue		= intClassMinSpinner->value();
	const int classMaxValue		= intClassMaxSpinner->value();
	
	intClassPreview->clear();
	for (int value = classMinValue; value < classMaxValue; value += classIncrement) {
		QString entry = QString::number(value) + " - " + QString::number(value + classIncrement - 1);
		intClassPreview->addItem(entry);
	}
}



void FilterWizardSettingsPage::initializePage()
{
	nameEdit->setText(columnPage.getSelectedColumn()->uiName);
	
	const bool showIntSettings = columnPage.getSelectedColumn()->contentType == Integer;
	intSettingsHLine		->setVisible(showIntSettings);
	exactValueRadiobutton	->setVisible(showIntSettings);
	classesRadiobutton		->setVisible(showIntSettings);
	intClassesGroupBox		->setVisible(showIntSettings);
	if (!showIntSettings) {
		// Qt bug currently prevents these commands from working
		exactValueRadiobutton	->setChecked(false);
		classesRadiobutton		->setChecked(false);
	}
}

bool FilterWizardSettingsPage::isComplete() const
{
	bool complete = true;
	complete &= !nameEdit->text().isEmpty();
	if (columnPage.getSelectedColumn()->contentType == Integer) {
		complete &= exactValueRadiobutton->isChecked() || classesRadiobutton->isChecked();
	}
	return complete;
}





FilterWizard::FilterWizard(QWidget* parent, const CompositeTable& tableToFilter, const QTableView& tableView) :
	QWizard(parent),
	tableToFilter(tableToFilter),
	columnPage(FilterWizardColumnPage(parent, tableToFilter, tableView)),
	modePage(FilterWizardModePage(parent, tableToFilter, columnPage)),
	settingsPage(FilterWizardSettingsPage(parent, tableToFilter, columnPage, modePage))
{
	setModal(true);
	setWizardStyle(QWizard::ModernStyle);
	setWindowTitle(tableToFilter.baseTable.getNewFilterString());
	setMinimumSize(500, 300);
	setSizeGripEnabled(false);
	
	setPage(Page_Column,	&columnPage);
	setPage(Page_Mode,		&modePage);
	setPage(Page_Settings,	&settingsPage);
	
	setStartId(Page_Column);
}

FilterWizard::~FilterWizard()
{}



Filter* FilterWizard::getFinishedFilter()
{
	const CompositeColumn* const columnToUse = columnPage.getSelectedColumn();
	const bool proxyIDMode = modePage.isProxyIDModeSelected();
	const QString name = settingsPage.getName();
	assert(columnToUse);
	assert(!name.isEmpty());
	
	const DataType type = proxyIDMode ? ID : columnToUse->contentType;
	switch (type) {
	case Integer: {
		QList<int> intSettings = settingsPage.getIntSettings();
		if (intSettings.size() == 3) {
			const int classIncrement	= intSettings.at(0);
			const int classMinValue		= intSettings.at(1);
			const int classMaxValue		= intSettings.at(2);
			return new IntFilter(tableToFilter, *columnToUse, name, classIncrement, classMinValue, classMaxValue);
		} else {
			return new IntFilter(tableToFilter, *columnToUse, name);
		}
	}
	case ID:		return new IDFilter			(tableToFilter, *columnToUse, name);
	case Enum:		return new EnumFilter		(tableToFilter, *columnToUse, name);
	case DualEnum:	return new DualEnumFilter	(tableToFilter, *columnToUse, name);
	case Bit:		return new BoolFilter		(tableToFilter, *columnToUse, name);
	case String:	return new StringFilter		(tableToFilter, *columnToUse, name);
	case Date:		return new DateFilter		(tableToFilter, *columnToUse, name);
	case Time:		return new TimeFilter		(tableToFilter, *columnToUse, name);
	default: assert(false);
	}
	return nullptr;
}



bool FilterWizard::columnEligibleForProxyIDMode(const CompositeColumn& column, bool* autoProxy)
{
	if (autoProxy) *autoProxy = false;
	
	const Column* contentColumn = nullptr;
	switch (column.type) {
	case Reference:
		contentColumn = &((ReferenceCompositeColumn&) column).contentColumn;
		break;
	case ListStringFold:
	case HikerListFold:
		contentColumn = ((ListStringFoldCompositeColumn&) column).contentColumn;
		assert(contentColumn);
		break;
	default: break;
	}
	if (!contentColumn) {
		return false;
	}
	
	if (contentColumn->primaryKey) {
		if (autoProxy) *autoProxy = true;
		return true;
	}
	
	assert(!contentColumn->table.isAssociative);
	const NormalTable& contentTable = (NormalTable&) contentColumn->table;
	const QList<const Column*> idRepColumns = contentTable.getIdentityRepresentationColumns();
	if (idRepColumns.contains(contentColumn)) {
		if (autoProxy) *autoProxy = idRepColumns.size() == 1 && idRepColumns.at(0) == contentColumn;
		return true;
	}
	
	return false;
}
