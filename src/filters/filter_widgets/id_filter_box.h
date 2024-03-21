#ifndef ID_FILTER_BOX_H
#define ID_FILTER_BOX_H

#include "src/data/item_id.h"
#include "src/filters/filter_widgets/filter_box.h"
#include "src/filters/id_filter.h"

#include <QComboBox>



class IDFilterBox : public FilterBox
{
	Q_OBJECT
	
	unique_ptr<IDFilter> filter;
	
	QComboBox* combo;
	std::function<void (QComboBox&, QList<ValidItemID>&)> populateItemCombo;
	
	QList<ValidItemID> selectableItemIDs;
	
public:
	explicit IDFilterBox(QWidget* parent, const QString& title, std::function<void (QComboBox&, QList<ValidItemID>&)> populateItemCombo, unique_ptr<IDFilter> filter);
	virtual ~IDFilterBox();
	
	virtual void setup();
	void setComboEntries();
	virtual void reset();
	
	virtual const Filter* getFilter() const;
};



#endif // ID_FILTER_BOX_H
