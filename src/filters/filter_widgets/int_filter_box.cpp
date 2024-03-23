#include "int_filter_box.h"



IntFilterBox::IntFilterBox(QWidget* parent, const QString& title, IntFilter& filter) :
	FilterBox(parent, Integer, title, filter),
	filter(filter),
	minSpinner(new QSpinBox(this)),
	maxSpinner(new QSpinBox(this)),
	setMaxCheckbox(new QCheckBox(this)),
	spacer(new QSpacerItem(5, 0, QSizePolicy::Expanding, QSizePolicy::Minimum))
{
	minSpinner->setObjectName("minSpinner");
	minSpinner->setMaximum(9999);
	
	setMaxCheckbox->setObjectName("setMaxCheckbox");
	setMaxCheckbox->setToolTip(tr("Set a maximum value"));
	
	maxSpinner->setObjectName("maxSpinner");
	maxSpinner->setMaximum(9999);
	
	filterLayout->addWidget(minSpinner);
	filterLayout->addItem(spacer);
	filterLayout->addWidget(setMaxCheckbox);
	filterLayout->addWidget(maxSpinner);
	
	connect(minSpinner,		&QSpinBox::valueChanged,	this,	&IntFilterBox::handle_minChanged);
	connect(setMaxCheckbox,	&QCheckBox::stateChanged,	this,	&IntFilterBox::handle_setMaxChanged);
	connect(maxSpinner,		&QSpinBox::valueChanged,	this,	&IntFilterBox::handle_maxChanged);
	
	minSpinner->setValue(filter.min);
	maxSpinner->setValue(filter.max);
	setMaxCheckbox->setChecked(filter.min < filter.max);
}

IntFilterBox::~IntFilterBox()
{
	// Widgets deleted by layout
}



void IntFilterBox::updateFilterTypeSpecific()
{
	filter.min = minSpinner->value();
	filter.max = maxSpinner->value();
	emit filterChanged();
}



const Filter& IntFilterBox::getFilter() const
{
	return filter;
}



void IntFilterBox::handle_minChanged()
{
	if (!setMaxCheckbox->isChecked()) {
		maxSpinner->setValue(minSpinner->value());
	}
	else if (minSpinner->value() > maxSpinner->value()) {
		maxSpinner->setValue(minSpinner->value());
	}
	
	updateFilterTypeSpecific();
}

void IntFilterBox::handle_maxChanged()
{
	if (minSpinner->value() > maxSpinner->value()) {
		return minSpinner->setValue(maxSpinner->value());
	}
	
	updateFilterTypeSpecific();
}

void IntFilterBox::handle_setMaxChanged()
{
	maxSpinner->setEnabled(setMaxCheckbox->isChecked());
	handle_minChanged();
}
