#include "bool_filter_box.h"



BoolFilterBox::BoolFilterBox(QWidget* parent, const QString& title, BoolFilter& filter) :
	FilterBox(parent, Bit, title, filter),
	filter(filter),
	yesRadiobutton(new QRadioButton(this)),
	noRadiobutton(new QRadioButton(this)),
	yesNoButtonGroup(QButtonGroup()),
	spacerL(new QSpacerItem(QSizePolicy::Expanding, QSizePolicy::Ignored)),
	spacerR(new QSpacerItem(QSizePolicy::Expanding, QSizePolicy::Ignored))
{
	yesRadiobutton->setObjectName("yesRadiobutton");
	yesRadiobutton->setText(tr("Yes"));
	yesRadiobutton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	
	noRadiobutton->setObjectName("noRadiobutton");
	noRadiobutton->setText(tr("No"));
	noRadiobutton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	
	yesNoButtonGroup.addButton(yesRadiobutton);
	yesNoButtonGroup.addButton(noRadiobutton);
	
	filterLayout->addItem(spacerL);
	filterLayout->addWidget(yesRadiobutton);
	filterLayout->addWidget(noRadiobutton);
	filterLayout->addItem(spacerR);
	
	connect(&yesNoButtonGroup,	&QButtonGroup::buttonClicked,	this,	&BoolFilterBox::updateFilterTypeSpecific);
	
	yesRadiobutton->setChecked(filter.value);
	noRadiobutton->setChecked(!filter.value);
}

BoolFilterBox::~BoolFilterBox()
{
	// Widgets deleted by layout
}



void BoolFilterBox::updateFilterTypeSpecific()
{
	filter.value = !noRadiobutton->isChecked();
	emit filterChanged();
}



const Filter& BoolFilterBox::getFilter() const
{
	return filter;
}
