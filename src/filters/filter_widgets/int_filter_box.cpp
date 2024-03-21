#include "int_filter_box.h"



IntFilterBox::IntFilterBox(QWidget* parent, const QString& title, unique_ptr<IntFilter> filter) :
	FilterBox(parent, Integer, title),
	filter(std::move(filter)),
	minSpinner(new QSpinBox(this)),
	maxSpinner(new QSpinBox(this)),
	setMaxCheckbox(new QCheckBox(this)),
	spacer(new QSpacerItem(5, 0, QSizePolicy::Expanding, QSizePolicy::Minimum))
{
	connect(minSpinner,		&QSpinBox::valueChanged,	this,	&IntFilterBox::handle_minChanged);
	connect(setMaxCheckbox,	&QCheckBox::stateChanged,	this,	&IntFilterBox::handle_setMaxChanged);
	connect(maxSpinner,		&QSpinBox::valueChanged,	this,	&IntFilterBox::handle_maxChanged);
	
	IntFilterBox::setup();
	IntFilterBox::reset();
}

IntFilterBox::~IntFilterBox()
{
	// Widgets deleted by layout
}



void IntFilterBox::setup()
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
}

void IntFilterBox::reset()
{
	FilterBox::reset();
	
	minSpinner		->setValue(0);
	maxSpinner		->setValue(0);
	setMaxCheckbox	->setChecked(true);
}



const Filter* IntFilterBox::getFilter() const
{
	return filter.get();
}



void IntFilterBox::handle_minChanged()
{
	if (!setMaxCheckbox->isChecked()) {
		maxSpinner->setValue(minSpinner->value());
	}
	else if (minSpinner->value() > maxSpinner->value()) {
		maxSpinner->setValue(minSpinner->value());
	}
	
	emit filterChanged();
}

void IntFilterBox::handle_maxChanged()
{
	if (minSpinner->value() > maxSpinner->value()) {
		return minSpinner->setValue(maxSpinner->value());
	}
	
	emit filterChanged();
}

void IntFilterBox::handle_setMaxChanged()
{
	maxSpinner->setEnabled(setMaxCheckbox->isChecked());
	handle_minChanged();
}
