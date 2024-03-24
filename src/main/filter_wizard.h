#ifndef FILTERWIZARD_H
#define FILTERWIZARD_H

#include "src/filters/filter.h"

#include <QWizard>
#include <QLabel>
#include <QRadioButton>
#include <QComboBox>
#include <QLineEdit>



enum FilterWizardPage {
	Page_Table,
	Page_Column,
	Page_FoldOp,
	Page_NumberPrefs,
	Page_Name
};



class FilterWizardTablePage : public QWizardPage
{
	Q_OBJECT
	
	const NormalTable& tableToFilter;
	
	QRadioButton* sameTableRadiobutton;
	QRadioButton* otherTableRadiobutton;
	QComboBox* otherTableCombo;
	
	QList<const NormalTable*> otherTableList;
	
public:
	FilterWizardTablePage(QWidget* parent, const NormalTable& tableToFilter);
	
	const NormalTable* getSelectedTable() const;
	
protected:
	bool isComplete() const override;
	int nextId() const override;
};



class FilterWizardColumnPage : public QWizardPage
{
	Q_OBJECT
	
	const NormalTable& tableToFilter;
	const FilterWizardTablePage& tablePage;
	
	QComboBox* columnCombo;
	
	QList<const Column*> columnList;
	
public:
	FilterWizardColumnPage(QWidget* parent, const NormalTable& tableToFilter, const FilterWizardTablePage& tablePage);
	
	const Column* getSelectedColumn() const;
	
protected:
	void initializePage() override;
	int nextId() const override;
};



class FilterWizardFoldOpPage : public QWizardPage
{
	Q_OBJECT
	
	const NormalTable& tableToFilter;
	const FilterWizardColumnPage& columnPage;
	
	QLabel* explainLabel;
	QComboBox* foldOpCombo;
	
	QList<FilterFoldOp> foldOpList;
	
public:
	FilterWizardFoldOpPage(QWidget* parent, const NormalTable& tableToFilter, const FilterWizardColumnPage& columnPage);
	
	FilterFoldOp getSelectedFoldOp() const;
	QString getSelectedFoldOpName() const;
	
protected:
	void initializePage() override;
	int nextId() const override;
};



class FilterWizardNumberPrefPage : public QWizardPage
{
	Q_OBJECT
	
	QRadioButton* exactValueRadiobutton;
	QRadioButton* classesRadiobutton;
	
public:
	FilterWizardNumberPrefPage(QWidget* parent);
	
protected:
	bool isComplete() const override;
	int nextId() const override;
};



class FilterWizardNamePage : public QWizardPage
{
	Q_OBJECT
	
	const NormalTable& tableToFilter;
	const FilterWizardTablePage& tablePage;
	const FilterWizardColumnPage& columnPage;
	const FilterWizardFoldOpPage& foldOpPage;
	
	QLineEdit* name;
	
public:
	FilterWizardNamePage(QWidget* parent, const NormalTable& tableToFilter, const FilterWizardTablePage& tablePage, const FilterWizardColumnPage& columnPage, const FilterWizardFoldOpPage& foldOpPage);
	
	QString getName() const;
	
protected:
	void initializePage() override;
	bool isComplete() const override;
	
	QString generateFilterName() const;
};



class FilterWizard : public QWizard
{
	Q_OBJECT
	
	const NormalTable& tableToFilter;
	
	FilterWizardTablePage tablePage;
	FilterWizardColumnPage columnPage;
	FilterWizardFoldOpPage foldOpPage;
	FilterWizardNumberPrefPage numberPrefPage;
	FilterWizardNamePage namePage;
	
public:
	FilterWizard(QWidget* parent, const NormalTable& tableToFilter);
	~FilterWizard();
	
	Filter* getFinishedFilter();
};



#endif // FILTERWIZARD_H
