#ifndef ALERTPAGE_H
#define ALERTPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QSplitter>
#include <QTextEdit>
#include <QDateTime>
#include <QVector>

// ================================================================
//  Alert Data Structure
// ================================================================
struct AlertItem {
    QString Time;
    QString Severity;      // CRITICAL / WARNING / INFO
    QString Message;
    QString Source;
    QString Destination;
    QString Port;
    QString Reason;
    QString Action;

    QDateTime Timestamp;
};

class AlertPage : public QWidget {
    Q_OBJECT

public:
    explicit AlertPage(QWidget *parent = nullptr);
    ~AlertPage();

private slots:
    void UpdateAlerts();
    void OnFilterChanged(int index);
    void OnAlertSelected();
    void OnClearAll();
    void OnSearchChanged(const QString &text);

private:
    // ---- UI ----
    QLineEdit   *SearchInput;
    QPushButton *FilterAll;
    QPushButton *FilterCritical;
    QPushButton *FilterWarning;
    QPushButton *FilterInfo;
    QPushButton *ClearButton;

    QListWidget *AlertList;

    // ---- Detail Panel ----
    QLabel      *DetailHeader;
    QLabel      *DetailTime;
    QLabel      *DetailSeverity;
    QLabel      *DetailSource;
    QLabel      *DetailDest;
    QLabel      *DetailPort;
    QLabel      *DetailReason;
    QLabel      *DetailAction;
    QLabel      *DetailPlaceholder;

    // ---- Timer ----
    QTimer *UpdateTimer;

    // ---- Data ----
    QVector<AlertItem> Alerts;
    QString CurrentFilter;

    // ---- Helpers ----
    AlertItem ParseAlert(const QString &raw);
    QString GetSeverityColor(const QString &severity);
    void UpdateDetailPanel(int index);
    void RefreshList();
    bool MatchesFilter(const AlertItem &item);
};

#endif