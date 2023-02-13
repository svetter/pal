#include "ascent.h"
#include "range.h"



const QStringList Range::continentNames = {
	tr("None"),
	tr("North America"),
	tr("South America"),
	tr("Europe"),
	tr("Africa"),
	tr("Asia"),
	tr("Australia"),
	tr("Antarctica")
};



const QStringList Ascent::hikeKindNames = {
	tr("Normal"),
	tr("Snow hike"),
	tr("Snowshoe"),
	tr("Ski tour")
};



// https://www.sac-cas.ch/en/ausbildung-und-sicherheit/tourenplanung/grading-systems/
const QList<QPair<QString, QStringList>> Ascent::difficultyNames = {
	qMakePair<QString, QStringList>(tr("None"), {}),
	qMakePair<QString, QStringList>(
		tr("SAC hiking scale"),
		// https://www.sac-cas.ch/fileadmin/Ausbildung_und_Wissen/Sicher_unterwegs/Sicher_unterwegs_Wandern/2020_Berg_Alpinwanderskala_EN.pdf
		{
			tr("None"),
			tr("T1 (Hiking)"),
			tr("T2 (Mountain hiking)"),
			tr("T3 (Difficult mountain hiking)"),
			tr("T4 (Alpine hiking)"),
			tr("T5 (Difficult alpine hiking)"),
			tr("T6 (Very difficult alpine hiking)")
		}
	),
	qMakePair<QString, QStringList>(
		tr("SAC mountaneering scale"),
		{
			tr("None"),
			tr("F (Easy)"),
			tr("PD (Moderate)"),
			tr("AD (Somewhat difficult)"),
			tr("D (Difficult)"),
			tr("TD (Very difficult)"),
			tr("ED (Extremely difficult)"),
			tr("ABO (Abominably difficult)")
		}
	),
	qMakePair<QString, QStringList>(
		tr("SAC ski tours scale"),
		// https://www.sac-cas.ch/fileadmin/Ausbildung_und_Wissen/Sicher_unterwegs/Sicher_unterwegs_Skitour/2020_Skitourenskala_EN.pdf
		{
			tr("None"),
			tr("L (Easy)"),
			tr("WS (Moderate)"),
			tr("ZS (Somewhat difficult)"),
			tr("S (Difficult)"),
			tr("SS (Very difficult)"),
			tr("AS (Exceptionally difficult)"),
			tr("EX (Extremely dificult)")
		}
	),
	qMakePair<QString, QStringList>(
		tr("SAC snowshoe tours scale"),
		// https://www.sac-cas.ch/fileadmin/Ausbildung_und_Wissen/Sicher_unterwegs/Sicher_unterwegs_Schneeschuhtour/2020_Schneeschuhtourenskala_EN.pdf
		{
			tr("None"),
			tr("WT1 (Easy snowshoe hike)"),
			tr("WT2 (Moderate snowshoe hike)"),
			tr("WT3 (Difficult snowshoe hike)"),
			tr("WT4 (Snowshoe tour)"),
			tr("WT5 (Alpine snowshoe tour)"),
			tr("WT6 (Difficult alpine snowshoe tour)")
		}
	),
	qMakePair<QString, QStringList>(
		tr("UIAA climbing scale"),
		// https://www.sac-cas.ch/fileadmin/Ausbildung_und_Wissen/Sicher_unterwegs/Sicher_unterwegs_Klettern/SAC-Kletterskala-UIAA-v0.1.pdf
		{
			tr("None"),
			tr("I (Easy)"),
			tr("II (Moderate)"),
			tr("III (Somewhat difficult)"),
			tr("IV (Difficult)"),
			tr("V (Very difficult)"),
			tr("VI (Incredibly difficult)"),
			tr("VII (Extremely difficult)"),
			tr("VIII"),
			tr("IX"),
			tr("X"),
			tr("XI")
		}
	)
};
