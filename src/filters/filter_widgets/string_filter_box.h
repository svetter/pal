#ifndef STRING_FILTER_BOX_H
#define STRING_FILTER_BOX_H

#include "src/filters/filter_widgets/filter_box.h"
#include "src/filters/string_filter.h"

#include <QLineEdit>



class StringFilterBox : public FilterBox
{
	Q_OBJECT
	
	unique_ptr<StringFilter> filter;
	
	QLineEdit* lineEdit;
	
public:
	explicit StringFilterBox(QWidget* parent, const QString& title, unique_ptr<StringFilter> filter);
	virtual ~StringFilterBox();
	
	virtual void setup();
	virtual void reset();
	
	virtual void updateFilterTypeSpecific();
	
	virtual const Filter* getFilter() const;
protected:
	virtual Filter* getFilter();
};



#endif // STRING_FILTER_BOX_H
