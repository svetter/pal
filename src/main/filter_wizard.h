#ifndef FILTERWIZARD_H
#define FILTERWIZARD_H

#include "src/comp_tables/composite_table.h"
#include "src/filters/filter.h"

#include <QWizard>
#include <QListWidget>
#include <QLineEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QSpinBox>



enum FilterWizardPage {
	Page_Column,
	Page_Settings
};



class FilterWizardColumnPage : public QWizardPage
{
	Q_OBJECT
	
	const CompositeTable& tableToFilter;
	
	QListWidget* const columnListWidget;
	
	QList<const CompositeColumn*> columnList;
	
public:
	FilterWizardColumnPage(QWidget* parent, const CompositeTable& tableToFilter);
	
	const CompositeColumn* getSelectedColumn() const;
	
protected:
	void initializePage() override;
	bool isComplete() const override;
	int nextId() const override;
};



class FilterWizardSettingsPage : public QWizardPage
{
	Q_OBJECT
	
	const CompositeTable& tableToFilter;
	const FilterWizardColumnPage& columnPage;
	
	QLineEdit* const nameEdit;
	QFrame* const intSettingsHLine;
	QRadioButton* const exactValueRadiobutton;
	QRadioButton* const classesRadiobutton;
	
	QGroupBox* const intClassesGroupBox;
	QLabel* const intClassIncrementLabel;
	QSpinBox* const intClassIncrementSpinner;
	QLabel* const intClassMinLabel;
	QSpinBox* const intClassMinSpinner;
	QLabel* const intClassMaxLabel;
	QSpinBox* const intClassMaxSpinner;
	QListWidget* const intClassPreview;
	
public:
	FilterWizardSettingsPage(QWidget* parent, const CompositeTable& tableToFilter, const FilterWizardColumnPage& columnPage);
	
	QString getName() const;
	QList<int> getIntSettings() const;
	
private:
	void handle_intFilterModeSelectionChanged();
	void handle_intClassIncrementChanged();
	void handle_intClassMinChanged();
	void handle_intClassMaxChanged();
	void updateIntClassPreview();
	
protected:
	void initializePage() override;
	bool isComplete() const override;
};



class FilterWizard : public QWizard
{
	Q_OBJECT
	
	const CompositeTable& tableToFilter;
	
	FilterWizardColumnPage columnPage;
	FilterWizardSettingsPage settingsPage;
	
public:
	FilterWizard(QWidget* parent, const CompositeTable& tableToFilter);
	~FilterWizard();
	
	Filter* getFinishedFilter();
};



#endif // FILTERWIZARD_H
