#include "enum_filter_box.h"

#include "src/data/enum_names.h"

#include <QAbstractItemView>



EnumFilterBox::EnumFilterBox(QWidget* parent, const QString& title, const QStringList& entries, EnumFilter& filter) :
	FilterBox(parent, Enum, title, filter),
	filter(filter),
	combo(new QComboBox(this)),
	entries(entries)
{
	combo->setObjectName("combo");
	combo->insertItems(0, EnumNames::translateList(entries));
	combo->setMaximumWidth(200);
	combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	combo->view()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);
	combo->view()->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	
	filterLayout->addWidget(combo);
	
	connect(combo,	&QComboBox::currentIndexChanged,	this,	&EnumFilterBox::updateFilterTypeSpecific);
	
	combo->setCurrentIndex(filter.value);
}

EnumFilterBox::~EnumFilterBox()
{
	// Widgets deleted by layout
}



void EnumFilterBox::updateFilterTypeSpecific()
{
	filter.value = combo->currentIndex();
	emit filterChanged();
}



const Filter& EnumFilterBox::getFilter() const
{
	return filter;
}
