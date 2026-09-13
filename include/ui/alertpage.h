#ifndef ALERTPAGE_H
#define ALERTPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QTimer>

class AlertPage : public QWidget {
    Q_OBJECT
public:
    explicit AlertPage(QWidget *parent = nullptr);
    ~AlertPage();

private slots:
    void UpdateAlerts();

private:
    QListWidget *AlertList;
    QTimer *UpdateTimer;
    static const int MaxAlerts = 500;
};

#endif