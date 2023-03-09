#include "photo_relocation_thread.h"



PhotoRelocationThread::PhotoRelocationThread(QDialog* parent, Database* db, QString oldPrefix, QString newPrefix) :
		QThread(parent),
		parent(parent),
		db(db),
		oldPrefix(oldPrefix),
		newPrefix(newPrefix),
		workloadSize(-1),
		abortWasCalled(false)
{}



void PhotoRelocationThread::run()
{
	workloadSize = db->photosTable->getNumberOfRows();
	reportWorkloadSize(workloadSize);
	
	int updatedPaths = 0;
	
	for (int i = 0; i < workloadSize; i++) {
		if (abortWasCalled) break;
		
		QString currentPath = db->photosTable->filepathColumn->getValueAt(i).toString();
		
		if (currentPath.startsWith(oldPrefix)) {
			QString newPath = currentPath.replace(0, oldPrefix.size(), newPrefix);
			callback_updateFilepathAt(i, newPath);
			updatedPaths++;
		}
		
		emit reportProgress(i + 1, updatedPaths);
	}
}



void PhotoRelocationThread::abort() {
	abortWasCalled = true;
}
