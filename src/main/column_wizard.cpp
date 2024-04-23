#include "column_wizard.h"

#include "src/db/database.h"

#include <QVBoxLayout>



ColumnWizardPage::ColumnWizardPage(QWidget* parent, Database& db, const CompositeTable& compTable) :
	QWizardPage(parent),
	db(db),
	compTable(compTable),
	baseTable(compTable.getBaseTable())
{}





ColumnWizardTablePage::ColumnWizardTablePage(QWidget* parent, Database& db, const CompositeTable& compTable) :
	ColumnWizardPage(parent, db, compTable),
	sameTableRadiobutton(new QRadioButton(this)),
	otherTableRadiobutton(new QRadioButton(this)),
	otherTableCombo(new QComboBox(this)),
	otherTableList(QList<const NormalTable*>())
{
	QVBoxLayout* const layout = new QVBoxLayout();
	setLayout(layout);
	setTitle(tr("Choose table"));
	setSubTitle(tr("Choose the table which contains the values to display (directly or processed) in the new column."));
	
	sameTableRadiobutton->setText(tr("Use values from the same table (%1)").arg(compTable.uiName));
	layout->addWidget(sameTableRadiobutton);
	layout->addWidget(new QLabel(tr("or"), this));
	otherTableRadiobutton->setText(tr("Use values from a different table:"));
	layout->addWidget(otherTableRadiobutton);
	otherTableCombo->setEnabled(false);
	otherTableCombo->setPlaceholderText(tr("Choose table"));
	for (const NormalTable* const table: db.getNormalItemTableList()) {
		if (table == &baseTable) continue;
		
		const Breadcrumbs& crumbs = db.getBreadcrumbsFor(baseTable, *table);
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

const NormalTable* ColumnWizardTablePage::getSelectedTable() const
{
	if (sameTableRadiobutton->isChecked()) {
		return &baseTable;
	} else if (otherTableRadiobutton->isChecked()) {
		if (otherTableCombo->currentIndex() < 0) return nullptr;
		return otherTableList.at(otherTableCombo->currentIndex());
	} else {
		return nullptr;
	}
}

bool ColumnWizardTablePage::isComplete() const
{
	if (sameTableRadiobutton->isChecked()) return true;
	if (!otherTableRadiobutton->isChecked()) return false;
	return otherTableCombo->currentIndex() != -1;
}

int ColumnWizardTablePage::nextId() const
{
	return ColumnWizardPage_Column;
}





ColumnWizardColumnPage::ColumnWizardColumnPage(QWidget* parent, Database& db, const CompositeTable& compTable, const ColumnWizardTablePage& tablePage) :
	ColumnWizardPage(parent, db, compTable),
	tablePage(tablePage),
	columnCombo(new QComboBox(this)),
	columnList(QList<const Column*>())
{
	QVBoxLayout* const layout = new QVBoxLayout();
	setLayout(layout);
	setTitle(tr("Choose value field"));
	setSubTitle(tr("Choose the field whose values to use for the new column."));
	
	layout->addWidget(columnCombo);
	
	registerField("column.combo*",	columnCombo);
}

const Column* ColumnWizardColumnPage::getSelectedColumn() const
{
	if (columnCombo->currentIndex() < 0) return nullptr;
	return columnList.at(columnCombo->currentIndex());
}

void ColumnWizardColumnPage::initializePage()
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

int ColumnWizardColumnPage::nextId() const
{
	const NormalTable* const tableToUse = tablePage.getSelectedTable();
	const Column* const columnToUse = getSelectedColumn();
	assert(columnToUse);
	
	const Breadcrumbs& crumbs = db.getBreadcrumbsFor(baseTable, *tableToUse);
	const bool forwardReference = crumbs.isForwardOnly();
	if (!forwardReference) {
		return ColumnWizardPage_FoldOp;
	}
	return ColumnWizardPage_Name;
}





ColumnWizardFoldOpPage::ColumnWizardFoldOpPage(QWidget* parent, Database& db, const CompositeTable& compTable, const ColumnWizardColumnPage& columnPage) :
	ColumnWizardPage(parent, db, compTable),
	columnPage(columnPage),
	explainLabel(new QLabel(this)),
	foldOpCombo(new QComboBox(this)),
	foldOpList(QList<FoldOp>())
{
	QVBoxLayout* const layout = new QVBoxLayout();
	setLayout(layout);
	setTitle(tr("Choose processing method"));
	setSubTitle(tr("Choose a way to collapse multiple values into one in order to display them."));
	
	explainLabel->setWordWrap(true);
	layout->addWidget(explainLabel);
	
	layout->addWidget(foldOpCombo);
	
	registerField("foldOp.combo*",	foldOpCombo);
}

FoldOp ColumnWizardFoldOpPage::getSelectedFoldOp() const
{
	if (foldOpCombo->currentIndex() < 0) return FoldOp(-1);
	return foldOpList.at(foldOpCombo->currentIndex());
}

QString ColumnWizardFoldOpPage::getSelectedFoldOpName() const
{
	if (foldOpCombo->currentIndex() < 0) return QString();
	return foldOpCombo->currentText();
}

void ColumnWizardFoldOpPage::initializePage()
{
	const Column* const columnToUse = columnPage.getSelectedColumn();
	assert(columnToUse);
	
	if (columnToUse->type == String) {
		explainLabel->setText(tr("The column you chose can contain multiple values for each row in the current table.\nThe new column can display a list of those values or their count."));
	} else if (columnToUse->type == Integer) {
		explainLabel->setText(tr("The column you chose can contain multiple values for each row in the current table.\nThe new column can display the count of those values, or their average, sum, maximum, or minimum."));
	} else {
		explainLabel->setText(tr("The column you chose can contain multiple values for each row in the current table.\nThe new column can display the count of those values."));
	}
	
	foldOpList.clear();
	foldOpCombo->clear();
	
	foldOpList.append(CountFold);			foldOpCombo->addItem(tr("Count"));
	if (columnToUse->type == Integer) {
		foldOpList.append(AverageFold);		foldOpCombo->addItem(tr("Average"));
		foldOpList.append(SumFold);			foldOpCombo->addItem(tr("Sum"));
		foldOpList.append(MaxFold);			foldOpCombo->addItem(tr("Maximum"));
		foldOpList.append(MinFold);			foldOpCombo->addItem(tr("Minimum"));
	}
	if (columnToUse->type == String) {
		foldOpList.append(StringListFold);	foldOpCombo->addItem(tr("List"));
	}
}

int ColumnWizardFoldOpPage::nextId() const
{
	return ColumnWizardPage_Name;
}





ColumnWizardNamePage::ColumnWizardNamePage(QWidget* parent, Database& db, const CompositeTable& compTable, const ColumnWizardTablePage& tablePage, const ColumnWizardColumnPage& columnPage, const ColumnWizardFoldOpPage& foldOpPage) :
	ColumnWizardPage(parent, db, compTable),
	tablePage(tablePage),
	columnPage(columnPage),
	foldOpPage(foldOpPage),
	name(new QLineEdit(this))
{
	QVBoxLayout* const layout = new QVBoxLayout();
	setLayout(layout);
	setTitle(tr("Set column name"));
	setSubTitle(tr("Choose a name for the new column, or leave the automatic name in place."));
	
	name->setPlaceholderText(tr("Name the new column"));
	layout->addWidget(name);
	
	registerField("name*",	name);
}

QString ColumnWizardNamePage::getName() const
{
	return name->text();
}

void ColumnWizardNamePage::initializePage()
{
	name->setText(generateColumnName());
}

bool ColumnWizardNamePage::isComplete() const
{
	return !name->text().isEmpty();
}

QString ColumnWizardNamePage::generateColumnName() const
{
	const NormalTable* const tableToUse = tablePage.getSelectedTable();
	const Column* const columnToUse = columnPage.getSelectedColumn();
	const FoldOp foldOp = foldOpPage.getSelectedFoldOp();
	assert(tableToUse);
	assert(columnToUse);
	
	QString name = "";
	if (tableToUse != &baseTable) {
		const Breadcrumbs& crumbs = db.getBreadcrumbsFor(baseTable, *tableToUse);
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
	
	if (foldOp != FoldOp(-1)) {
		name += " (" + foldOpPage.getSelectedFoldOpName() + ")";
	}
	
	return name;
}





ColumnWizard::ColumnWizard(QWidget* parent, Database& db, const CompositeTable& compTable) :
	QWizard(parent),
	db(db),
	compTable(compTable),
	tablePage	(ColumnWizardTablePage	(parent, db, compTable)),
	columnPage	(ColumnWizardColumnPage	(parent, db, compTable, tablePage)),
	foldOpPage	(ColumnWizardFoldOpPage	(parent, db, compTable, columnPage)),
	namePage	(ColumnWizardNamePage	(parent, db, compTable, tablePage, columnPage, foldOpPage))
{
	setModal(true);
	setWizardStyle(QWizard::ModernStyle);
	setWindowTitle(compTable.getBaseTable().getNewCustomColumnString());
	setMinimumSize(500, 300);
	setSizeGripEnabled(false);
	
	setPage(ColumnWizardPage_Table,		&tablePage);
	setPage(ColumnWizardPage_Column,	&columnPage);
	setPage(ColumnWizardPage_FoldOp,	&foldOpPage);
	setPage(ColumnWizardPage_Name,		&namePage);
	
	setStartId(ColumnWizardPage_Table);
}

ColumnWizard::~ColumnWizard()
{}



CompositeColumn* ColumnWizard::getFinishedColumn()
{
	const Column* const columnToUse = columnPage.getSelectedColumn();
	const FoldOp foldOp = foldOpPage.getSelectedFoldOp();
	const QString name = field("name").toString();
	assert(columnToUse);
	assert(!name.isEmpty());
	
	// TODO
	return nullptr;
}
