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

#ifndef FILTERWIZARD_H
#define FILTERWIZARD_H

#include "src/comp_tables/composite_table.h"
#include "src/filters/filter.h"

#include <QWizard>
#include <QBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QSpinBox>

class FilterBox;



enum FilterWizardPage {
	Page_Column,
	Page_Mode,
	Page_Settings
};



class FilterWizardColumnPage : public QWizardPage
{
	Q_OBJECT
	
	const CompositeTable& tableToFilter;
	const QTableView& tableView;
	
	QListWidget* const columnListWidget;
	
	QList<const CompositeColumn*> columnList;
	
public:
	FilterWizardColumnPage(QWidget* parent, const CompositeTable& tableToFilter, const QTableView& tableView);
	
	const CompositeColumn* getSelectedColumn() const;
	
protected:
	void initializePage() override;
	bool isComplete() const override;
	int nextId() const override;
};



class FilterWizardModePage : public QWizardPage
{
	Q_OBJECT
	
	const CompositeTable& tableToFilter;
	const FilterWizardColumnPage& columnPage;
	
	QRadioButton* const filterIdentityRadio;
	QRadioButton* const filterStringRadio;
	
	QGroupBox* const previewBox;
	QHBoxLayout* const previewLayout;
	FilterBox* idFilterBox;
	FilterBox* stringFilterBox;
	
public:
	FilterWizardModePage(QWidget* parent, const CompositeTable& tableToFilter, const FilterWizardColumnPage& columnPage);
	
	bool isProxyIDModeSelected() const;
	
private:
	void handle_radiosChanged();
	
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
	const FilterWizardModePage& modePage;
	
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
	FilterWizardSettingsPage(QWidget* parent, const CompositeTable& tableToFilter, const FilterWizardColumnPage& columnPage, const FilterWizardModePage& modePage);
	
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
	FilterWizardModePage modePage;
	FilterWizardSettingsPage settingsPage;
	
public:
	FilterWizard(QWidget* parent, const CompositeTable& tableToFilter, const QTableView& tableView);
	~FilterWizard();
	
	Filter* getFinishedFilter();
	
	static bool columnEligibleForProxyIDMode(const CompositeColumn& column, bool* autoProxy = nullptr);
};



#endif // FILTERWIZARD_H
