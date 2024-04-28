#include "int_class_filter_box.h"



IntClassFilterBox::IntClassFilterBox(QWidget* parent, const QString& title, int classIncrement, int classMinValue, int classMaxValue, IntFilter& filter) :
	FilterBox(parent, Integer, title, filter),
	filter(filter),
	classIncrement(classIncrement),
	classMinValue(classMinValue),
	classMaxValue(classMaxValue),
	minCombo(new QComboBox(this)),
	maxCombo(new QComboBox(this)),
	setMaxCheckbox(new QCheckBox(this)),
	spacer(new QSpacerItem(5, 0, QSizePolicy::Expanding, QSizePolicy::Minimum))
{
	QStringList minComboEntries = QStringList();
	QStringList maxComboEntries = QStringList();
	for (int value = classMinValue; value < classMaxValue; value += classIncrement) {
		minComboEntries.append(QString::number(value));
		maxComboEntries.append(QString::number(value + classIncrement - 1));
	}
	
	minCombo->setObjectName("minCombo");
	minCombo->addItems(minComboEntries);
	
	setMaxCheckbox->setObjectName("setMaxCheckbox");
	setMaxCheckbox->setToolTip(tr("Set a maximum class"));
	setMaxCheckbox->setChecked(false);
	
	maxCombo->setObjectName("maxCombo");
	maxCombo->setEnabled(false);
	maxCombo->addItems(maxComboEntries);
	
	filterLayout->addWidget(minCombo);
	filterLayout->addItem(spacer);
	filterLayout->addWidget(setMaxCheckbox);
	filterLayout->addWidget(maxCombo);
	
	connect(minCombo,		&QComboBox::currentIndexChanged,	this,	&IntClassFilterBox::handle_minChanged);
	connect(setMaxCheckbox,	&QCheckBox::stateChanged,			this,	&IntClassFilterBox::handle_setMaxChanged);
	connect(maxCombo,		&QComboBox::currentIndexChanged,	this,	&IntClassFilterBox::handle_maxChanged);
	
	minCombo->setCurrentIndex(filter.min / classIncrement - classMinValue / classIncrement);
	maxCombo->setCurrentIndex(filter.max / classIncrement - classMinValue / classIncrement);
	setMaxCheckbox->setChecked(minCombo->currentIndex() < maxCombo->currentIndex());
}

IntClassFilterBox::~IntClassFilterBox()
{
	// Widgets deleted by layout
}



void IntClassFilterBox::updateFilterTypeSpecific()
{
	const int newMin = (minCombo->currentIndex() + classMinValue / classIncrement    ) * classIncrement;
	const int newMax = (maxCombo->currentIndex() + classMinValue / classIncrement + 1) * classIncrement - 1;
	filter.min = newMin;
	filter.max = newMax;
	emit filterChanged();
}



const Filter& IntClassFilterBox::getFilter() const
{
	return filter;
}



void IntClassFilterBox::handle_minChanged()
{
	if (!setMaxCheckbox->isChecked()) {
		maxCombo->setCurrentIndex(minCombo->currentIndex());
	}
	else if (minCombo->currentIndex() > maxCombo->currentIndex()) {
		maxCombo->setCurrentIndex(minCombo->currentIndex());
	}
	
	updateFilterTypeSpecific();
}

void IntClassFilterBox::handle_maxChanged()
{
	if (minCombo->currentIndex() > maxCombo->currentIndex()) {
		return minCombo->setCurrentIndex(maxCombo->currentIndex());
	}
	
	updateFilterTypeSpecific();
}

void IntClassFilterBox::handle_setMaxChanged()
{
	maxCombo->setEnabled(setMaxCheckbox->isChecked());
	handle_minChanged();
}
