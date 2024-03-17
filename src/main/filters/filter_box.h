#ifndef FILTER_BOX_H
#define FILTER_BOX_H

#include "ui_filter_box.h"

#include <QGroupBox>
#include <QToolButton>
#include <QButtonGroup>
#include <QStyle>



class FilterBox : public QGroupBox, public Ui_FilterBox
{
	Q_OBJECT
	
	const QString title;
	
	QToolButton* removeButton;
	QButtonGroup invertButtonGroup;
	
protected:
	inline explicit FilterBox(QWidget* parent, const QString& title) :
		QGroupBox(parent),
		title(title),
		removeButton(new QToolButton(parent)),
		invertButtonGroup(QButtonGroup())
	{
		setupUi(this);
		
		setTitle(title);
		
		invertButtonGroup.addButton(includeRadiobutton);
		invertButtonGroup.addButton(excludeRadiobutton);
		
		removeButton->setIcon(style()->standardIcon(QStyle::SP_DockWidgetCloseButton));
		removeButton->setFixedSize(12, 12);
		
		connect(includeRadiobutton,	&QRadioButton::clicked,	this,	&FilterBox::handle_invertChanged);
		connect(excludeRadiobutton,	&QRadioButton::clicked,	this,	&FilterBox::handle_invertChanged);
		connect(removeButton,		&QToolButton::clicked,	this,	&FilterBox::removeRequested);
		
		FilterBox::reset();
	}
	
public:
	inline virtual ~FilterBox()
	{
		delete removeButton;
	}
	
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
	
	inline void resizeEvent(QResizeEvent* event) override
	{
		removeButton->move(x() + width() - removeButton->width() + 5, y() + 3);
		QGroupBox::resizeEvent(event);
	}
	
signals:
	void filterChanged();
	void removeRequested();
};



#endif // FILTER_BOX_H
