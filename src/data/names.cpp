#include "ascent.h"
#include "range.h"

#include <QTranslator>



const QStringList Peak::continentNames = {
	QObject::tr("North America"),
	QObject::tr("South America"),
	QObject::tr("Europe"),
	QObject::tr("Africa"),
	QObject::tr("Asia"),
	QObject::tr("Australia"),
	QObject::tr("Antarctica")
};



const QStringList Ascent::hikeKindNames = {
	QObject::tr("Normal"),
	QObject::tr("Snow hike"),
	QObject::tr("Snowshoe"),
	QObject::tr("Ski tour")
};



// https://www.sac-cas.ch/en/ausbildung-und-sicherheit/tourenplanung/grading-systems/
const QList<QPair<QString, QStringList>> Ascent::difficultyNames = {
	QPair(
		QObject::tr("SAC hiking scale"),
		{
			// SAC hiking scale
			// https://www.sac-cas.ch/fileadmin/Ausbildung_und_Wissen/Sicher_unterwegs/Sicher_unterwegs_Wandern/2020_Berg_Alpinwanderskala_EN.pdf
			QObject::tr("T1 (Hiking)"),
			QObject::tr("T2 (Mountain hiking)"),
			QObject::tr("T3 (Difficult mountain hiking)"),
			QObject::tr("T4 (Alpine hiking)"),
			QObject::tr("T5 (Difficult alpine hiking)"),
			QObject::tr("T6 (Very difficult alpine hiking)")
		}
	),
	QPair(
		QObject::tr("SAC mountaneering scale"),
		{
			// SAC mountaineering scale
			QObject::tr("F (Easy)"),
			QObject::tr("PD (Moderate)"),
			QObject::tr("AD (Somewhat difficult)"),
			QObject::tr("D (Difficult)"),
			QObject::tr("TD (Very difficult)"),
			QObject::tr("ED (Extremely difficult)"),
			QObject::tr("ABO (Abominably difficult)")
		},
	),
	QPair(
		QObject::tr("SAC ski tours scale"),
		{
			// SAC ski tours scale
			// https://www.sac-cas.ch/fileadmin/Ausbildung_und_Wissen/Sicher_unterwegs/Sicher_unterwegs_Skitour/2020_Skitourenskala_EN.pdf
			QObject::tr("L (Easy)"),
			QObject::tr("WS (Moderate)"),
			QObject::tr("ZS (Somewhat difficult)"),
			QObject::tr("S (Difficult)"),
			QObject::tr("SS (Very difficult)"),
			QObject::tr("AS (Exceptionally difficult)"),
			QObject::tr("EX (Extremely dificult)")
		},
	),
	QPair(
		QObject::tr("SAC snowshoe tours scale"),
		{
			// SAC snowshoe tours scale
			// https://www.sac-cas.ch/fileadmin/Ausbildung_und_Wissen/Sicher_unterwegs/Sicher_unterwegs_Schneeschuhtour/2020_Schneeschuhtourenskala_EN.pdf
			QObject::tr("WT1 (Easy snowshoe hike)"),
			QObject::tr("WT2 (Moderate snowshoe hike)"),
			QObject::tr("WT3 (Difficult snowshoe hike)"),
			QObject::tr("WT4 (Snowshoe tour)"),
			QObject::tr("WT5 (Alpine snowshoe tour)"),
			QObject::tr("WT6 (Difficult alpine snowshoe tour)")
		},
	),
	QPair(
		QObject::tr("UIAA climbing scale"),
		{
			// UIAA climbing scale
			// https://www.sac-cas.ch/fileadmin/Ausbildung_und_Wissen/Sicher_unterwegs/Sicher_unterwegs_Klettern/SAC-Kletterskala-UIAA-v0.1.pdf
			QObject::tr("I (Easy)"),
			QObject::tr("II (Moderate)"),
			QObject::tr("III (Somewhat difficult)"),
			QObject::tr("IV (Difficult)"),
			QObject::tr("V (Very difficult)"),
			QObject::tr("VI (Incredibly difficult)"),
			QObject::tr("VII (Extremely difficult)"),
			QObject::tr("VIII"),
			QObject::tr("IX"),
			QObject::tr("X"),
			QObject::tr("XI")
		}
	)
};
