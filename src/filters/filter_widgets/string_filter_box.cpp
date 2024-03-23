#include "string_filter_box.h"



StringFilterBox::StringFilterBox(QWidget* parent, const QString& title, StringFilter& filter) :
	FilterBox(parent, String, title, filter),
	filter(filter),
	lineEdit(new QLineEdit(this))
{
	lineEdit->setObjectName("lineEdit");
	lineEdit->setMinimumWidth(150);
	lineEdit->setPlaceholderText(tr("Contains..."));
	
	filterLayout->addWidget(lineEdit);
	
	connect(lineEdit,	&QLineEdit::textChanged,	this,	&StringFilterBox::updateFilterTypeSpecific);
	
	lineEdit->setText(filter.value);
}

StringFilterBox::~StringFilterBox()
{
	// Widgets deleted by layout
}



void StringFilterBox::updateFilterTypeSpecific()
{
	filter.value = lineEdit->text();
	emit filterChanged();
}



const Filter& StringFilterBox::getFilter() const
{
	return filter;
}
