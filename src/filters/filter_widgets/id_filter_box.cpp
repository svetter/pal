#include "id_filter_box.h"

#include "src/data/item_id.h"

#include <QAbstractItemView>



IDFilterBox::IDFilterBox(QWidget* parent, const QString& title, std::function<void (QComboBox&, QList<ValidItemID>&)> populateItemCombo, unique_ptr<IDFilter> filter) :
	FilterBox(parent, ID, title),
	filter(std::move(filter)),
	combo(new QComboBox(this)),
	populateItemCombo(populateItemCombo),
	selectableItemIDs(QList<ValidItemID>())
{
	connect(combo,	&QComboBox::currentIndexChanged,	this,	&IDFilterBox::updateFilterTypeSpecific);
	
	IDFilterBox::setup();
	IDFilterBox::reset();
	setComboEntries();
}

IDFilterBox::~IDFilterBox()
{
	// Widgets deleted by layout
}



void IDFilterBox::setup()
{
	combo->setObjectName("combo");
	combo->setMaximumWidth(200);
	combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	combo->view()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);
	combo->view()->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	
	filterLayout->addWidget(combo);
}

void IDFilterBox::setComboEntries()
{
	ItemID previouslySelectedItemID = ItemID();
	const int comboIndex = combo->currentIndex();
	if (comboIndex > 0) {
		previouslySelectedItemID = selectableItemIDs.at(comboIndex - 1);	// 0 is None
	}
	populateItemCombo(*combo, selectableItemIDs);
	
	int newComboIndex = 0;
	ItemID newlySelectedItemID = ItemID();
	if (selectableItemIDs.contains(previouslySelectedItemID)) {
		newComboIndex = selectableItemIDs.indexOf(previouslySelectedItemID) + 1;	// 0 is None
		newlySelectedItemID = previouslySelectedItemID;
	}
	combo->setCurrentIndex(newComboIndex);
	
	if (newlySelectedItemID != previouslySelectedItemID) {
		emit filterChanged();
	}
}

void IDFilterBox::reset()
{
	FilterBox::reset();
	
	combo->clear();
	combo->setCurrentIndex(0);
}



void IDFilterBox::updateFilterTypeSpecific()
{
	filter->setValue(combo->currentIndex());
	emit filterChanged();
}



const Filter* IDFilterBox::getFilter() const
{
	return filter.get();
}

Filter* IDFilterBox::getFilter()
{
	return filter.get();
}
