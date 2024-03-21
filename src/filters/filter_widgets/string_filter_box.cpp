#include "string_filter_box.h"



StringFilterBox::StringFilterBox(QWidget* parent, const QString& title, unique_ptr<StringFilter> filter) :
	FilterBox(parent, String, title),
	filter(std::move(filter)),
	lineEdit(new QLineEdit(this))
{
	connect(lineEdit,	&QLineEdit::textChanged,	this,	&StringFilterBox::filterChanged);
	
	StringFilterBox::setup();
	StringFilterBox::reset();
}

StringFilterBox::~StringFilterBox()
{
	// Widgets deleted by layout
}



void StringFilterBox::setup()
{
	lineEdit->setObjectName("lineEdit");
	lineEdit->setMinimumWidth(150);
	lineEdit->setPlaceholderText(tr("Contains..."));
	
	filterLayout->addWidget(lineEdit);
}

void StringFilterBox::reset()
{
	FilterBox::reset();
	
	lineEdit->clear();
}



const Filter* StringFilterBox::getFilter() const
{
	return filter.get();
}
