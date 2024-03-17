#include "string_filter_box.h"



StringFilterBox::StringFilterBox(QWidget* parent, const QString& title) :
	FilterBox(parent, title),
	lineEdit(new QLineEdit(this))
{
	connect(lineEdit,	&QLineEdit::textChanged,	this,	&StringFilterBox::filterChanged);
	
	StringFilterBox::setup();
	StringFilterBox::reset();
}

StringFilterBox::~StringFilterBox()
{}



void StringFilterBox::setup()
{
	lineEdit->setObjectName("lineEdit");
	lineEdit->setPlaceholderText(tr("Contains..."));
	
	filterLayout->addWidget(lineEdit);
}

void StringFilterBox::reset()
{
	FilterBox::reset();
	
	lineEdit	->clear();
}
