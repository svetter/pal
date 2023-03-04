#include "relocate_photos_dialog.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>



RelocatePhotosDialog::RelocatePhotosDialog(QWidget* parent, Database* db) :
		QDialog(parent),
		db(db),
		running(false)
{
	setupUi(this);
	
	updateEnableUI();
	
	
	connect(bottomButtonBox->button(QDialogButtonBox::Close),	&QPushButton::clicked,	this,	&RelocatePhotosDialog::handle_close);
	
	connect(oldPathBrowseButton,	&QPushButton::clicked,		this,	&RelocatePhotosDialog::handle_browseOldPath);
	connect(newPathBrowseButton,	&QPushButton::clicked,		this,	&RelocatePhotosDialog::handle_browseNewPath);
	
	connect(oldPathLineEdit,		&QLineEdit::textChanged,	this,	&RelocatePhotosDialog::updateEnableUI);
	connect(newPathLineEdit,		&QLineEdit::textChanged,	this,	&RelocatePhotosDialog::updateEnableUI);
	
	connect(startButton,			&QPushButton::clicked,		this,	&RelocatePhotosDialog::handle_start);
	connect(abortButton,			&QPushButton::clicked,		this,	&RelocatePhotosDialog::handle_abort);
}



void RelocatePhotosDialog::handle_browseOldPath()
{
	QString caption = tr("Select old location of photos");
	QString preSelectedDir = oldPathLineEdit->text();
	if (preSelectedDir.isEmpty()) preSelectedDir = newPathLineEdit->text();
	if (preSelectedDir.isEmpty()) preSelectedDir = QDir::homePath();
	QString oldPath = QFileDialog::getExistingDirectory(this, caption, preSelectedDir);
	if (!oldPath.isEmpty()) oldPathLineEdit->setText(oldPath);
}

void RelocatePhotosDialog::handle_browseNewPath()
{
	QString caption = tr("Select new location of photos");
	QString preSelectedDir = newPathLineEdit->text();
	if (preSelectedDir.isEmpty()) preSelectedDir = oldPathLineEdit->text();
	if (preSelectedDir.isEmpty()) preSelectedDir = QDir::homePath();
	QString newPath = QFileDialog::getExistingDirectory(this, caption, preSelectedDir);
	if (!newPath.isEmpty()) oldPathLineEdit->setText(newPath);
}



void RelocatePhotosDialog::handle_start()
{
	assert(!running);
	
	running = true;
	updateEnableUI();
	
	progressBar->setMinimum(0);
	progressBar->setValue(0);
	
	workerThread = new PhotoRelocationThread(this, db, oldPathLineEdit->text(), newPathLineEdit->text());
	connect(workerThread, &PhotoRelocationThread::reportWorkloadSize,			this,	&RelocatePhotosDialog::handle_workloadSize);
	connect(workerThread, &PhotoRelocationThread::reportProgress,				this,	&RelocatePhotosDialog::handle_progressUpdate);
	connect(workerThread, &PhotoRelocationThread::callback_updateFilepathAt,	this,	&RelocatePhotosDialog::handle_callback_updateFilepath);
	connect(workerThread, &PhotoRelocationThread::finished,						this,	&RelocatePhotosDialog::handle_finished);
	
	workerThread->start();
}

void RelocatePhotosDialog::handle_finished()
{
	assert(running);
	
	workerThread->wait();
	
	workerThread->deleteLater();
	workerThread = nullptr;
	
	running = false;
	updateEnableUI();
}

void RelocatePhotosDialog::handle_abort()
{
	assert(running);
	
	workerThread->abort();
}

void RelocatePhotosDialog::handle_close()
{
	if (!running) return accept();
	
	QMessageBox::StandardButton resultButton;
	
	QString title = tr("Operation running");
	QString question = tr("Do you want to abort the running process?");
	auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
	auto selected = QMessageBox::Cancel;
	resultButton = QMessageBox::question(this, title, question, options, selected);
	
	if (resultButton == QMessageBox::Yes) {
		handle_abort();
		accept();
	}
}



void RelocatePhotosDialog::handle_workloadSize(int workloadSize)
{
	progressBar->setMaximum(workloadSize);
}

void RelocatePhotosDialog::handle_progressUpdate(int processed, int updated)
{
	progressBar->setValue(processed);
	feedbackLabel->setText(tr("Photo locations updated: %1").arg(updated));
}

void RelocatePhotosDialog::handle_callback_updateFilepath(int bufferRowIndex, QString newFilepath)
{
	return db->photosTable->updateFilepathAt(this, bufferRowIndex, newFilepath);
}



void RelocatePhotosDialog::updateEnableUI()
{
	oldPathLineEdit->setEnabled(!running);
	newPathLineEdit->setEnabled(!running);
	bool canStart = !oldPathLineEdit->text().isEmpty() || !newPathLineEdit->text().isEmpty();
	startButton->setEnabled(!running && canStart);
	abortButton->setEnabled(running);
}



void RelocatePhotosDialog::reject()
{
	handle_close();
}
