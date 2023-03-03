#include "relocate_photos_window.h"

#include "src/dialogs/parse_helper.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>



RelocatePhotosWindow::RelocatePhotosWindow(QWidget* parent, Database* db) :
		QDialog(parent),
		db(db),
		running(false)
{
	setupUi(this);
	
	abortButton->setEnabled(false);
	
	
	connect(bottomButtonBox->button(QDialogButtonBox::Close),	&QPushButton::clicked,	this,	&ProjectSettingsWindow::handle_close);
	
	connect(oldPathBrowseButton,	&QPushButton::clicked,	this,	&ProjectSettingsWindow::handle_browseOldPath);
	connect(newPathBrowseButton,	&QPushButton::clicked,	this,	&ProjectSettingsWindow::handle_browseNewPath);
	
	connect(startButton,			&QPushButton::clicked,	this,	&ProjectSettingsWindow::handle_start);
	connect(abortButton,			&QPushButton::clicked,	this,	&ProjectSettingsWindow::handle_abort);
}



void RelocatePhotosWindow::handle_browseOldPath()
{
	int newHikerIndex = openNewHikerDialogAndStore(this, db);
	if (newHikerIndex < 0) return;
	defaultHikerCombo->setCurrentIndex(newHikerIndex + 1);	// 0 is None
}



void RelocatePhotosWindow::handle_start()
{
	
}

void RelocatePhotosWindow::handle_abort()
{
	
}

void RelocatePhotosWindow::handle_close()
{
	QMessageBox::StandardButton resultButton = QMessageBox::Yes;
	
	if (running) {
		QString title = tr("Operation running");
		QString question = tr("Do you want to abort the running process?");
		auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		auto selected = QMessageBox::Cancel;
		resultButton = QMessageBox::question(this, title, question, options, selected);
	}
	
	if (resultButton == QMessageBox::Yes) {
		handle_abort();
		accept();
	}
}



void RelocatePhotosWindow::reject()
{
	handle_close();
}