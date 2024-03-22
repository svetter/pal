#include "enum_filter_box.h"

#include "src/data/enum_names.h"

#include <QAbstractItemView>



EnumFilterBox::EnumFilterBox(QWidget* parent, const QString& title, const QStringList& entries, unique_ptr<EnumFilter> filter) :
	FilterBox(parent, Enum, title),
	filter(std::move(filter)),
	combo(new QComboBox(this)),
	entries(entries)
{
	connect(combo,	&QComboBox::currentIndexChanged,	this,	&EnumFilterBox::updateFilterTypeSpecific);
	
	EnumFilterBox::setup();
	EnumFilterBox::reset();
}

EnumFilterBox::~EnumFilterBox()
{
	// Widgets deleted by layout
}



void EnumFilterBox::setup()
{
	combo->setObjectName("combo");
	combo->insertItems(0, EnumNames::translateList(entries));
	combo->setMaximumWidth(200);
	combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	combo->view()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);
	combo->view()->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	
	filterLayout->addWidget(combo);
}

void EnumFilterBox::reset()
{
	FilterBox::reset();
	
	combo->setCurrentIndex(0);
}



void EnumFilterBox::updateFilterTypeSpecific()
{
	filter->setValue(combo->currentIndex());
	emit filterChanged();
}



const Filter* EnumFilterBox::getFilter() const
{
	return filter.get();
}

Filter* EnumFilterBox::getFilter()
{
	return filter.get();
}
