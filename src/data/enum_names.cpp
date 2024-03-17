/*
 * Copyright 2023-2024 Simon Vetter
 * 
 * This file is part of PeakAscentLogger.
 * 
 * PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with PeakAscentLogger.
 * If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file enum_names.cpp
 * 
 * This file defines the EnumNames class containing names for enumerative fields in items.
 */

#include "enum_names.h"



/**
 * Translates one-dimensional list of enum name strings.
 * 
 * @param list	List of enum name strings.
 * @return		Translated list of enum name strings.
 */
QStringList EnumNames::translateList(const QStringList& list)
{
	QStringList translatedList = QStringList();
	for (const QString& string : list) {
		QString translated = tr(string.toStdString().c_str());
		translatedList.append(translated);
	}
	return translatedList;
}



/**
 * List of continent names in English, including a "None" entry at the beginning.
 */
const QStringList EnumNames::continentNames = {
	QT_TR_NOOP("None"),
	QT_TR_NOOP("North America"),
	QT_TR_NOOP("South America"),
	QT_TR_NOOP("Europe"),
	QT_TR_NOOP("Africa"),
	QT_TR_NOOP("Asia"),
	QT_TR_NOOP("Australia"),
	QT_TR_NOOP("Antarctica")
};



/**
 * List of English descriptions for different kinds of hikes, the first of which is "Normal".
 */
const QStringList EnumNames::hikeKindNames = {
	QT_TR_NOOP("Normal"),
	QT_TR_NOOP("Snow hike"),
	QT_TR_NOOP("Snowshoe"),
	QT_TR_NOOP("Ski tour")
};



/**
 * Two-dimensional list of English names for hike difficulty systems and their difficulty grades.
 * 
 * The first dimension is the difficulty system, the second dimension is the difficulty grade.
 * The first entry in the system dimension is "None" with a list of difficulty levels which is empty
 * other than the "None" placeholder entry.
 * The first and second entries in the grade dimension are always "None" and "All" for every
 * difficulty system except the first one (which is itself "None"). For selecting a difficulty
 * grade, the first entry ("None") is used and the second one dropped, while for use in filters,
 * the second entry ("All") is used and the first one dropped.
 * 
 * The systems and their grades are taken from this source:
 * https://www.sac-cas.ch/en/ausbildung-und-sicherheit/tourenplanung/grading-systems/
 */
const QList<QPair<QString, QStringList>> EnumNames::difficultyNames = {
	qMakePair<QString, QStringList>(QT_TR_NOOP("No difficulty"),
		{
			QT_TR_NOOP("No grade")
		}),
	qMakePair<QString, QStringList>(
		QT_TR_NOOP("SAC hiking scale"),
		// https://www.sac-cas.ch/fileadmin/Ausbildung_und_Wissen/Sicher_unterwegs/Sicher_unterwegs_Wandern/2020_Berg_Alpinwanderskala_EN.pdf
		{
			QT_TR_NOOP("No grade"),
			QT_TR_NOOP("All grades"),
			QT_TR_NOOP("T1 (Hiking)"),
			QT_TR_NOOP("T2 (Mountain hiking)"),
			QT_TR_NOOP("T3 (Difficult mountain hiking)"),
			QT_TR_NOOP("T4 (Alpine hiking)"),
			QT_TR_NOOP("T5 (Difficult alpine hiking)"),
			QT_TR_NOOP("T6 (Very difficult alpine hiking)")
		}
	),
	qMakePair<QString, QStringList>(
		QT_TR_NOOP("SAC mountaneering scale"),
		{
			QT_TR_NOOP("No grade"),
			QT_TR_NOOP("All grades"),
			QT_TR_NOOP("F (Easy)"),
			QT_TR_NOOP("F+ (Easy)"),
			QT_TR_NOOP("PD- (Moderate)"),
			QT_TR_NOOP("PD (Moderate)"),
			QT_TR_NOOP("PD+ (Moderate)"),
			QT_TR_NOOP("AD- (Somewhat difficult)"),
			QT_TR_NOOP("AD (Somewhat difficult)"),
			QT_TR_NOOP("AD+ (Somewhat difficult)"),
			QT_TR_NOOP("D- (Difficult)"),
			QT_TR_NOOP("D (Difficult)"),
			QT_TR_NOOP("D+ (Difficult)"),
			QT_TR_NOOP("TD- (Very difficult)"),
			QT_TR_NOOP("TD (Very difficult)"),
			QT_TR_NOOP("TD+ (Very difficult)"),
			QT_TR_NOOP("ED- (Extremely difficult)"),
			QT_TR_NOOP("ED (Extremely difficult)"),
			QT_TR_NOOP("ED+ (Extremely difficult)"),
			QT_TR_NOOP("ABO- (Abominably difficult)"),
			QT_TR_NOOP("ABO (Abominably difficult)")
		}
	),
	qMakePair<QString, QStringList>(
		QT_TR_NOOP("SAC ski tours scale"),
		// https://www.sac-cas.ch/fileadmin/Ausbildung_und_Wissen/Sicher_unterwegs/Sicher_unterwegs_Skitour/2020_Skitourenskala_EN.pdf
		{
			QT_TR_NOOP("No grade"),
			QT_TR_NOOP("All grades"),
			QT_TR_NOOP("L (Easy)"),
			QT_TR_NOOP("WS (Moderate)"),
			QT_TR_NOOP("ZS (Somewhat difficult)"),
			QT_TR_NOOP("S (Difficult)"),
			QT_TR_NOOP("SS (Very difficult)"),
			QT_TR_NOOP("AS (Exceptionally difficult)"),
			QT_TR_NOOP("EX (Extremely dificult)")
		}
	),
	qMakePair<QString, QStringList>(
		QT_TR_NOOP("SAC snowshoe tours scale"),
		// https://www.sac-cas.ch/fileadmin/Ausbildung_und_Wissen/Sicher_unterwegs/Sicher_unterwegs_Schneeschuhtour/2020_Schneeschuhtourenskala_EN.pdf
		{
			QT_TR_NOOP("No grade"),
			QT_TR_NOOP("All grades"),
			QT_TR_NOOP("WT1 (Easy snowshoe hike)"),
			QT_TR_NOOP("WT2 (Moderate snowshoe hike)"),
			QT_TR_NOOP("WT3 (Difficult snowshoe hike)"),
			QT_TR_NOOP("WT4 (Snowshoe tour)"),
			QT_TR_NOOP("WT5 (Alpine snowshoe tour)"),
			QT_TR_NOOP("WT6 (Difficult alpine snowshoe tour)")
		}
	),
	qMakePair<QString, QStringList>(
		QT_TR_NOOP("UIAA climbing scale"),
		// https://www.sac-cas.ch/fileadmin/Ausbildung_und_Wissen/Sicher_unterwegs/Sicher_unterwegs_Klettern/SAC-Kletterskala-UIAA-v0.1.pdf
		{
			QT_TR_NOOP("No grade"),
			QT_TR_NOOP("All grades"),
			QT_TR_NOOP("I (Easy)"),
			QT_TR_NOOP("I+ (Easy)"),
			QT_TR_NOOP("II- (Moderate)"),
			QT_TR_NOOP("II (Moderate)"),
			QT_TR_NOOP("II+ (Moderate)"),
			QT_TR_NOOP("III- (Somewhat difficult)"),
			QT_TR_NOOP("III (Somewhat difficult)"),
			QT_TR_NOOP("III+ (Somewhat difficult)"),
			QT_TR_NOOP("IV- (Difficult)"),
			QT_TR_NOOP("IV (Difficult)"),
			QT_TR_NOOP("IV+ (Difficult)"),
			QT_TR_NOOP("V- (Very difficult)"),
			QT_TR_NOOP("V (Very difficult)"),
			QT_TR_NOOP("V+ (Very difficult)"),
			QT_TR_NOOP("VI- (Incredibly difficult)"),
			QT_TR_NOOP("VI (Incredibly difficult)"),
			QT_TR_NOOP("VI+ (Incredibly difficult)"),
			QT_TR_NOOP("VII- (Extremely difficult)"),
			QT_TR_NOOP("VII (Extremely difficult)"),
			QT_TR_NOOP("VII+ (Extremely difficult)"),
			"VIII-",
			"VIII",
			"VIII+",
			"IX-",
			"IX",
			"IX+",
			"X-",
			"X",
			"X+",
			"XI-",
			"XI"
		}
	)
};
