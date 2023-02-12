#ifndef DB_ERROR_H
#define DB_ERROR_H

#include <QWidget>
#include <QString>
#include <QSqlError>



void displayError(QWidget* parent, QString error);
void displayError(QWidget* parent, QString error, QString& queryString);
void displayError(QWidget* parent, QSqlError error);
void displayError(QWidget* parent, QSqlError error, QString& queryString);



#endif // DB_ERROR_H
