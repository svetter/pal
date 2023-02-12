#include "db_error.h"

#include <QMessageBox>



void displayError(QWidget* parent, QString error)
{
	QMessageBox::critical(parent, "Database error", error);
	exit(1);
}

void displayError(QWidget* parent, QString error, QString& queryString)
{
	return displayError(parent, error + "\n\nQuery:\n" + queryString);
}

void displayError(QWidget* parent, QSqlError error, QString& queryString)
{
	return displayError(parent, error.text(), queryString);
}

void displayError(QWidget* parent, QSqlError error)
{
	return displayError(parent, error.text());
}
