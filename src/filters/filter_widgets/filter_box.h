#ifndef FILTER_BOX_H
#define FILTER_BOX_H

#include "src/filters/filter.h"
#include "ui_filter_box.h"

#include <QGroupBox>
#include <QToolButton>
#include <QButtonGroup>

using std::unique_ptr, std::make_unique;



class FilterBox : public QGroupBox, public Ui_FilterBox
{
	Q_OBJECT
	
public:
	const DataType type;
	const QString title;
	
private:
	QToolButton* removeButton;
	QButtonGroup invertButtonGroup;
	
protected:
	FilterBox(QWidget* parent, DataType type, const QString& title);
	
public:
	virtual ~FilterBox();
	
	virtual void setup() = 0;
	virtual void reset();
	
private:
	void updateFilter();
protected:
	virtual void updateFilterTypeSpecific() = 0;
	
public:
	virtual const Filter* getFilter() const = 0;
protected:
	virtual Filter* getFilter() = 0;
	
protected:
	virtual void resizeEvent(QResizeEvent* event);
	virtual void moveEvent(QMoveEvent* event);
	
private:
	void positionRemoveButton();
	
signals:
	void filterChanged();
	void removeRequested();
};



#endif // FILTER_BOX_H
