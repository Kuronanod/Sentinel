#ifndef MANAGEMENTPAGE_H
#define MANAGEMENTPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QTimer>
#include <QSpinBox>
#include <QHBoxLayout>

class ManagementPage : public QWidget {
    Q_OBJECT

public:
    explicit ManagementPage(QWidget *parent = nullptr);
    ~ManagementPage();

private slots:
    void OnAddBlacklistClicked();
    void OnClearRulesClicked();
    void OnApplyThresholdClicked();
    void OnAddPortClicked();
    void RefreshUI();

private:
    // ============ Stats ============
    QLabel *TotalPacketsLabel;
    QLabel *AllowedPacketsLabel;
    QLabel *BlockedPacketsLabel;

    // ============ Blacklist ============
    QLineEdit   *IPInput;
    QPushButton *AddIPButton;
    QListWidget *BlacklistList;

    // ============ Suspicious Ports ============
    QLineEdit *PortInput;
    QWidget   *PortsContainer;
    QHBoxLayout *PortsLayout;

    // ============ Rate Threshold ============
    QSpinBox    *RateThresholdInput;
    QPushButton *ApplyThresholdButton;

    // ============ Clear ============
    QPushButton *ClearButton;

    // ============ Timer ============
    QTimer *RefreshTimer;

    // ============ Helpers ============
    void RefreshBlacklist();
    void RefreshPorts();
    void RebuildPortChips();
};

#endif