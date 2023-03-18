#ifndef IMAGE_DISPLAY_H
#define IMAGE_DISPLAY_H

#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>



class ImageDisplay : public QLabel
{
	Q_OBJECT
	
	QPixmap imagePixmap;
	
public:
	inline explicit ImageDisplay(QWidget* parent) :
			QLabel(parent)
	{
		setMinimumSize(1, 1);
		setScaledContents(false);
	}
	
	inline virtual int heightForWidth(int width) const override
	{
		if (imagePixmap.isNull()) {
			return height();
		}
		return ((double) imagePixmap.height() * width) / imagePixmap.width();
	}
	
	inline virtual QSize sizeHint() const override
	{
		return QSize(width(), heightForWidth(width()));
	}
	
public slots:
	inline void setPixmap(const QPixmap& pixmap)
	{
		this->imagePixmap = pixmap;
		QLabel::setPixmap(scaledPixmap());
	}
	
	inline void resizeEvent(QResizeEvent* event) override
	{
		Q_UNUSED(event);
		
		if(!imagePixmap.isNull()) {
			QLabel::setPixmap(scaledPixmap());
		}
	}
	
private:
	inline QPixmap scaledPixmap() const
	{
		return imagePixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
};



#endif // IMAGE_DISPLAY_H
