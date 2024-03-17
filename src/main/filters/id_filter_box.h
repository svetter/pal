#ifndef ID_FILTER_BOX_H
#define ID_FILTER_BOX_H

#include "src/data/item_id.h"
#include "src/main/filters/filter_box.h"

#include <QComboBox>



class IDFilterBox : public FilterBox
{
	Q_OBJECT
	
	QComboBox* combo;
	std::function<void (QComboBox&, QList<ValidItemID>&)> populateItemCombo;
	
	QList<ValidItemID> selectableItemIDs;
	
public:
	explicit IDFilterBox(QWidget* parent, const QString& title, std::function<void (QComboBox&, QList<ValidItemID>&)> populateItemCombo);
	virtual ~IDFilterBox();
	
	virtual void setup();
	void setComboEntries();
	virtual void reset();
};



#endif // ID_FILTER_BOX_H
