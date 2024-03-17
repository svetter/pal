#include "dual_enum_filter_box.h"

#include "src/data/enum_names.h"



DualEnumFilterBox::DualEnumFilterBox(QWidget* parent, const QString& title, const QList<QPair<QString, QStringList>>& entries) :
	FilterBox(parent, title),
	comboDiscerning(new QComboBox(this)),
	comboDependant(new QComboBox(this)),
	entries(entries)
{
	connect(comboDiscerning,	&QComboBox::currentIndexChanged,	this,	&DualEnumFilterBox::handle_discerningComboChanged);
	connect(comboDependant,		&QComboBox::currentIndexChanged,	this,	&DualEnumFilterBox::filterChanged);
	
	DualEnumFilterBox::setup();
	DualEnumFilterBox::reset();
}

DualEnumFilterBox::~DualEnumFilterBox()
{}



void DualEnumFilterBox::setup()
{
	comboDiscerning->setObjectName("comboDiscerning");
	comboDiscerning->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);
	
	QStringList discerningEntries = QStringList();
	std::transform(
		entries.constBegin(),
		entries.constEnd(),
		std::back_inserter(discerningEntries),
		[](QPair<QString, QStringList> qPair){ return qPair.first; }
		);
	comboDiscerning->insertItems(0, EnumNames::translateList(discerningEntries));
	
	comboDependant->setObjectName("comboDependant");
	comboDependant->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);
	
	filterLayout->addWidget(comboDiscerning);
	filterLayout->addWidget(comboDependant);
}

void DualEnumFilterBox::reset()
{
	FilterBox::reset();
	
	comboDiscerning->setCurrentIndex(0);
	comboDependant->setCurrentIndex(0);
}



void DualEnumFilterBox::handle_discerningComboChanged()
{
	const int discerningIndex = comboDiscerning->currentIndex();
	const bool dependantEnabled = discerningIndex > 0;
	comboDependant->setEnabled(dependantEnabled);
	
	comboDependant->clear();
	if (dependantEnabled) {
		QStringList translatedEntries = EnumNames::translateList(entries.at(discerningIndex).second);
		translatedEntries.removeAt(0);
		comboDependant->insertItems(0, translatedEntries);
		comboDependant->setCurrentIndex(0);
	} else {
		comboDependant->setPlaceholderText(EnumNames::translateList(entries.at(0).second).at(0));
	}
}
