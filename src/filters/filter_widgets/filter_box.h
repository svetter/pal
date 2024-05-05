#ifndef FILTER_BOX_H
#define FILTER_BOX_H

#include "src/filters/filter.h"
#include "ui_filter_box.h"

#include <QGroupBox>
#include <QToolButton>
#include <QButtonGroup>



class FilterBox : public QGroupBox, public Ui_FilterBox
{
	Q_OBJECT
	
	Filter& genericFilter;
	
public:
	const DataType type;
	const QString title;
	
private:
	QToolButton* removeButton;
	QButtonGroup invertButtonGroup;
	
protected:
	FilterBox(QWidget* parent, DataType type, const QString& title, Filter& genericFilter);
	
public:
	virtual ~FilterBox();
	
private:
	void updateFilter();
protected:
	virtual void updateFilterTypeSpecific() = 0;
	
public:
	virtual const Filter& getFilter() const = 0;
	
protected:
	virtual void showEvent(QShowEvent* event) override;
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void moveEvent(QMoveEvent* event) override;
	virtual void changeEvent(QEvent* event) override;
	
private:
	void positionRemoveButton();
	
signals:
	void filterChanged();
	void removeRequested();
};



#endif // FILTER_BOX_H
