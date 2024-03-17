#include "enum_filter_box.h"



EnumFilterBox::EnumFilterBox(QWidget* parent, const QString& title, const QStringList& entries) :
	FilterBox(parent, title),
	combo(new QComboBox(this)),
	entries(entries)
{
	connect(combo,	&QComboBox::currentIndexChanged,	this,	&EnumFilterBox::filterChanged);
	
	EnumFilterBox::setup();
	EnumFilterBox::reset();
}

EnumFilterBox::~EnumFilterBox()
{}



void EnumFilterBox::setup()
{
	combo->setObjectName("combo");
	combo->insertItems(0, entries);
	
	filterLayout->addWidget(combo);
}

void EnumFilterBox::reset()
{
	FilterBox::reset();
	
	combo->setCurrentIndex(0);
}
