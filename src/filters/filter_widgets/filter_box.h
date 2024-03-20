#ifndef FILTER_BOX_H
#define FILTER_BOX_H

#include "src/db/column.h"
#include "ui_filter_box.h"

#include <QGroupBox>
#include <QToolButton>
#include <QButtonGroup>
#include <QStyle>



class FilterBox : public QGroupBox, public Ui_FilterBox
{
	Q_OBJECT
	
public:
	const DataType type;
	const QString title;
	
private:
	QToolButton* removeButton;
	QButtonGroup invertButtonGroup;
	
protected:
	inline explicit FilterBox(QWidget* parent, DataType type, const QString& title) :
		QGroupBox(parent),
		type(type),
		title(title),
		removeButton(new QToolButton(parent)),
		invertButtonGroup(QButtonGroup())
	{
		setupUi(this);
		
		setTitle(title);
		
		invertButtonGroup.addButton(includeRadiobutton);
		invertButtonGroup.addButton(excludeRadiobutton);
		
		removeButton->setObjectName("filterRemoveButton");
		removeButton->setIcon(style()->standardIcon(QStyle::SP_DockWidgetCloseButton));
		removeButton->setFixedSize(12, 12);
		
		connect(includeRadiobutton,	&QRadioButton::clicked,	this,	&FilterBox::handle_invertChanged);
		connect(excludeRadiobutton,	&QRadioButton::clicked,	this,	&FilterBox::handle_invertChanged);
		connect(removeButton,		&QToolButton::clicked,	this,	&FilterBox::handle_removeButtonPressed);
		
		FilterBox::reset();
	}
	
public:
	inline virtual ~FilterBox()
	{
		removeButton->deleteLater();
	}
	
	virtual void setup() = 0;
	inline virtual void reset() {
		includeRadiobutton->setChecked(true);
		excludeRadiobutton->setChecked(false);
	}
	
	inline void positionRemoveButton()
	{
		const int removeButtonX = frameGeometry().right()   - removeButton->width()  / 2;
		const int removeButtonY = frameGeometry().top() + 9 - removeButton->height() / 2;
		removeButton->move(removeButtonX, removeButtonY);
		removeButton->setVisible(true);
	}
	
private slots:
	inline void handle_invertChanged()
	{
		assert(includeRadiobutton->isChecked() != excludeRadiobutton->isChecked());
		
		emit filterChanged();
	}
	
	inline void handle_removeButtonPressed()
	{
		emit removeRequested();
	}
	
protected:
	inline virtual void resizeEvent(QResizeEvent* event) override
	{
		QGroupBox::resizeEvent(event);
		positionRemoveButton();
	}
	
	inline virtual void moveEvent(QMoveEvent* event) override
	{
		QGroupBox::moveEvent(event);
		positionRemoveButton();
	}
	
signals:
	void filterChanged();
	void removeRequested();
};



#endif // FILTER_BOX_H
