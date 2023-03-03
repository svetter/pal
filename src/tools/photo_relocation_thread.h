#ifndef PHOTO_RELOCATION_THREAD_H
#define PHOTO_RELOCATION_THREAD_H

#include "src/db/database.h"

#include <QThread>
#include <QDialog>



class PhotoRelocationThread : public QThread
{
	Q_OBJECT
	
	QDialog* parent;
	
	Database* db;
	QString oldPrefix;
	QString newPrefix;
	
	int workloadSize;
	bool abortWasCalled;
	
public:
	PhotoRelocationThread(QDialog* parent, Database* db, QString oldPrefix, QString newPrefix);
	
	void run() override;
	void abort();
	
signals:
	int reportWorkloadSize(int workloadSize);
	void reportProgress(int processed);
	
	void callback_updateFilepathAt(int bufferRowIndex, QString newFilepath);
};



#endif // PHOTO_RELOCATION_THREAD_H
