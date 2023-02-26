#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

#include "src/main/settings.h"
#include "ui_settings_window.h"



class SettingsWindow : public QDialog, public Ui_SettingsWindow, public Settings
{
	Q_OBJECT
	
public:
	SettingsWindow(QWidget* parent);
	
private:
	void loadSettings();
	void saveSettings();
	void loadDefaults();
	void updateEnabled();
	
private slots:
	void handle_ascentDateCheckboxChanged();
	void handle_ascentTimeCheckboxChanged();
	void handle_ascentElevationGainCheckboxChanged();
	void handle_peakHeightCheckboxChanged();
	
	void handle_save();
	void handle_apply();
	void handle_cancel();
	void handle_loadDefaults();
	
private:
	void reject() override;
};



#endif // SETTINGS_WINDOW_H
