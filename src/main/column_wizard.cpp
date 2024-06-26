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

#include "column_wizard.h"

#include "src/comp_tables/fold_composite_column.h"
#include "src/db/database.h"

#include <QMessageBox>



ColumnWizardPage::ColumnWizardPage(QWidget* parent, Database& db, const CompositeTable& compTable) :
	QWizardPage(parent),
	db(db),
	compTable(compTable),
	baseTable(compTable.baseTable)
{}





ColumnWizardTableColumnPage::ColumnWizardTableColumnPage(QWidget* parent, Database& db, const CompositeTable& compTable) :
	ColumnWizardPage(parent, db, compTable),
	tableListWidget(new QListWidget(this)),
	columnListWidget(new QListWidget(this)),
	useCountCheckbox(new QCheckBox(this)),
	tableList(QList<const NormalTable*>()),
	columnList(QList<const Column*>())
{
	QVBoxLayout* const layout = new QVBoxLayout();
	setLayout(layout);
	setTitle(tr("Choose data source"));
	setSubTitle(tr("Choose the table and column whose data you want to be displayed (directly or processed) in the new column."));
	
	// Populate table list
	// First entry for the same table
	tableList.append(&baseTable);
	tableListWidget->addItem(tr("%1 (same table)").arg(baseTable.getItemNameSingular()));
	QListWidgetItem& firstItem = *tableListWidget->item(0);
	firstItem.setFont(QFont(firstItem.font().family(), firstItem.font().pointSize(), QFont::Bold));
	// Add entries for other tables
	for (const NormalTable* const table: db.getNormalItemTableList()) {
		if (table == &baseTable) continue;
		
		const Breadcrumbs& crumbs = db.getBreadcrumbsFor(baseTable, *table);
		QString listEntry = table->uiName;
		if (crumbs.isForwardOnly()) {
			listEntry = table->getItemNameSingular();
		}
		
		tableList.append(table);
		tableListWidget->addItem(listEntry);
	}
	tableListWidget->setCurrentRow(0);
	updateColumnList();
	
	// Set list widgets background
	tableListWidget->setAutoFillBackground(true);
	tableListWidget->setBackgroundRole(QPalette::Base);
	columnListWidget->setAutoFillBackground(true);
	columnListWidget->setBackgroundRole(QPalette::Base);

	QHBoxLayout* const listsLayout = new QHBoxLayout();
	listsLayout->addWidget(tableListWidget);
	QVBoxLayout* const columnListLayout = new QVBoxLayout();
	columnListLayout->addWidget(columnListWidget);
	useCountCheckbox->setText(tr("Use number of connected entries"));
	useCountCheckbox->setToolTip(tr("Show the number of items in this table which are connected to each row in the table containing the new column."));
	columnListLayout->addWidget(useCountCheckbox);
	listsLayout->addLayout(columnListLayout);
	layout->addLayout(listsLayout);
	
	connect(tableListWidget,	&QListWidget::currentRowChanged,	this, &ColumnWizardTableColumnPage::updateColumnList);
	connect(useCountCheckbox,	&QCheckBox::toggled,				this, &ColumnWizardTableColumnPage::updateColumnListEnabled);
	
	connect(tableListWidget,	&QListWidget::currentRowChanged,	this, &ColumnWizardTableColumnPage::completeChanged);
	connect(columnListWidget,	&QListWidget::currentRowChanged,	this, &ColumnWizardTableColumnPage::completeChanged);
	connect(useCountCheckbox,	&QCheckBox::toggled,				this, &ColumnWizardTableColumnPage::completeChanged);
}



void ColumnWizardTableColumnPage::updateColumnList()
{
	const NormalTable* const tableToUse = getSelectedTable();
	const bool sameTable = tableToUse == &baseTable;
	
	columnListWidget->clear();
	columnList.clear();
	if (!tableToUse) return;
	
	const QList<const Column*> identityRepColumns = tableToUse->getIdentityRepresentationColumns();
	const Breadcrumbs& crumbs = db.getBreadcrumbsFor(baseTable, *tableToUse);
	const bool multiResultPossible = !crumbs.isForwardOnly();
	
	const QList<const Column*> allColumns = tableToUse->getColumnList();
	for (int i = 0; i < allColumns.size(); i++) {
		const Column* column = allColumns.at(i);
		if (column->foreignColumn || column->type == DualEnum) {
			continue;
		} else if (column->primaryKey && (sameTable || identityRepColumns.size() < 2)) {
			continue;
		}
		
		QString uiName = column->uiName;
		if (column->primaryKey) {
			QStringList repColumnUiNames = QStringList();
			for (const Column* repColumn : identityRepColumns) {
				if (repColumn->isKey()) continue;
				repColumnUiNames.append(repColumn->uiName);
			}
			uiName = tr("Identity (%1)").arg(repColumnUiNames.join(", "));
		}
		
		columnList.append(column);
		columnListWidget->addItem(uiName);
		
		// Mark identity column in bold
		const bool isIdentityColumn = identityRepColumns.size() == 1 && identityRepColumns.contains(column);
		if (column->primaryKey || (!sameTable && isIdentityColumn)) {
			QListWidgetItem& item = *columnListWidget->item(columnListWidget->count() - 1);
			item.setFont(QFont(item.font().family(), item.font().pointSize(), QFont::Bold));
		}
	}
	
	// Update useCountCheckbox enabled state
	useCountCheckbox->setEnabled(multiResultPossible);
	if (!multiResultPossible) {
		useCountCheckbox->setChecked(false);
	}
}

void ColumnWizardTableColumnPage::updateColumnListEnabled()
{
	if (useCountCheckbox->isVisible()) {
		columnListWidget->setEnabled(!useCountCheckbox->isChecked());
	} else {
		columnListWidget->setEnabled(true);
	}
}



const NormalTable* ColumnWizardTableColumnPage::getSelectedTable() const
{
	if (tableListWidget->currentRow() < 0) return nullptr;
	return tableList.at(tableListWidget->currentRow());
}

const Column* ColumnWizardTableColumnPage::getSelectedColumn() const
{
	if (columnListWidget->currentRow() < 0) return nullptr;
	return columnList.at(columnListWidget->currentRow());
}

bool ColumnWizardTableColumnPage::getUseCountSelected() const
{
	return useCountCheckbox->isChecked();
}

bool ColumnWizardTableColumnPage::selectionCanLeadToMultiResult() const
{
	const NormalTable* const tableToUse = getSelectedTable();
	if (!tableToUse) return false;
	
	const Breadcrumbs& crumbs = db.getBreadcrumbsFor(baseTable, *tableToUse);
	if (!crumbs.isForwardOnly()) {
		return !useCountCheckbox->isChecked();
	}
	return false;
}



bool ColumnWizardTableColumnPage::isComplete() const
{
	return getSelectedTable() && (getSelectedColumn() || getUseCountSelected());
}

int ColumnWizardTableColumnPage::nextId() const
{
	if (selectionCanLeadToMultiResult()) {
		return ColumnWizardPage_FoldOp;
	}
	return ColumnWizardPage_Settings;
}





ColumnWizardFoldOpPage::ColumnWizardFoldOpPage(QWidget* parent, Database& db, const CompositeTable& compTable, const ColumnWizardTableColumnPage& tableColumnPage) :
	ColumnWizardPage(parent, db, compTable),
	tableColumnPage(tableColumnPage),
	numericLabel(new QLabel(this)),
	averageRadio(new QRadioButton(this)),
	sumRadio(new QRadioButton(this)),
	maxRadio(new QRadioButton(this)),
	minRadio(new QRadioButton(this)),
	spacer(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed)),
	listLabel(new QLabel(this)),
	listStringRadio(new QRadioButton(this))
{
	QVBoxLayout* const layout = new QVBoxLayout();
	setLayout(layout);
	setTitle(tr("Choose fold operation"));
	setSubTitle(tr("Choose a method for reducing multiple values into one for displaying them."));
	
	numericLabel->setText(tr("Numeric values can be processed in the following ways:"));
	numericLabel->setWordWrap(true);
	layout->addWidget(numericLabel);
	
	averageRadio->setText(tr("Average"));
	sumRadio->setText(tr("Sum"));
	maxRadio->setText(tr("Maximum"));
	minRadio->setText(tr("Minimum"));
	layout->addWidget(averageRadio);
	layout->addWidget(sumRadio);
	layout->addWidget(maxRadio);
	layout->addWidget(minRadio);
	
	layout->addSpacerItem(spacer);
	
	listLabel->setText(tr("Values can be collected in a comma-separated list:"));
	listLabel->setWordWrap(true);
	layout->addWidget(listLabel);
	
	listStringRadio->setText(tr("List"));
	layout->addWidget(listStringRadio);
	
	connect(averageRadio,		&QRadioButton::toggled,	this, &ColumnWizardFoldOpPage::completeChanged);
	connect(sumRadio,			&QRadioButton::toggled,	this, &ColumnWizardFoldOpPage::completeChanged);
	connect(maxRadio,			&QRadioButton::toggled,	this, &ColumnWizardFoldOpPage::completeChanged);
	connect(minRadio,			&QRadioButton::toggled,	this, &ColumnWizardFoldOpPage::completeChanged);
	connect(listStringRadio,	&QRadioButton::toggled,	this, &ColumnWizardFoldOpPage::completeChanged);
}



bool ColumnWizardFoldOpPage::numericFoldSelected() const
{
	return averageRadio->isChecked() || sumRadio->isChecked() || maxRadio->isChecked() || minRadio->isChecked();
}

bool ColumnWizardFoldOpPage::listStringFoldSelected() const
{
	return listStringRadio->isChecked();
}

NumericFoldOp ColumnWizardFoldOpPage::getSelectedNumericFoldOp() const
{
	if (averageRadio->isChecked())	return AverageFold;
	if (sumRadio->isChecked())		return SumFold;
	if (maxRadio->isChecked())		return MaxFold;
	if (minRadio->isChecked())		return MinFold;
	return NumericFoldOp(-1);
}



void ColumnWizardFoldOpPage::initializePage()
{
	const Column* const columnToUse = tableColumnPage.getSelectedColumn();
	assert(columnToUse);
	
	const bool numericPossible = columnToUse->type == Integer;
	numericLabel->setVisible(numericPossible);
	averageRadio->setVisible(numericPossible);
	sumRadio->setVisible(numericPossible);
	maxRadio->setVisible(numericPossible);
	minRadio->setVisible(numericPossible);
	if (!numericPossible) {
		averageRadio->setChecked(false);
		sumRadio->setChecked(false);
		maxRadio->setChecked(false);
		minRadio->setChecked(false);
		spacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
		listStringRadio->setChecked(true);
	} else {
		spacer->changeSize(0, 15, QSizePolicy::Fixed, QSizePolicy::Fixed);
	}
}

bool ColumnWizardFoldOpPage::isComplete() const
{
	return numericFoldSelected() || listStringFoldSelected();
}

int ColumnWizardFoldOpPage::nextId() const
{
	return ColumnWizardPage_Settings;
}





ColumnWizardSettingsPage::ColumnWizardSettingsPage(QWidget* parent, Database& db, const CompositeTable& compTable, const ColumnWizardTableColumnPage& tableColumnPage, const ColumnWizardFoldOpPage& foldOpPage) :
	ColumnWizardPage(parent, db, compTable),
	tableColumnPage(tableColumnPage),
	foldOpPage(foldOpPage),
	nameEdit(new QLineEdit(this)),
	suffixHLine(new QFrame(this)),
	suffixLabel(new QLabel(this)),
	suffixEdit(new QLineEdit(this))
{
	QVBoxLayout* const layout = new QVBoxLayout();
	setLayout(layout);
	setTitle(tr("Customization"));
	setSubTitle(tr("Choose a name for the new column and customize further settings."));
	
	QLabel* const nameLabel = new QLabel(tr("Name the new column:"), this);
	nameLabel->setWordWrap(true);
	layout->addWidget(nameLabel);
	nameEdit->setPlaceholderText(tr("Required"));
	layout->addWidget(nameEdit);
	
	suffixHLine->setFrameShape(QFrame::HLine);
	suffixHLine->setFrameShadow(QFrame::Sunken);
	layout->addSpacing(10);
	layout->addWidget(suffixHLine);
	layout->addSpacing(10);
	
	QLabel* const suffixLabel = new QLabel(tr("Set a suffix to be appended to every cell of the column:"), this);
	suffixLabel->setWordWrap(true);
	layout->addWidget(suffixLabel);
	suffixEdit->setPlaceholderText(tr("Optional"));
	layout->addWidget(suffixEdit);
}

QString ColumnWizardSettingsPage::getName() const
{
	return nameEdit->text();
}

QString ColumnWizardSettingsPage::getSuffix() const
{
	return suffixEdit->text();
}

void ColumnWizardSettingsPage::initializePage()
{
	const Column* const columnToUse = tableColumnPage.getSelectedColumn();
	const bool useCount = tableColumnPage.getUseCountSelected();
	const bool numericFold = foldOpPage.numericFoldSelected();
	assert(columnToUse || useCount);
	if (columnToUse) assert(!columnToUse->table.isAssociative);
	
	nameEdit->setText(generateColumnName());
	
	const bool setSuffix = useCount || numericFold || columnToUse->type == Integer;
	suffixHLine->setVisible(setSuffix);
	suffixLabel->setVisible(setSuffix);
	suffixEdit->setVisible(setSuffix);
	suffixEdit->clear();
}

bool ColumnWizardSettingsPage::isComplete() const
{
	return !nameEdit->text().isEmpty();
}

QString ColumnWizardSettingsPage::generateColumnName() const
{
	const NormalTable* const tableToUse = tableColumnPage.getSelectedTable();
	const bool useCount = tableColumnPage.getUseCountSelected();
	const Column* const columnToUse = tableColumnPage.getSelectedColumn();
	const bool multiResult = tableColumnPage.selectionCanLeadToMultiResult();
	const bool numericFold = multiResult && foldOpPage.numericFoldSelected();
	const bool listStringFold = multiResult && foldOpPage.listStringFoldSelected();
	const NumericFoldOp foldOp = foldOpPage.getSelectedNumericFoldOp();
	assert(tableToUse);
	assert(columnToUse || useCount);
	
	QString tableString = "";
	if (tableToUse != &baseTable) {
		if (multiResult || useCount) {
			tableString = tableToUse->uiName;
		} else {
			tableString = tableToUse->getItemNameSingular();
		}
	}
	
	if (useCount) {
		return tr("# %1").arg(tableString);
	}
	
	QString columnString = "";
	if (columnToUse->foreignColumn) {
		const Table& foreignTable = columnToUse->getReferencedForeignColumn().table;
		assert(!foreignTable.isAssociative);
		const NormalTable& targetTable = (const NormalTable&) foreignTable;
		columnString = targetTable.getItemNameSingular();
	} else {
		columnString = columnToUse->uiName;
	}
	if (columnToUse->type == DualEnum) {
		const Column& secondColumn = columnToUse->table.getColumnByIndex(columnToUse->getIndex() + 1);
		assert(secondColumn.enumNameLists == columnToUse->enumNameLists);
		columnString += "/" + secondColumn.uiName;
	}
	
	const bool sameTable = tableToUse == &baseTable;
	const QList<const Column*> identityRepColumns = tableToUse->getIdentityRepresentationColumns();
	const bool isIdentityColumn = identityRepColumns.size() == 1 && identityRepColumns.contains(columnToUse);
	if (columnToUse->primaryKey || (!sameTable && isIdentityColumn)) {
		// Identity column - only use table name
		columnString = tableString;
		tableString = "";
	}
	
	QString valueString = columnString;
	if (numericFold) {
		switch (foldOp) {
		case AverageFold:	valueString = tr("Avg. %1")		.arg(columnString);	break;
		case SumFold:		valueString = tr("Sum of %1")	.arg(columnString);	break;
		case MaxFold:		valueString = tr("Max %1")		.arg(columnString);	break;
		case MinFold:		valueString = tr("Min %1")		.arg(columnString);	break;
		default: assert(false);
		}
	}
	else if (listStringFold) {
		if (columnToUse->primaryKey || isIdentityColumn) {
			valueString = tr("All %1").arg(columnString);
		} else {
			valueString = tr("%1 (List)").arg(columnString);
		}
	}
	
	const QString separator = tableString.isEmpty() || valueString.isEmpty() ? "" : ": ";
	return tableString + separator + valueString;
}





ColumnWizard::ColumnWizard(QWidget* parent, Database& db, CompositeTable& compTable) :
	QWizard(parent),
	db(db),
	compTable(compTable),
	tableColumnPage	(ColumnWizardTableColumnPage(parent, db, compTable)),
	foldOpPage		(ColumnWizardFoldOpPage		(parent, db, compTable, tableColumnPage)),
	settingsPage	(ColumnWizardSettingsPage	(parent, db, compTable, tableColumnPage, foldOpPage))
{
	setModal(true);
	setOption(HaveHelpButton, true);
	setOption(HelpButtonOnRight, false);
	setWizardStyle(QWizard::ModernStyle);
	setWindowTitle(compTable.baseTable.getNewCustomColumnString());
	setMinimumSize(500, 300);
	setSizeGripEnabled(false);
	
	setPage(ColumnWizardPage_TableColumn,	&tableColumnPage);
	setPage(ColumnWizardPage_FoldOp,		&foldOpPage);
	setPage(ColumnWizardPage_Settings,		&settingsPage);
	
	setStartId(ColumnWizardPage_TableColumn);
	
	connect(this, &ColumnWizard::helpRequested, this, &ColumnWizard::handle_helpRequested);
}

ColumnWizard::~ColumnWizard()
{}



CompositeColumn* ColumnWizard::getFinishedColumn()
{
	const NormalTable* tableToUse = tableColumnPage.getSelectedTable();
	assert(tableToUse);
	const bool sameTable = &compTable.baseTable == tableToUse;
	const bool useCount = tableColumnPage.getUseCountSelected();
	assert(!sameTable || !useCount);
	const Column* const columnToUse = tableColumnPage.getSelectedColumn();
	assert(columnToUse || useCount);
	if (columnToUse) assert(!columnToUse->table.isAssociative);
	const bool multiResult = tableColumnPage.selectionCanLeadToMultiResult();
	assert(!multiResult || !useCount);
	const bool numericFold = multiResult && foldOpPage.numericFoldSelected();
	const bool listStringFold = multiResult && foldOpPage.listStringFoldSelected();
	const NumericFoldOp foldOp = foldOpPage.getSelectedNumericFoldOp();
	const QString uiName = settingsPage.getName();
	assert(!uiName.isEmpty());
	
	// Create internal name
	ProjectSetting<int>& counterSetting = db.projectSettings.numCustomColumnsCreated;
	const int newCustomColumnIndex = counterSetting.get() + 1;
	counterSetting.set(*this, newCustomColumnIndex);
	QString name = "CUSTOM_COLUMN_" + QString::number(newCustomColumnIndex) + "_";
	if (useCount) {
		name += "count_" + tableToUse->name;
	} else {
		name += columnToUse->table.name + "_" + columnToUse->name;
	}
	
	const QString suffix = settingsPage.getSuffix();
	
	
	if (sameTable) {
		return new DirectCompositeColumn(compTable, name, uiName, suffix, *columnToUse);
	}
	
	if (useCount) {
		return new CountFoldCompositeColumn(compTable, name, uiName, suffix, *tableToUse);
	}
	
	if (!multiResult) {
		return new ReferenceCompositeColumn(compTable, name, uiName, suffix, *columnToUse);
	}
	
	if (numericFold) {
		assert(foldOp != NumericFoldOp(-1));
		return new NumericFoldCompositeColumn(compTable, name, uiName, suffix, foldOp, (const ValueColumn&) *columnToUse);
	}
	
	if (listStringFold) {
		return new ListStringFoldCompositeColumn(compTable, name, uiName, (const ValueColumn&) *columnToUse, columnToUse->enumNames);
	}
	
	assert(false);
	return nullptr;
}



void ColumnWizard::handle_helpRequested()
{
	QString title = tr("Help with creating a custom column");
	QString text = tr(
		"This wizard will create a new column for the %1 table.\n"
		"Note that this custom column will not hold any new data, but show data already in the database, either directly or processed to show statistical data.\n\n"
		"On the first page, you can choose any table from the project database, and then a column from that table. "
		"The new column will then show the entry from the selected column which is connected to each %2.\n"
		"Depending on the selected table, it is possible that multiple entries are connected to a single %2. "
		"In that case, the new column can either list all connected values, or, if the selected column contains numbers, the average, sum, maximum or minimum of those numbers can be calculated.\n"
		"Alternatively, where multiple values are possible, you can choose to display the count of those values, using the checkbox on the first page.\n\n"
		"For some tables, there is an additional column listed as \"Identity\". "
		"If chosen, this will create a representation of each item's identity by combining values from two or more other fields.\n\n"
		"Each table has an entry which is shown in bold. "
		"These can be used for identity filters, which let you select an item and include or exclude all rows where that item makes an appearance."
	).arg(compTable.baseTable.uiName, compTable.baseTable.getItemNameSingular());
	
	QMessageBox::information(this, title, text, QMessageBox::Ok, QMessageBox::Ok);
}
