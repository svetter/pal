#include "table.h"

#include "db_error.h"

#include <QSqlQuery>



Table::Table(QString name, QString uiName, bool isAssociative) :
		name(name),
		uiName(uiName),
		associative(isAssociative),
		buffer(nullptr)
{}

Table::~Table()
{
	deleteBuffer();
}


QString Table::getName()
{
	return name;
}
QString Table::getUIName()
{
	return uiName;
}

bool Table::isAssociative()
{
	return associative;
}



void Table::deleteBuffer()
{
	if (buffer) {
		qDeleteAll(*buffer);
		delete buffer;
	}
}

void Table::initBuffer(QWidget* parent)
{
	deleteBuffer();
	buffer = getAllEntriesFromSql(parent);
}

const QList<QVariant>* Table::getBufferRow(int rowIndex) const
{
	return buffer->at(rowIndex);
}



QString Table::getColumnListString() const
{
	return getColumnListStringOf(getColumnList());
}

int Table::getColumnIndex(Column* column) const
{
	return getColumnList().indexOf(column);
}



int Table::addRow(QWidget* parent, const QList<QVariant>& data, const QList<Column*>& columns)
{	
	int numColumns = columns.size();
	assert(!isAssociative() && numColumns == (getNumberOfColumns() - 1) || isAssociative() && numColumns == getNumberOfColumns());
	assert(data.size() == numColumns);
	
	// Add data to SQL database
	int newRowID = addRowToSql(parent, data, columns);
	
	// Update buffer
	QList<QVariant>* newBufferRow = new QList<QVariant>(data);
	newBufferRow->insert(0, newRowID);
	buffer->append(newBufferRow);
	
	return buffer->size() - 1;
}



QList<QList<QVariant>*>* Table::getAllEntriesFromSql(QWidget* parent) const
{
	QString queryString = QString(
			"SELECT " + getColumnListString() +
			"\nFROM " + name
	);
	QSqlQuery query = QSqlQuery();
	query.setForwardOnly(true);
	QList<QList<QVariant>*>* buffer = new QList<QList<QVariant>*>();
	
	if (!query.exec(queryString))
		displayError(parent, query.lastError(), queryString);
	
	QList<Column*> columns = getColumnList();
	
	int rowIndex = 0;
	while (query.next()) {
		buffer->append(new QList<QVariant>());
		int columnIndex = 0;
		for (auto columnIter = columns.constBegin(); columnIter!= columns.constEnd(); columnIter++) {
			QVariant value = query.value(columnIndex);
			Column* column = *columnIter;
			assert(column->isNullable() || !value.isNull());
			buffer->at(rowIndex)->append(value);
			columnIndex++;
		}
		rowIndex++;
	}
	
	if (buffer->empty())
		displayError(parent, "Couldn't read record from SQL query", queryString);
	return buffer;
}

int Table::addRowToSql(QWidget* parent, const QList<QVariant>& data, const QList<Column*>& columns)
{
	QString questionMarks = "";
	for (int i = 0; i < columns.size(); i++) {
		questionMarks = questionMarks + ((i == 0) ? "?" : ", ?");
	}
	QString queryString = QString(
			"INSERT INTO " + getName() + "(" + getColumnListStringOf(columns) + ")" +
			"\nVALUES(" + questionMarks + ")"
	);
	QSqlQuery query = QSqlQuery();
	if (!query.prepare(queryString))
		displayError(parent, query.lastError(), queryString);
	for (auto iter = data.constBegin(); iter != data.constEnd(); iter++) {
		query.addBindValue(*iter);
	}
	
	if (!query.exec())
		displayError(parent, query.lastError(), queryString);
	
	int newRowID = query.lastInsertId().toInt();
	assert(newRowID > 0);
	return newRowID;
}
