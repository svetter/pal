#ifndef FILTER_BOX_H
#define FILTER_BOX_H

#include "ui_filter_box.h"

#include <QGroupBox>
#include <QButtonGroup>



class FilterBox : public QGroupBox, public Ui_FilterBox
{
	Q_OBJECT
	
	const QString title;
	
	QButtonGroup invertButtonGroup;
	
protected:
	explicit FilterBox(QWidget* parent, const QString& title) :
		QGroupBox(parent),
		title(title),
		invertButtonGroup(QButtonGroup())
	{
		setupUi(this);
		
		setTitle(title);
		
		invertButtonGroup.addButton(includeRadiobutton);
		invertButtonGroup.addButton(excludeRadiobutton);
		
		connect(includeRadiobutton,	&QRadioButton::clicked,	this,	&FilterBox::handle_invertChanged);
		connect(excludeRadiobutton,	&QRadioButton::clicked,	this,	&FilterBox::handle_invertChanged);
		
		FilterBox::reset();
	}
public:
	inline virtual ~FilterBox()
	{}
	
	virtual void setup() = 0;
	inline virtual void reset() {
		includeRadiobutton->setChecked(true);
		excludeRadiobutton->setChecked(false);
	}
	
private slots:
	inline void handle_invertChanged()
	{
		assert(includeRadiobutton->isChecked() != excludeRadiobutton->isChecked());
		
		emit filterChanged();
	}
	
signals:
	void filterChanged();
};



#endif // FILTER_BOX_H
