#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();
	
private:
	Ui::MainWindow *ui;
	void handle_newAscent();
	void handle_newPeak();
	void handle_newTrip();
	void handle_newRegion();
	void handle_newRange();
	void handle_newCountry();
};



#endif // MAIN_WINDOW_H
