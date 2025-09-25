/*
 * Copyright 2023-2025 Simon Vetter
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

#ifndef TAB_BEHAVIOR_MODE_H
#define TAB_BEHAVIOR_MODE_H

#include <QString>
#include <QObject>



enum AscentViewerTabBehaviorMode {
	Manual,
	PreferLastClicked,
	PreferImages,
	PreferMap,
	AlwaysImages,
	AlwaysMap
};



class AscentViewerTabBehaviorModeNames : private QObject {
	Q_OBJECT
	
	static inline QList<AscentViewerTabBehaviorMode> getEntries()
	{
		return {
			Manual,
			PreferLastClicked,
			PreferImages,
			PreferMap,
			AlwaysImages,
			AlwaysMap
		};
	}
	
public:
	static inline QString getCodeFor(AscentViewerTabBehaviorMode mode)
	{
		switch (mode) {
		case Manual:			return "manual";
		case PreferLastClicked:	return "preferLastClicked";
		case PreferImages:		return "preferImages";
		case PreferMap:			return "preferMap";
		case AlwaysImages:		return "alwaysImages";
		case AlwaysMap:			return "alwaysMap";
		}
		return QString();
	}
	
private:
	static inline QStringList getCodes()
	{
		const QList<AscentViewerTabBehaviorMode> entries = getEntries();
		QStringList translatedList = QStringList();
		for (const AscentViewerTabBehaviorMode& entry : entries) {
			const QString translated = getCodeFor(entry);
			translatedList.append(translated);
		}
		return translatedList;
	}
	
	static inline QString getUntranslatedUiStringFor(AscentViewerTabBehaviorMode mode)
	{
		switch (mode) {
		case Manual:			return QT_TR_NOOP("Manual");
		case PreferLastClicked:	return QT_TR_NOOP("Prefer last clicked");
		case PreferImages:		return QT_TR_NOOP("Prefer images");
		case PreferMap:			return QT_TR_NOOP("Prefer map");
		case AlwaysImages:		return QT_TR_NOOP("Always switch to images");
		case AlwaysMap:			return QT_TR_NOOP("Always switch to map");
		}
		return QString();
	}
	
public:
	static inline QStringList getTranslatedNames()
	{
		const QList<AscentViewerTabBehaviorMode> entries = getEntries();
		QStringList translatedList = QStringList();
		for (const AscentViewerTabBehaviorMode& entry : entries) {
			const QString translated = tr(getUntranslatedUiStringFor(entry).toStdString().c_str());
			translatedList.append(translated);
		}
		return translatedList;
	}
	
	static inline AscentViewerTabBehaviorMode parseAscentViewerTabBehaviorMode(const QString& code)
	{
		const QList<AscentViewerTabBehaviorMode> entries = getEntries();
		for (const AscentViewerTabBehaviorMode& entry : entries) {
			if (getCodeFor(entry) == code) return entry;
		}
		return AscentViewerTabBehaviorMode(-1);
	}
};



#endif // TAB_BEHAVIOR_MODE_H
