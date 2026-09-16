#ifndef LOGPAGE_H
#define LOGPAGE_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QCheckBox>

class LogPage : public QWidget {
    Q_OBJECT

public:
    explicit LogPage(QWidget *parent = nullptr);
    ~LogPage();

private slots:
    void UpdateLogs();
    void OnSearchChanged(const QString &text);
    void OnClearClicked();
    void OnExportClicked();
    void OnPauseToggled(bool paused);

private:
    QTextEdit   *LogView;
    QLineEdit   *SearchInput;
    QPushButton *ClearBtn;
    QPushButton *ExportBtn;
    QPushButton *PauseBtn;
    QLabel      *StatusLabel;

    QTimer      *UpdateTimer;
    bool         Paused = false;
    int          TotalLines = 0;

    // ---- Levels to show ----
    bool ShowDebug = false;
    bool ShowInfo  = true;
    bool ShowWarn  = true;
    bool ShowCrit  = true;

    QString BuildHtml(int level, const QString &time, const QString &msg);
};

#endif