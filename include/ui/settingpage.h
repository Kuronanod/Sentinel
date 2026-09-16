#ifndef SETTINGPAGE_H
#define SETTINGPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>

class SettingPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingPage(QWidget *parent = nullptr);
    ~SettingPage();

private slots:
    void OnBrowseLogPath();
    void OnSaveSettings();
    void OnResetSettings();

private:
    QWidget* MakeSection(const QString &title);

    // ---- Network ----
    QComboBox *InterfaceCombo;
    QSpinBox  *RefreshRateInput;

    // ---- Alert Preferences ----
    QCheckBox *SoundCheck;
    QCheckBox *NotifyCheck;
    QSpinBox  *MaxAlertsInput;

    // ---- Logging ----
    QCheckBox *LogToFileCheck;
    QLineEdit *LogPathInput;
    QPushButton *BrowseBtn;
};

#endif