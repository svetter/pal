#ifndef RELOCATE_PHOTOS_DIALOG_H
#define RELOCATE_PHOTOS_DIALOG_H

#include "src/db/database.h"
#include "src/tools/photo_relocation_thread.h"
#include "ui_relocate_photos_dialog.h"

#include <QDialog>
#include <QThread>



class RelocatePhotosDialog : public QDialog, public Ui_RelocatePhotosDialog
{
	Q_OBJECT
	
	Database* db;
	
	bool running;
	PhotoRelocationThread* workerThread;
	
public:
	RelocatePhotosDialog(QWidget* parent, Database* db);
	
private slots:
	void handle_browseOldPath();
	void handle_browseNewPath();
	
	void handle_start();
	void handle_finished();
	void handle_abort();
	void handle_close();
	
	void handle_workloadSize(int workloadSize);
	void handle_progressUpdate(int processed);
	void handle_callback_updateFilepath(int bufferRowIndex, QString newFilepath);
	
private:
	void updateStartAbortButtons();
	
	void reject() override;
};



#endif // RELOCATE_PHOTOS_DIALOG_H
