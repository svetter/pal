#include "bool_filter_box.h"



BoolFilterBox::BoolFilterBox(QWidget* parent, const QString& title) :
	FilterBox(parent, title),
	yesRadiobutton(new QRadioButton(this)),
	noRadiobutton(new QRadioButton(this)),
	absentRadiobutton(new QRadioButton(this))
{
	connect(yesRadiobutton,	&QRadioButton::clicked,	this,	&BoolFilterBox::handle_radiobuttonClicked);
	connect(noRadiobutton,		&QRadioButton::clicked,	this,	&BoolFilterBox::handle_radiobuttonClicked);
	connect(absentRadiobutton,	&QRadioButton::clicked,	this,	&BoolFilterBox::handle_radiobuttonClicked);
	
	BoolFilterBox::setup();
	BoolFilterBox::reset();
}

BoolFilterBox::~BoolFilterBox()
{}



void BoolFilterBox::setup()
{
	yesRadiobutton->setObjectName("yesRadiobutton");
	yesRadiobutton->setText(tr("Yes"));
	
	noRadiobutton->setObjectName("noRadiobutton");
	noRadiobutton->setText(tr("No"));
	
	absentRadiobutton->setObjectName("absentRadiobutton");
	absentRadiobutton->setText(tr("Absent"));
	
	filterLayout->addWidget(yesRadiobutton);
	filterLayout->addWidget(noRadiobutton);
	filterLayout->addWidget(absentRadiobutton);
}

void BoolFilterBox::reset()
{
	FilterBox::reset();
	
	yesRadiobutton		->setChecked(true);
	noRadiobutton		->setChecked(false);
	absentRadiobutton	->setChecked(false);
}



void BoolFilterBox::handle_radiobuttonClicked()
{
	assert((int) yesRadiobutton->isChecked() + (int) noRadiobutton->isChecked() + (int) absentRadiobutton->isChecked() == 1);
	emit filterChanged();
}
