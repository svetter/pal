#ifndef COLUMN_WIZARD_H
#define COLUMN_WIZARD_H

#include "src/comp_tables/composite_table.h"

#include <QWizard>
#include <QLabel>
#include <QRadioButton>
#include <QComboBox>
#include <QLineEdit>



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
	ColumnWizardPage_Table,
	ColumnWizardPage_Column,
	ColumnWizardPage_FoldOp,
	ColumnWizardPage_Name
};



class ColumnWizardTablePage : public ColumnWizardPage
{
	Q_OBJECT
	
	QRadioButton* sameTableRadiobutton;
	QRadioButton* otherTableRadiobutton;
	QComboBox* otherTableCombo;
	
	QList<const NormalTable*> otherTableList;
	
public:
	ColumnWizardTablePage(QWidget* parent, Database& db, const CompositeTable& compTable);
	
	const NormalTable* getSelectedTable() const;
	
protected:
	bool isComplete() const override;
	int nextId() const override;
};



class ColumnWizardColumnPage : public ColumnWizardPage
{
	Q_OBJECT
	
	const ColumnWizardTablePage& tablePage;
	
	QComboBox* columnCombo;
	
	QList<const Column*> columnList;
	
public:
	ColumnWizardColumnPage(QWidget* parent, Database& db, const CompositeTable& compTable, const ColumnWizardTablePage& tablePage);
	
	const Column* getSelectedColumn() const;
	
protected:
	void initializePage() override;
	int nextId() const override;
};



class ColumnWizardFoldOpPage : public ColumnWizardPage
{
	Q_OBJECT
	
	const ColumnWizardColumnPage& columnPage;
	
	QLabel* explainLabel;
	QComboBox* foldOpCombo;
	
	QList<FoldOp> foldOpList;
	
public:
	ColumnWizardFoldOpPage(QWidget* parent, Database& db, const CompositeTable& compTable, const ColumnWizardColumnPage& columnPage);
	
	FoldOp getSelectedFoldOp() const;
	QString getSelectedFoldOpName() const;
	
protected:
	void initializePage() override;
	int nextId() const override;
};



class ColumnWizardNamePage : public ColumnWizardPage
{
	Q_OBJECT
	
	const ColumnWizardTablePage& tablePage;
	const ColumnWizardColumnPage& columnPage;
	const ColumnWizardFoldOpPage& foldOpPage;
	
	QLineEdit* name;
	
public:
	ColumnWizardNamePage(QWidget* parent, Database& db, const CompositeTable& compTable, const ColumnWizardTablePage& tablePage, const ColumnWizardColumnPage& columnPage, const ColumnWizardFoldOpPage& foldOpPage);
	
	QString getName() const;
	
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
	
	ColumnWizardTablePage	tablePage;
	ColumnWizardColumnPage	columnPage;
	ColumnWizardFoldOpPage	foldOpPage;
	ColumnWizardNamePage	namePage;
	
public:
	int visualIndexToUse;
	
	ColumnWizard(QWidget* parent, Database& db, CompositeTable& compTable);
	~ColumnWizard();
	
	CompositeColumn* getFinishedColumn();
};



#endif // COLUMN_WIZARD_H
