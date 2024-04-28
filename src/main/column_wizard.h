#ifndef COLUMN_WIZARD_H
#define COLUMN_WIZARD_H

#include "src/comp_tables/composite_table.h"
#include "src/comp_tables/numeric_fold_op.h"

#include <QWizard>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QListWidget>
#include <QRadioButton>
#include <QHBoxLayout>



class ColumnWizardPage : public QWizardPage
{
	Q_OBJECT
	
protected:
	Database& db;
	const CompositeTable& compTable;
	const NormalTable& baseTable;
	
	ColumnWizardPage(QWidget* parent, Database& db, const CompositeTable& compTable);
};



enum ColumnWizardPageID {
	ColumnWizardPage_TableColumn,
	ColumnWizardPage_FoldOp,
	ColumnWizardPage_Settings
};



class ColumnWizardTableColumnPage : public ColumnWizardPage
{
	Q_OBJECT
	
	QListWidget* const tableListWidget;
	QListWidget* const columnListWidget;
	QCheckBox* const useCountCheckbox;
	QLabel* const hintLabel;
	
	QList<const NormalTable*> tableList;
	QList<const Column*> columnList;
	
public:
	ColumnWizardTableColumnPage(QWidget* parent, Database& db, const CompositeTable& compTable);
	
private:
	void updateColumnList();
	void updateColumnListEnabled();
	void updateHint();
	
public:
	const NormalTable* getSelectedTable() const;
	const Column* getSelectedColumn() const;
	bool getUseCountSelected() const;
	bool selectionCanLeadToMultiResult() const;
	
protected:
	bool isComplete() const override;
	int nextId() const override;
};



class ColumnWizardFoldOpPage : public ColumnWizardPage
{
	Q_OBJECT
	
	const ColumnWizardTableColumnPage& tableColumnPage;
	
	QLabel* const numericLabel;
	QRadioButton* const averageRadio;
	QRadioButton* const sumRadio;
	QRadioButton* const maxRadio;
	QRadioButton* const minRadio;
	QSpacerItem* spacer;
	QLabel* const listLabel;
	QRadioButton* const listStringRadio;
	
public:
	ColumnWizardFoldOpPage(QWidget* parent, Database& db, const CompositeTable& compTable, const ColumnWizardTableColumnPage& tableColumnPage);
	
	bool numericFoldSelected() const;
	bool listStringFoldSelected() const;
	NumericFoldOp getSelectedNumericFoldOp() const;
	
protected:
	void initializePage() override;
	bool isComplete() const override;
	int nextId() const override;
};



class ColumnWizardSettingsPage : public ColumnWizardPage
{
	Q_OBJECT
	
	const ColumnWizardTableColumnPage&	tableColumnPage;
	const ColumnWizardFoldOpPage&		foldOpPage;
	
	QLineEdit* const nameEdit;
	QFrame* const suffixHLine;
	QLabel* const suffixLabel;
	QLineEdit* const suffixEdit;
	
public:
	ColumnWizardSettingsPage(QWidget* parent, Database& db, const CompositeTable& compTable, const ColumnWizardTableColumnPage& tableColumnPage, const ColumnWizardFoldOpPage& foldOpPage);
	
	QString getName() const;
	QString getSuffix() const;
	
protected:
	void initializePage() override;
	bool isComplete() const override;
	
	QString generateColumnName() const;
};



class ColumnWizard : public QWizard
{
	Q_OBJECT
	
	Database& db;
	CompositeTable& compTable;
	
	ColumnWizardTableColumnPage	tableColumnPage;
	ColumnWizardFoldOpPage		foldOpPage;
	ColumnWizardSettingsPage	settingsPage;
	
public:
	int visualIndexToUse;
	
	ColumnWizard(QWidget* parent, Database& db, CompositeTable& compTable);
	~ColumnWizard();
	
	CompositeColumn* getFinishedColumn();
	
	void handle_helpRequested();
};



#endif // COLUMN_WIZARD_H
