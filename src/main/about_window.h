/*
 * Copyright 2023 Simon Vetter
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
 * @file about_window.h
 * 
 * This file declares the class controlling the About window.
 */

#ifndef ABOUT_WINDOW_H
#define ABOUT_WINDOW_H

#include "ui_about_window.h"

#include <QDialog>



/**
 * Control class for About window.
 * 
 * This class controls the About window.
 */
class AboutWindow : public QDialog, public Ui_AboutWindow
{
	Q_OBJECT
	
public:
	AboutWindow(QWidget* parent);
};



#endif // ABOUT_WINDOW_H
