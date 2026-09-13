#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QMenu>
#include <QAction>

#include "managementpage.h"
#include "receiver/prefilter.h"
#include "receiver/packet_counter.h"

static QString IPToString(unsigned int IP) {
    return QString("%1.%2.%3.%4")
        .arg(IP & 0xFF)
        .arg((IP >> 8) & 0xFF)
        .arg((IP >> 16) & 0xFF)
        .arg((IP >> 24) & 0xFF);
}

static unsigned int StringToIP(const QString &String) {
    QStringList parts = String.split('.');
    if (parts.size() != 4) return 0;
    return (parts[0].toUInt() & 0xFF) |
           ((parts[1].toUInt() & 0xFF) << 8) |
           ((parts[2].toUInt() & 0xFF) << 16) |
           ((parts[3].toUInt() & 0xFF) << 24);
}

ManagementPage::ManagementPage(QWidget *parent) : QWidget(parent) {
    this->setStyleSheet("background-color: #1a2127;");

    QVBoxLayout *MainLayout = new QVBoxLayout(this);
    MainLayout->setContentsMargins(20, 20, 20, 20);
    MainLayout->setSpacing(15);

    QLabel *Title = new QLabel("⚙ Management", this);
    Title->setStyleSheet("color: white; font-size: 22px; font-weight: bold;");
    MainLayout->addWidget(Title);

    QGroupBox *StatsGroup = new QGroupBox("📊 Statistics", this);
    StatsGroup->setStyleSheet(
        "QGroupBox { color: #00ffcc; font-size: 14px; font-weight: bold; "
        "border: 1px solid #333; border-radius: 5px; margin-top: 10px; padding-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
    );
    QHBoxLayout *StatsLayout = new QHBoxLayout(StatsGroup);

    // Total
    QWidget *TotalBox = new QWidget(this);
    TotalBox->setStyleSheet("background-color: #2a2a2a; border-radius: 5px;");
    QVBoxLayout *TotalVBox = new QVBoxLayout(TotalBox);
    QLabel *TotalTitle = new QLabel("TOTAL PACKETS", TotalBox);
    TotalTitle->setStyleSheet("color: #888; font-size: 11px;");
    TotalPacketsLabel = new QLabel("0", TotalBox);
    TotalPacketsLabel->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");
    TotalVBox->addWidget(TotalTitle);
    TotalVBox->addWidget(TotalPacketsLabel);
    StatsLayout->addWidget(TotalBox, 1);

    // Allowed
    QWidget *AllowedBox = new QWidget(this);
    AllowedBox->setStyleSheet("background-color: #2a2a2a; border-radius: 5px;");
    QVBoxLayout *AllowedVBox = new QVBoxLayout(AllowedBox);
    QLabel *AllowedTitle = new QLabel("ALLOWED", AllowedBox);
    AllowedTitle->setStyleSheet("color: #888; font-size: 11px;");
    AllowedPacketsLabel = new QLabel("0", AllowedBox);
    AllowedPacketsLabel->setStyleSheet("color: #00ffcc; font-size: 20px; font-weight: bold;");
    AllowedVBox->addWidget(AllowedTitle);
    AllowedVBox->addWidget(AllowedPacketsLabel);
    StatsLayout->addWidget(AllowedBox, 1);

    // Blocked
    QWidget *BlockedBox = new QWidget(this);
    BlockedBox->setStyleSheet("background-color: #2a2a2a; border-radius: 5px;");
    QVBoxLayout *BlockedVBox = new QVBoxLayout(BlockedBox);
    QLabel *BlockedTitle = new QLabel("BLOCKED", BlockedBox);
    BlockedTitle->setStyleSheet("color: #888; font-size: 11px;");
    BlockedPacketsLabel = new QLabel("0", BlockedBox);
    BlockedPacketsLabel->setStyleSheet("color: #ff5555; font-size: 20px; font-weight: bold;");
    BlockedVBox->addWidget(BlockedTitle);
    BlockedVBox->addWidget(BlockedPacketsLabel);
    StatsLayout->addWidget(BlockedBox, 1);

    MainLayout->addWidget(StatsGroup);

    // ---- Current Rules Group ----
    QGroupBox *RulesGroup = new QGroupBox("Current Rules", this);
    RulesGroup->setStyleSheet(
        "QGroupBox { color: #00ffcc; font-size: 14px; font-weight: bold; "
        "border: 1px solid #333; border-radius: 5px; margin-top: 10px; padding-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
    );
    QVBoxLayout *RulesLayout = new QVBoxLayout(RulesGroup);

    BlacklistCountLabel = new QLabel(this);
    BlacklistCountLabel->setStyleSheet("color: #ff8888; font-size: 14px;");
    RulesLayout->addWidget(BlacklistCountLabel);

    SuspiciousPortsLabel = new QLabel(this);
    SuspiciousPortsLabel->setStyleSheet("color: #ffaa00; font-size: 14px;");
    RulesLayout->addWidget(SuspiciousPortsLabel);

    RateThresholdLabel = new QLabel(this);
    RateThresholdLabel->setStyleSheet("color: #00ffcc; font-size: 14px;");
    RulesLayout->addWidget(RateThresholdLabel);

    MainLayout->addWidget(RulesGroup);

    // ---- Add Blacklist ----
    QGroupBox *AddGroup = new QGroupBox("Add IP to Blacklist", this);
    AddGroup->setStyleSheet(
        "QGroupBox { color: #00ffcc; font-size: 14px; font-weight: bold; "
        "border: 1px solid #333; border-radius: 5px; margin-top: 10px; padding-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
    );
    QHBoxLayout *AddLayout = new QHBoxLayout(AddGroup);

    IPInput = new QLineEdit(this);
    IPInput->setPlaceholderText("192.168.1.100");
    IPInput->setStyleSheet(
        "QLineEdit { background-color: #2a2a2a; color: white; "
        "border: 1px solid #444; padding: 6px; border-radius: 3px; }"
    );
    AddLayout->addWidget(IPInput);

    AddButton = new QPushButton("Add", this);
    AddButton->setStyleSheet(
        "QPushButton { background-color: #2c3e50; color: white; "
        "padding: 6px 20px; border: none; border-radius: 3px; font-weight: bold; }"
        "QPushButton:hover { background-color: #34495e; }"
    );
    AddLayout->addWidget(AddButton);

    MainLayout->addWidget(AddGroup);

    // ---- Blacklist List ----
    QGroupBox *ListGroup = new QGroupBox("Blacklist", this);
    ListGroup->setStyleSheet(
        "QGroupBox { color: #ff5555; font-size: 14px; font-weight: bold; "
        "border: 1px solid #333; border-radius: 5px; margin-top: 10px; padding-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
    );
    QVBoxLayout *ListLayout = new QVBoxLayout(ListGroup);

    BlacklistList = new QListWidget(this);
    BlacklistList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(BlacklistList, &QListWidget::customContextMenuRequested,
            this, [this](const QPoint &pos) {
        QListWidgetItem *Item = BlacklistList->itemAt(pos);
        if (!Item){
            return;
        }

        unsigned int IP = Item->data(Qt::UserRole).toUInt();

        RefreshTimer->stop();  

        QMenu menu(this);
        QAction *RemoveAction = menu.addAction("🗑 Remove");
        QAction *Selected = menu.exec(BlacklistList->mapToGlobal(pos));

        RefreshTimer->start(1000);

        if (Selected == RemoveAction) {
            PreFilterRemoveBlacklist(IP);
            RefreshUI();
        }
    });
    BlacklistList->setStyleSheet(
        "QListWidget { background-color: #1a1a1a; color: #ff8888; "
        "font-family: Consolas; font-size: 12px; border: 1px solid #333; }"
        "QListWidget::item { padding: 5px; }"
    );
    ListLayout->addWidget(BlacklistList);

    MainLayout->addWidget(ListGroup, 1);

    // ---- Clear Button ----
    ClearButton = new QPushButton("🗑 Clear All Rules", this);
    ClearButton->setStyleSheet(
        "QPushButton { background-color: #c0392b; color: white; "
        "padding: 10px; border: none; border-radius: 3px; font-weight: bold; }"
        "QPushButton:hover { background-color: #e74c3c; }"
    );
    MainLayout->addWidget(ClearButton);

    // ---- Connect ----
    connect(AddButton, &QPushButton::clicked, this, &ManagementPage::OnAddBlacklistClicked);
    connect(ClearButton, &QPushButton::clicked, this, &ManagementPage::OnClearRulesClicked);

    // ---- Timer Refresh (ทุก 1 วินาที) ----
    RefreshTimer = new QTimer(this);
    connect(RefreshTimer, &QTimer::timeout, this, &ManagementPage::RefreshUI);
    RefreshTimer->start(1000);

    RefreshUI();
}

ManagementPage::~ManagementPage() {}

void ManagementPage::OnAddBlacklistClicked() {
    QString IPInString = IPInput->text().trimmed();
    if (IPInString.isEmpty()) return;

    // ตรวจสอบรูปแบบ IP ง่าย ๆ
    QStringList Parts = IPInString.split('.');
    if (Parts.size() != 4) {
        QMessageBox::warning(this, "Invalid IP", "กรุณากรอก IP ให้ถูกต้อง เช่น 192.168.1.100");
        return;
    }

    unsigned int IP = StringToIP(IPInString);
    PreFilterAddBlacklist(IP);
    IPInput->clear();

    QMessageBox::information(this, "Success", QString("IP %1 added to Blacklist\nและถูกบล็อกผ่าน Windows Firewall แล้ว").arg(IPInString));

    RefreshUI();
}

void ManagementPage::OnClearRulesClicked() {
    auto reply = QMessageBox::question(this, "Confirm",
        "Clear all rules?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        PreFilterClear();
        RefreshUI();
    }
}

void ManagementPage::RefreshUI() {

    int TotalPacket = GetPacketCount() + GetBlockedPacketCount();
    int BlockedPacket = GetBlockedPacketCount();
    int AllowedPacket = GetPacketCount();

    TotalPacketsLabel->setText(QString::number(TotalPacket));
    BlockedPacketsLabel->setText(QString("%1 IPs").arg(GetBlockedIPCount()));
    AllowedPacketsLabel->setText(QString::number(AllowedPacket));

    // ---- Count labels ----
    BlacklistCountLabel->setText(
        QString("Blacklist: %1 IPs").arg(PreFilterGetBlacklistCount()));

    SuspiciousPortsLabel->setText(
        QString("Suspicious Ports: %1 ports").arg(PreFilterGetSuspiciousPortCount()));

    RateThresholdLabel->setText(
        QString("Rate Threshold: %1 pps").arg(PreFilterGetRateThreshold()));

    // ---- List ----
    BlacklistList->clear();
    int Count = PreFilterGetBlacklistCount();
    for (int Index = 0; Index < Count; Index++) {
        unsigned int ip = PreFilterGetBlacklistIP(Index);
        QListWidgetItem *Item = new QListWidgetItem(IPToString(ip));
        Item->setData(Qt::UserRole, ip);
        BlacklistList->addItem(Item);
    }
}