#ifndef STRING_FILTER_BOX_H
#define STRING_FILTER_BOX_H

#include "src/filters/filter_widgets/filter_box.h"
#include "src/filters/string_filter.h"

#include <QLineEdit>



class StringFilterBox : public FilterBox
{
	Q_OBJECT
	
	StringFilter& filter;
	
	QLineEdit* lineEdit;
	
public:
	explicit StringFilterBox(QWidget* parent, const QString& title, StringFilter& filter);
	virtual ~StringFilterBox();
	
	virtual void updateFilterTypeSpecific();
	
	virtual const Filter& getFilter() const;
};



#endif // STRING_FILTER_BOX_H
