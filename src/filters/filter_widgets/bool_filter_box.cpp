#include "bool_filter_box.h"



BoolFilterBox::BoolFilterBox(QWidget* parent, const QString& title) :
	FilterBox(parent, Bit, title),
	yesRadiobutton(new QRadioButton(this)),
	noRadiobutton(new QRadioButton(this)),
	spacerL(new QSpacerItem(QSizePolicy::Expanding, QSizePolicy::Ignored)),
	spacerR(new QSpacerItem(QSizePolicy::Expanding, QSizePolicy::Ignored))
{
	connect(yesRadiobutton,	&QRadioButton::clicked,	this,	&BoolFilterBox::handle_radiobuttonClicked);
	connect(noRadiobutton,	&QRadioButton::clicked,	this,	&BoolFilterBox::handle_radiobuttonClicked);
	
	BoolFilterBox::setup();
	BoolFilterBox::reset();
}

BoolFilterBox::~BoolFilterBox()
{}



void BoolFilterBox::setup()
{
	yesRadiobutton->setObjectName("yesRadiobutton");
	yesRadiobutton->setText(tr("Yes"));
	yesRadiobutton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	
	noRadiobutton->setObjectName("noRadiobutton");
	noRadiobutton->setText(tr("No"));
	noRadiobutton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	
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



void BoolFilterBox::handle_radiobuttonClicked()
{
	assert(yesRadiobutton->isChecked() != noRadiobutton->isChecked());
	emit filterChanged();
}
