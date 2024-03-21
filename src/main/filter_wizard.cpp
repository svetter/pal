#include "filter_wizard.h"
#include "src/filters/bool_filter.h"
#include "src/filters/date_filter.h"
#include "src/filters/dual_enum_filter.h"
#include "src/filters/enum_filter.h"
#include "src/filters/id_filter.h"
#include "src/filters/int_filter.h"
#include "src/filters/string_filter.h"
#include "src/filters/time_filter.h"

#include <QVBoxLayout>

using std::make_unique;



FilterWizardTablePage::FilterWizardTablePage(QWidget* parent, const NormalTable& tableToFilter) :
	QWizardPage(parent),
	tableToFilter(tableToFilter),
	sameTableRadiobutton(new QRadioButton(this)),
	otherTableRadiobutton(new QRadioButton(this)),
	otherTableCombo(new QComboBox(this)),
	otherTableList(QList<const NormalTable*>())
{
	QVBoxLayout* const layout = new QVBoxLayout();
	setLayout(layout);
	setTitle(tr("Choose table"));
	setSubTitle(tr("Choose the table which contains the values the new filter will work on."));
	
	sameTableRadiobutton->setText(tr("Use a value from the same table (%1)").arg(tableToFilter.uiName));
	layout->addWidget(sameTableRadiobutton);
	layout->addWidget(new QLabel(tr("or"), this));
	otherTableRadiobutton->setText(tr("Use a value from a different table:"));
	layout->addWidget(otherTableRadiobutton);
	otherTableCombo->setEnabled(false);
	otherTableCombo->setPlaceholderText(tr("Choose table"));
	for (const NormalTable* const table: tableToFilter.db.getNormalItemTableList()) {
		if (table == &tableToFilter) continue;
		
		const Breadcrumbs& crumbs = tableToFilter.db.getBreadcrumbsFor(tableToFilter, *table);
		const bool useSingular = crumbs.isForwardOnly();
		QString comboEntry = QString();
		if (useSingular) {
			comboEntry = table->getItemNameSingular();
		} else {
			comboEntry = table->uiName;
		}
		
		otherTableList.append(table);
		otherTableCombo->addItem(comboEntry);
	}
	layout->addWidget(otherTableCombo);
	
	registerField("table.same*",		sameTableRadiobutton);
	registerField("table.other*",		otherTableRadiobutton);
	registerField("table.otherCombo*",	otherTableCombo);
	
	
	connect(otherTableRadiobutton,	&QRadioButton::toggled,	otherTableCombo,	&QComboBox::setEnabled);
}

const NormalTable* FilterWizardTablePage::getSelectedTable() const
{
	if (sameTableRadiobutton->isChecked()) {
		return &tableToFilter;
	} else if (otherTableRadiobutton->isChecked()) {
		if (otherTableCombo->currentIndex() < 0) return nullptr;
		return otherTableList.at(otherTableCombo->currentIndex());
	} else {
		return nullptr;
	}
}

bool FilterWizardTablePage::isComplete() const
{
	if (sameTableRadiobutton->isChecked()) return true;
	if (!otherTableRadiobutton->isChecked()) return false;
	return otherTableCombo->currentIndex() != -1;
}

int FilterWizardTablePage::nextId() const
{
	return Page_Column;
}





FilterWizardColumnPage::FilterWizardColumnPage(QWidget* parent, const NormalTable& tableToFilter, const FilterWizardTablePage& tablePage) :
	QWizardPage(parent),
	tableToFilter(tableToFilter),
	tablePage(tablePage),
	columnCombo(new QComboBox(this)),
	columnList(QList<const Column*>())
{
	QVBoxLayout* const layout = new QVBoxLayout();
	setLayout(layout);
	setTitle(tr("Choose column"));
	setSubTitle(tr("Choose the column which contains the values the new filter will work on."));
	
	layout->addWidget(columnCombo);
	
	registerField("column.combo*",	columnCombo);
}

const Column* FilterWizardColumnPage::getSelectedColumn() const
{
	if (columnCombo->currentIndex() < 0) return nullptr;
	return columnList.at(columnCombo->currentIndex());
}

void FilterWizardColumnPage::initializePage()
{
	const NormalTable* const tableToUse = tablePage.getSelectedTable();
	assert(tableToUse);
	
	columnList.clear();
	columnCombo->clear();
	const QList<const Column*> allColumns = tableToUse->getColumnList();
	for (int i = 0; i < allColumns.size(); ++i) {
		const Column* column = allColumns.at(i);
		if (column->primaryKey) {
			continue;
		}
		else if (column->foreignColumn) {
			columnList.append(column);
			columnCombo->addItem(column->getReferencedForeignColumn().table.uiName);
		}
		else if (column->type == DualEnum) {
			const Column& firstDualEnumColumn = *column;
			assert(i + 1 < allColumns.size());
			columnList.append(column);
			column = allColumns.at(++i);
			assert(column->type == DualEnum);
			columnCombo->addItem(firstDualEnumColumn.uiName + "/" + column->uiName);
		}
		else {
			columnList.append(column);
			columnCombo->addItem(column->uiName);
		}
	}
}

int FilterWizardColumnPage::nextId() const
{
	const NormalTable* const tableToUse = tablePage.getSelectedTable();
	const Column* const columnToUse = getSelectedColumn();
	assert(columnToUse);
	
	const Breadcrumbs& crumbs = tableToFilter.db.getBreadcrumbsFor(tableToFilter, *tableToUse);
	const bool forwardReference = crumbs.isForwardOnly();
	if (!forwardReference) {
		return Page_FoldOp;
	}
	else if (columnToUse->type == Integer) {
		return Page_NumberPrefs;
	}
	else {
		return Page_Name;
	}
}





FilterWizardFoldOpPage::FilterWizardFoldOpPage(QWidget* parent, const NormalTable& tableToFilter, const FilterWizardColumnPage& columnPage) :
	QWizardPage(parent),
	tableToFilter(tableToFilter),
	columnPage(columnPage),
	explainLabel(new QLabel(this)),
	foldOpCombo(new QComboBox(this)),
	foldOpList(QList<FilterFoldOp>())
{
	QVBoxLayout* const layout = new QVBoxLayout();
	setLayout(layout);
	setTitle(tr("Choose fold operation"));
	setSubTitle(tr("Choose a way to combine multiple values into one for filtering."));
	
	explainLabel->setWordWrap(true);
	layout->addWidget(explainLabel);
	
	layout->addWidget(foldOpCombo);
	
	registerField("foldOp.combo*",	foldOpCombo);
}

FilterFoldOp FilterWizardFoldOpPage::getSelectedFoldOp() const
{
	if (foldOpCombo->currentIndex() < 0) return FilterFoldOp(-1);
	return foldOpList.at(foldOpCombo->currentIndex());
}

QString FilterWizardFoldOpPage::getSelectedFoldOpName() const
{
	if (foldOpCombo->currentIndex() < 0) return QString();
	return foldOpCombo->currentText();
}

void FilterWizardFoldOpPage::initializePage()
{
	const Column* const columnToUse = columnPage.getSelectedColumn();
	assert(columnToUse);
	
	if (columnToUse->type == Integer) {
		explainLabel->setText(tr("The column you chose can contain multiple values for each row in the filtered table.\nA filter can be applied to a list of those values, their count, or on their maximum, minimum, sum, or average."));
	} else {
		explainLabel->setText(tr("The column you chose can contain multiple values for each row in the filtered table.\nA filter can be applied to a list of those values, or on their count."));
	}
	
	foldOpList.clear();
	foldOpCombo->clear();
	
	foldOpList.append(FilterFoldOp_StringList);	foldOpCombo->addItem(tr("List"));
	foldOpList.append(FilterFoldOp_Count);		foldOpCombo->addItem(tr("Count"));
	if (columnToUse->type == Integer) {
		foldOpList.append(FilterFoldOp_Max);		foldOpCombo->addItem(tr("Maximum"));
		foldOpList.append(FilterFoldOp_Min);		foldOpCombo->addItem(tr("Minimum"));
		foldOpList.append(FilterFoldOp_Sum);		foldOpCombo->addItem(tr("Sum"));
		foldOpList.append(FilterFoldOp_Average);	foldOpCombo->addItem(tr("Average"));
	}
}

int FilterWizardFoldOpPage::nextId() const
{
	const FilterFoldOp selectedFoldOp = getSelectedFoldOp();
	assert(selectedFoldOp != FilterFoldOp(-1));
	
	if (selectedFoldOp == FilterFoldOp_StringList) {
		return Page_Name;
	} else {
		return Page_NumberPrefs;
	}
}





FilterWizardNumberPrefPage::FilterWizardNumberPrefPage(QWidget* parent) :
	QWizardPage(parent),
	exactValueRadiobutton(new QRadioButton(this)),
	classesRadiobutton(new QRadioButton(this))
{
	QVBoxLayout* const layout = new QVBoxLayout();
	setLayout(layout);
	setTitle(tr("Set number filter preferences"));
	setSubTitle(tr("Choose how to filter numbers."));
	
	exactValueRadiobutton->setText(tr("Filter by exact value"));
	layout->addWidget(exactValueRadiobutton);
	
	classesRadiobutton->setText(tr("Filter by 1000s classes (1000-1999, 2000-2999, etc.)"));
	layout->addWidget(classesRadiobutton);
	
	registerField("numberPref.exact*",	exactValueRadiobutton);
	registerField("numberPref.class*",	classesRadiobutton);
}

bool FilterWizardNumberPrefPage::isComplete() const
{
	return exactValueRadiobutton->isChecked() || classesRadiobutton->isChecked();
}

int FilterWizardNumberPrefPage::nextId() const
{
	return Page_Name;
}





FilterWizardNamePage::FilterWizardNamePage(QWidget* parent, const NormalTable& tableToFilter, const FilterWizardTablePage& tablePage, const FilterWizardColumnPage& columnPage, const FilterWizardFoldOpPage& foldOpPage) :
	QWizardPage(parent),
	tableToFilter(tableToFilter),
	tablePage(tablePage),
	columnPage(columnPage),
	foldOpPage(foldOpPage),
	name(new QLineEdit(this))
{
	QVBoxLayout* const layout = new QVBoxLayout();
	setLayout(layout);
	setTitle(tr("Set filter name"));
	setSubTitle(tr("Choose a name for the new filter, or leave the automatic name in place."));
	
	name->setPlaceholderText(tr("Name the new filter"));
	layout->addWidget(name);
	
	registerField("name*",	name);
}

QString FilterWizardNamePage::getName() const
{
	return name->text();
}

void FilterWizardNamePage::initializePage()
{
	name->setText(generateFilterName());
}

bool FilterWizardNamePage::isComplete() const
{
	return !name->text().isEmpty();
}

QString FilterWizardNamePage::generateFilterName() const
{
	const NormalTable* const tableToUse = tablePage.getSelectedTable();
	const Column* const columnToUse = columnPage.getSelectedColumn();
	const FilterFoldOp foldOp = foldOpPage.getSelectedFoldOp();
	assert(tableToUse);
	assert(columnToUse);
	
	QString name = "";
	if (tableToUse != &tableToFilter) {
		const Breadcrumbs& crumbs = tableToFilter.db.getBreadcrumbsFor(tableToFilter, *tableToUse);
		const bool useSingular = crumbs.isForwardOnly();
		if (useSingular) {
			name += tableToUse->getItemNameSingular();
		} else {
			name += tableToUse->uiName;
		}
		name += ": ";
	}
	
	if (columnToUse->foreignColumn) {
		const Table& foreignTable = columnToUse->getReferencedForeignColumn().table;
		assert(!foreignTable.isAssociative);
		const NormalTable& targetTable = (const NormalTable&) foreignTable;
		name = targetTable.getItemNameSingular();
	} else {
		name += columnToUse->uiName;
	}
	if (columnToUse->type == DualEnum) {
		const Column& secondColumn = columnToUse->table.getColumnByIndex(columnToUse->getIndex() + 1);
		assert(secondColumn.enumNameLists == columnToUse->enumNameLists);
		name += "/" + secondColumn.uiName;
	}
	
	if (foldOp != FilterFoldOp(-1)) {
		name += " (" + foldOpPage.getSelectedFoldOpName() + ")";
	}
	
	return name;
}





FilterWizard::FilterWizard(QWidget* parent, const NormalTable& tableToFilter) :
	QWizard(parent),
	tableToFilter(tableToFilter),
	tablePage(FilterWizardTablePage(parent, tableToFilter)),
	columnPage(FilterWizardColumnPage(parent, tableToFilter, tablePage)),
	foldOpPage(FilterWizardFoldOpPage(parent, tableToFilter, columnPage)),
	numberPrefPage(FilterWizardNumberPrefPage(parent)),
	namePage(FilterWizardNamePage(parent, tableToFilter, tablePage, columnPage, foldOpPage))
{
	setModal(true);
	setWizardStyle(QWizard::ModernStyle);
	setWindowTitle("New ascent filter");
	setMinimumSize(500, 300);
	setSizeGripEnabled(false);
	
	setPage(Page_Table,			&tablePage);
	setPage(Page_Column,		&columnPage);
	setPage(Page_FoldOp,		&foldOpPage);
	setPage(Page_NumberPrefs,	&numberPrefPage);
	setPage(Page_Name,			&namePage);
	
	setStartId(Page_Table);
}

FilterWizard::~FilterWizard()
{}



unique_ptr<Filter> FilterWizard::getFinishedFilter()
{
	const Column* const columnToUse = columnPage.getSelectedColumn();
	const FilterFoldOp foldOp = foldOpPage.getSelectedFoldOp();
	const QString name = field("name").toString();
	assert(columnToUse);
	assert(!name.isEmpty());
	
	DataType type = columnToUse->type;
	if (foldOp == FilterFoldOp_StringList) {
		type = String;
	} else if (foldOp != FilterFoldOp(-1)) {
		type = Integer;
	}
	
	switch (type) {
	case Integer: {
		const bool useIntClasses = field("numberPref.class").toBool();
		if (useIntClasses) {
			return make_unique<IntFilter>(tableToFilter, *columnToUse, name, 1000, 0, 8848);
		} else {
			return make_unique<IntFilter>(tableToFilter, *columnToUse, name);
		}
	}
	case ID: {
		return make_unique<IDFilter>(tableToFilter, *columnToUse, name);
	}
	case Enum: {
		return make_unique<EnumFilter>(tableToFilter, *columnToUse, name);
	}
	case DualEnum: {
		return make_unique<DualEnumFilter>(tableToFilter, *columnToUse, name);
	}
	case Bit: {
		return make_unique<BoolFilter>(tableToFilter, *columnToUse, name);
	}
	case String: {
		return make_unique<StringFilter>(tableToFilter, *columnToUse, name);
	}
	case Date: {
		return make_unique<DateFilter>(tableToFilter, *columnToUse, name);
	}
	case Time: {
		return make_unique<TimeFilter>(tableToFilter, *columnToUse, name);
	}
	case IDList: {
		qDebug() << "FilterWizard::getFinishedFilter - hit unexpected case IDList";
	}
	}
	assert(false);
	return nullptr;
}
