#include "id_filter_box.h"

#include "src/data/item_id.h"

#include <QAbstractItemView>



IDFilterBox::IDFilterBox(QWidget* parent, const QString& title, std::function<void (QComboBox&, QList<ValidItemID>&)> populateItemCombo) :
	FilterBox(parent, ID, title),
	combo(new QComboBox(this)),
	populateItemCombo(populateItemCombo),
	selectableItemIDs(QList<ValidItemID>())
{
	connect(combo,	&QComboBox::currentIndexChanged,	this,	&IDFilterBox::filterChanged);
	
	IDFilterBox::setup();
	IDFilterBox::reset();
	setComboEntries();
}

IDFilterBox::~IDFilterBox()
{}



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
