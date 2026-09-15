#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QFrame>

#include "managementpage.h"
#include "receiver/prefilter.h"
#include "receiver/packet_counter.h"

// ================================================================
//  Palette (VSCode Dark) — เหมือน trafficgraph.cpp
// ================================================================
#define COLOR_BG      "#1a1a1a"
#define COLOR_PANEL   "#212121"
#define COLOR_BORDER  "#2d2d2d"
#define COLOR_TEXT    "#e0e0e0"
#define COLOR_DIM     "#707070"
#define COLOR_ACCENT  "#4ec9b0"
#define COLOR_SUCCESS "#4ec9b0"
#define COLOR_WARNING "#dcdcaa"
#define COLOR_ERROR   "#f48771"

// ================================================================
//  Helper: IP <-> String
// ================================================================
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

// ================================================================
//  Helper: สร้าง Stat Card
// ================================================================
static QWidget* MakeStatCard(const QString &title, const QString &color,
                             QLabel **valueLabel, QWidget *parent)
{
    QWidget *Card = new QWidget(parent);
    Card->setStyleSheet(
        "background-color: " COLOR_PANEL ";"
        "border: 1px solid " COLOR_BORDER ";"
        "border-radius: 4px;"
    );

    QVBoxLayout *Layout = new QVBoxLayout(Card);
    Layout->setContentsMargins(14, 10, 14, 10);
    Layout->setSpacing(4);

    QLabel *Title = new QLabel(title, Card);
    Title->setStyleSheet(
        "color: " COLOR_DIM ";"
        "font-size: 10px;"
        "font-weight: 600;"
        "letter-spacing: 1px;"
        "background: transparent;"
        "border: none;"
    );
    Layout->addWidget(Title);

    *valueLabel = new QLabel("0", Card);
    (*valueLabel)->setStyleSheet(
        QString("color: %1; font-size: 22px; font-weight: bold;"
                "background: transparent; border: none;").arg(color)
    );
    Layout->addWidget(*valueLabel);

    return Card;
}

// ================================================================
//  Constructor
// ================================================================
ManagementPage::ManagementPage(QWidget *parent) : QWidget(parent) {

    this->setStyleSheet(
        QString("ManagementPage { background-color: %1; }").arg(COLOR_BG)
    );

    QVBoxLayout *MainLayout = new QVBoxLayout(this);
    MainLayout->setContentsMargins(20, 20, 20, 20);
    MainLayout->setSpacing(14);

    // ============================================================
    //  Title
    // ============================================================
    QLabel *Title = new QLabel("Management", this);
    Title->setStyleSheet(
        "color: " COLOR_TEXT ";"
        "font-size: 16px;"
        "font-weight: 600;"
        "background: transparent;"
    );
    MainLayout->addWidget(Title);

    QFrame *Line0 = new QFrame(this);
    Line0->setFixedHeight(1);
    Line0->setStyleSheet(QString("background-color: %1;").arg(COLOR_BORDER));
    MainLayout->addWidget(Line0);

    // ============================================================
    //  Statistics
    // ============================================================
    QLabel *StatsTitle = new QLabel("Statistics", this);
    StatsTitle->setStyleSheet(
        QString("color: %1; font-size: 12px; font-weight: 600;"
                "background: transparent;").arg(COLOR_TEXT)
    );
    MainLayout->addWidget(StatsTitle);

    QHBoxLayout *StatsLayout = new QHBoxLayout();
    StatsLayout->setSpacing(12);

    QWidget *TotalCard   = MakeStatCard("TOTAL PACKETS", COLOR_TEXT,   &TotalPacketsLabel,   this);
    QWidget *AllowedCard = MakeStatCard("ALLOWED",       COLOR_ACCENT, &AllowedPacketsLabel, this);
    QWidget *BlockedCard = MakeStatCard("BLOCKED",       COLOR_ERROR,  &BlockedPacketsLabel, this);

    StatsLayout->addWidget(TotalCard,   1);
    StatsLayout->addWidget(AllowedCard, 1);
    StatsLayout->addWidget(BlockedCard, 1);

    MainLayout->addLayout(StatsLayout);

    // ============================================================
    //  Current Rules
    // ============================================================
    QLabel *RulesTitle = new QLabel("Current Rules", this);
    RulesTitle->setStyleSheet(
        QString("color: %1; font-size: 12px; font-weight: 600;"
                "background: transparent;").arg(COLOR_TEXT)
    );
    MainLayout->addWidget(RulesTitle);

    QFrame *Line1 = new QFrame(this);
    Line1->setFixedHeight(1);
    Line1->setStyleSheet(QString("background-color: %1;").arg(COLOR_BORDER));
    MainLayout->addWidget(Line1);

    // ---- Rule Labels ----
    BlacklistCountLabel = new QLabel(this);
    BlacklistCountLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; font-family: Consolas;"
                "background: transparent; padding: 3px 0;").arg(COLOR_ERROR)
    );
    MainLayout->addWidget(BlacklistCountLabel);

    SuspiciousPortsLabel = new QLabel(this);
    SuspiciousPortsLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; font-family: Consolas;"
                "background: transparent; padding: 3px 0;").arg(COLOR_WARNING)
    );
    MainLayout->addWidget(SuspiciousPortsLabel);

    RateThresholdLabel = new QLabel(this);
    RateThresholdLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; font-family: Consolas;"
                "background: transparent; padding: 3px 0;").arg(COLOR_ACCENT)
    );
    MainLayout->addWidget(RateThresholdLabel);

    // ============================================================
    //  Add IP
    // ============================================================
    QLabel *AddTitle = new QLabel("Add IP to Blacklist", this);
    AddTitle->setStyleSheet(
        QString("color: %1; font-size: 12px; font-weight: 600;"
                "background: transparent; padding-top: 6px;").arg(COLOR_TEXT)
    );
    MainLayout->addWidget(AddTitle);

    QFrame *Line2 = new QFrame(this);
    Line2->setFixedHeight(1);
    Line2->setStyleSheet(QString("background-color: %1;").arg(COLOR_BORDER));
    MainLayout->addWidget(Line2);

    QHBoxLayout *AddLayout = new QHBoxLayout();
    AddLayout->setSpacing(8);

    IPInput = new QLineEdit(this);
    IPInput->setPlaceholderText("192.168.1.100");
    IPInput->setStyleSheet(
        QString("QLineEdit {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-family: Consolas;"
                "  font-size: 12px;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 7px 10px;"
                "}"
                "QLineEdit:focus {"
                "  border: 1px solid %4;"
                "}").arg(COLOR_PANEL, COLOR_TEXT, COLOR_BORDER, COLOR_ACCENT)
    );
    AddLayout->addWidget(IPInput, 1);

    AddButton = new QPushButton("Add", this);
    AddButton->setFixedWidth(90);
    AddButton->setCursor(Qt::PointingHandCursor);
    AddButton->setStyleSheet(
        QString("QPushButton {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-size: 12px;"
                "  font-weight: 600;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 7px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #2a3a35;"
                "  border: 1px solid %2;"
                "}").arg(COLOR_PANEL, COLOR_ACCENT, COLOR_BORDER)
    );
    AddLayout->addWidget(AddButton);

    MainLayout->addLayout(AddLayout);

    // ============================================================
    //  Blacklist
    // ============================================================
    QLabel *ListTitle = new QLabel("Blacklist", this);
    ListTitle->setStyleSheet(
        QString("color: %1; font-size: 12px; font-weight: 600;"
                "background: transparent; padding-top: 6px;").arg(COLOR_TEXT)
    );
    MainLayout->addWidget(ListTitle);

    QFrame *Line3 = new QFrame(this);
    Line3->setFixedHeight(1);
    Line3->setStyleSheet(QString("background-color: %1;").arg(COLOR_BORDER));
    MainLayout->addWidget(Line3);

    BlacklistList = new QListWidget(this);
    BlacklistList->setContextMenuPolicy(Qt::CustomContextMenu);
    BlacklistList->setStyleSheet(
        QString("QListWidget {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-family: Consolas;"
                "  font-size: 12px;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 4px;"
                "  outline: none;"
                "}"
                "QListWidget::item {"
                "  padding: 6px 8px;"
                "  border-radius: 3px;"
                "}"
                "QListWidget::item:hover {"
                "  background-color: %4;"
                "}"
                "QListWidget::item:selected {"
                "  background-color: #2a3a35;"
                "  color: %5;"
                "}").arg(COLOR_BG, COLOR_ERROR, COLOR_BORDER, COLOR_PANEL, COLOR_ACCENT)
    );

    connect(BlacklistList, &QListWidget::customContextMenuRequested,
            this, [this](const QPoint &pos) {
        QListWidgetItem *Item = BlacklistList->itemAt(pos);
        if (!Item) return;

        unsigned int IP = Item->data(Qt::UserRole).toUInt();

        RefreshTimer->stop();

        QMenu menu(this);
        menu.setStyleSheet(
            QString("QMenu {"
                    "  background-color: %1;"
                    "  color: %2;"
                    "  border: 1px solid %3;"
                    "  padding: 4px;"
                    "}"
                    "QMenu::item {"
                    "  padding: 6px 20px;"
                    "  border-radius: 3px;"
                    "}"
                    "QMenu::item:selected {"
                    "  background-color: %4;"
                    "}").arg(COLOR_PANEL, COLOR_TEXT, COLOR_BORDER, COLOR_ERROR)
        );

        QAction *RemoveAction = menu.addAction("🗑  Remove");
        QAction *Selected = menu.exec(BlacklistList->mapToGlobal(pos));

        RefreshTimer->start(1000);

        if (Selected == RemoveAction) {
            PreFilterRemoveBlacklist(IP);
            RefreshUI();
        }
    });

    MainLayout->addWidget(BlacklistList, 1);

    // ============================================================
    //  Clear Button
    // ============================================================
    ClearButton = new QPushButton("Clear All Rules", this);
    ClearButton->setCursor(Qt::PointingHandCursor);
    ClearButton->setStyleSheet(
        QString("QPushButton {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-size: 12px;"
                "  font-weight: 600;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 10px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #3a2020;"
                "  border: 1px solid %2;"
                "}").arg(COLOR_PANEL, COLOR_ERROR, COLOR_BORDER)
    );
    MainLayout->addWidget(ClearButton);

    // ============================================================
    //  Connect
    // ============================================================
    connect(AddButton,   &QPushButton::clicked, this, &ManagementPage::OnAddBlacklistClicked);
    connect(ClearButton, &QPushButton::clicked, this, &ManagementPage::OnClearRulesClicked);

    // ============================================================
    //  Timer
    // ============================================================
    RefreshTimer = new QTimer(this);
    connect(RefreshTimer, &QTimer::timeout, this, &ManagementPage::RefreshUI);
    RefreshTimer->start(1000);

    RefreshUI();
}

ManagementPage::~ManagementPage() {}

// ================================================================
//  Add IP
// ================================================================
void ManagementPage::OnAddBlacklistClicked() {
    QString IPInString = IPInput->text().trimmed();
    if (IPInString.isEmpty()) return;

    QStringList Parts = IPInString.split('.');
    if (Parts.size() != 4) {
        QMessageBox::warning(this, "Invalid IP",
            "กรุณากรอก IP ให้ถูกต้อง เช่น 192.168.1.100");
        return;
    }

    unsigned int IP = StringToIP(IPInString);
    PreFilterAddBlacklist(IP);
    IPInput->clear();

    QMessageBox::information(this, "Success",
        QString("IP %1 added to Blacklist").arg(IPInString));

    RefreshUI();
}

// ================================================================
//  Clear All
// ================================================================
void ManagementPage::OnClearRulesClicked() {
    auto reply = QMessageBox::question(this, "Confirm",
        "Clear all rules?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        PreFilterClear();
        RefreshUI();
    }
}

// ================================================================
//  Refresh UI
// ================================================================
void ManagementPage::RefreshUI() {

    int TotalPacket   = GetPacketCount() + GetBlockedPacketCount();
    int BlockedPacket = GetBlockedPacketCount();
    int AllowedPacket = GetPacketCount();

    TotalPacketsLabel->setText(QString::number(TotalPacket));
    AllowedPacketsLabel->setText(QString::number(AllowedPacket));
    BlockedPacketsLabel->setText(QString::number(BlockedPacket));

    // ---- Rules ----
    BlacklistCountLabel->setText(
        QString("  ● Blacklist:        %1 IPs")
            .arg(PreFilterGetBlacklistCount()));

    SuspiciousPortsLabel->setText(
        QString("  ● Suspicious Ports: %1 ports")
            .arg(PreFilterGetSuspiciousPortCount()));

    RateThresholdLabel->setText(
        QString("  ● Rate Threshold:   %1 pps")
            .arg(PreFilterGetRateThreshold()));

    // ---- List ----
    BlacklistList->clear();
    int Count = PreFilterGetBlacklistCount();
    for (int Index = 0; Index < Count; Index++) {
        unsigned int ip = PreFilterGetBlacklistIP(Index);
        QListWidgetItem *Item = new QListWidgetItem("  " + IPToString(ip));
        Item->setData(Qt::UserRole, ip);
        BlacklistList->addItem(Item);
    }
}