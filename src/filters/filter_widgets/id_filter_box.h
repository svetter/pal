#ifndef ID_FILTER_BOX_H
#define ID_FILTER_BOX_H

#include "src/data/item_id.h"
#include "src/filters/filter_widgets/filter_box.h"
#include "src/filters/id_filter.h"

#include <QComboBox>



class IDFilterBox : public FilterBox
{
	Q_OBJECT
	
	IDFilter& filter;
	
	QComboBox* combo;
	std::function<void (QComboBox&, QList<ValidItemID>&)> populateItemCombo;
	
	QList<ValidItemID> selectableItemIDs;
	
public:
	explicit IDFilterBox(QWidget* parent, const QString& title, std::function<void (QComboBox&, QList<ValidItemID>&)> populateItemCombo, IDFilter& filter);
	virtual ~IDFilterBox();
	
	void setComboEntries();
	
	virtual void updateFilterTypeSpecific();
	
	virtual const Filter& getFilter() const;
};



#endif // ID_FILTER_BOX_H
