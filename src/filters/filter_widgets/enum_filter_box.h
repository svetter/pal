#ifndef ENUM_FILTER_BOX_H
#define ENUM_FILTER_BOX_H

#include "src/filters/enum_filter.h"
#include "src/filters/filter_widgets/filter_box.h"

#include <QComboBox>



class EnumFilterBox : public FilterBox
{
	Q_OBJECT
	
	EnumFilter& filter;
	
	QComboBox* combo;
	const QStringList entries;
	
public:
	explicit EnumFilterBox(QWidget* parent, const QString& title, const QStringList& entries, EnumFilter& filter);
	virtual ~EnumFilterBox();
	
	virtual void updateFilterTypeSpecific();
	
	virtual const Filter& getFilter() const;
};



#endif // ENUM_FILTER_BOX_H