#ifndef MANAGEMENTPAGE_H
#define MANAGEMENTPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QTimer>

class ManagementPage : public QWidget {
    Q_OBJECT

public:
    explicit ManagementPage(QWidget *parent = nullptr);
    ~ManagementPage();

private slots:
    void OnAddBlacklistClicked();
    void OnClearRulesClicked();
    void RefreshUI();

private:

    QLabel *TotalPacketsLabel;
    QLabel *BlockedPacketsLabel;
    QLabel *AllowedPacketsLabel;

    QLabel *BlacklistCountLabel;
    QLabel *SuspiciousPortsLabel;
    QLabel *RateThresholdLabel;

    QLineEdit *IPInput;
    QPushButton *AddButton;
    QPushButton *ClearButton;

    QListWidget *BlacklistList;
    QTimer *RefreshTimer;
};

#endif