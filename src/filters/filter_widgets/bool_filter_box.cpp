#include "bool_filter_box.h"



BoolFilterBox::BoolFilterBox(QWidget* parent, const QString& title, unique_ptr<BoolFilter> filter) :
	FilterBox(parent, Bit, title),
	filter(std::move(filter)),
	yesRadiobutton(new QRadioButton(this)),
	noRadiobutton(new QRadioButton(this)),
	valueButtonGroup(QButtonGroup()),
	spacerL(new QSpacerItem(QSizePolicy::Expanding, QSizePolicy::Ignored)),
	spacerR(new QSpacerItem(QSizePolicy::Expanding, QSizePolicy::Ignored))
{
	connect(&valueButtonGroup,	&QButtonGroup::buttonClicked,	this,	&FilterBox::filterChanged);
	
	BoolFilterBox::setup();
	BoolFilterBox::reset();
}

BoolFilterBox::~BoolFilterBox()
{
	// Widgets deleted by layout
}



void BoolFilterBox::setup()
{
	yesRadiobutton->setObjectName("yesRadiobutton");
	yesRadiobutton->setText(tr("Yes"));
	yesRadiobutton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	
	noRadiobutton->setObjectName("noRadiobutton");
	noRadiobutton->setText(tr("No"));
	noRadiobutton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	
	valueButtonGroup.addButton(yesRadiobutton);
	valueButtonGroup.addButton(noRadiobutton);
	
	filterLayout->addItem(spacerL);
	filterLayout->addWidget(yesRadiobutton);
	filterLayout->addWidget(noRadiobutton);
	filterLayout->addItem(spacerR);
}

void BoolFilterBox::reset()
{
	FilterBox::reset();
	
	yesRadiobutton	->setChecked(true);
	noRadiobutton	->setChecked(false);
}



const Filter* BoolFilterBox::getFilter() const
{
	return filter.get();
}
