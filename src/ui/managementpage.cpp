#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QDebug>

#include "managementpage.h"
#include "receiver/prefilter.h"
#include "receiver/packet_counter.h"

// ================================================================
//  Palette (VSCode Dark)
// ================================================================
#define COLOR_BG      "#1a1a1a"
#define COLOR_PANEL   "#212121"
#define COLOR_BORDER  "#2d2d2d"
#define COLOR_TEXT    "#e0e0e0"
#define COLOR_DIM     "#707070"
#define COLOR_ACCENT  "#4ec9b0"
#define COLOR_WARNING "#dcdcaa"
#define COLOR_ERROR   "#f48771"
#define COLOR_INFO    "#6cb6ff"

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
        QString("background-color: %1;"
                "border: 1px solid %2;"
                "border-radius: 4px;").arg(COLOR_PANEL, COLOR_BORDER)
    );

    QVBoxLayout *Layout = new QVBoxLayout(Card);
    Layout->setContentsMargins(14, 10, 14, 10);
    Layout->setSpacing(4);

    QLabel *Title = new QLabel(title, Card);
    Title->setStyleSheet(
        QString("color: %1;"
                "font-size: 10px;"
                "font-weight: 600;"
                "letter-spacing: 1px;"
                "background: transparent;"
                "border: none;").arg(COLOR_DIM)
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
//  Helper: สร้าง Section Title + Line
// ================================================================
static QWidget* MakeSectionHeader(const QString &title, QWidget *parent) {
    QWidget *W = new QWidget(parent);
    QVBoxLayout *L = new QVBoxLayout(W);
    L->setContentsMargins(0, 0, 0, 0);
    L->setSpacing(4);

    QLabel *Title = new QLabel(title, W);
    Title->setStyleSheet(
        QString("color: %1; font-size: 12px; font-weight: 600;"
                "background: transparent;").arg(COLOR_TEXT)
    );
    L->addWidget(Title);

    QFrame *Line = new QFrame(W);
    Line->setFixedHeight(1);
    Line->setStyleSheet(QString("background-color: %1;").arg(COLOR_BORDER));
    L->addWidget(Line);

    return W;
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
        QString("color: %1; font-size: 16px; font-weight: 600;"
                "background: transparent;").arg(COLOR_TEXT)
    );
    MainLayout->addWidget(Title);

    QFrame *Line0 = new QFrame(this);
    Line0->setFixedHeight(1);
    Line0->setStyleSheet(QString("background-color: %1;").arg(COLOR_BORDER));
    MainLayout->addWidget(Line0);

    // ============================================================
    //  Statistics
    // ============================================================
    MainLayout->addWidget(MakeSectionHeader("Statistics", this));

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
    //  Rate Threshold
    // ============================================================
    MainLayout->addWidget(MakeSectionHeader("Rate Threshold", this));

    QHBoxLayout *ThresholdLayout = new QHBoxLayout();
    ThresholdLayout->setSpacing(10);

    QLabel *ThresholdDesc = new QLabel("Packets per second limit:", this);
    ThresholdDesc->setStyleSheet(
        QString("color: %1; font-size: 11px; background: transparent;")
            .arg(COLOR_DIM)
    );
    ThresholdLayout->addWidget(ThresholdDesc);

    RateThresholdInput = new QSpinBox(this);
    RateThresholdInput->setRange(10, 10000);
    RateThresholdInput->setValue(PreFilterGetRateThreshold());
    RateThresholdInput->setSuffix(" pps");
    RateThresholdInput->setFixedWidth(120);
    RateThresholdInput->setStyleSheet(
        QString("QSpinBox {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-family: Consolas;"
                "  font-size: 11px;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 5px 8px;"
                "}"
                "QSpinBox:focus { border: 1px solid %4; }")
            .arg(COLOR_PANEL, COLOR_TEXT, COLOR_BORDER, COLOR_ACCENT)
    );
    ThresholdLayout->addWidget(RateThresholdInput);

    ApplyThresholdButton = new QPushButton("Apply", this);
    ApplyThresholdButton->setCursor(Qt::PointingHandCursor);
    ApplyThresholdButton->setFixedHeight(28);
    ApplyThresholdButton->setStyleSheet(
        QString("QPushButton {"
                "  background-color: #2a3a35;"
                "  color: %1;"
                "  font-size: 11px;"
                "  font-weight: 600;"
                "  border: 1px solid %1;"
                "  border-radius: 4px;"
                "  padding: 0 16px;"
                "}"
                "QPushButton:hover { background-color: #3a4a45; }")
            .arg(COLOR_ACCENT)
    );
    ThresholdLayout->addWidget(ApplyThresholdButton);

    ThresholdLayout->addStretch();
    MainLayout->addLayout(ThresholdLayout);

    // ============================================================
    //  Blacklist
    // ============================================================
    MainLayout->addWidget(MakeSectionHeader("Blacklist IPs", this));

    QHBoxLayout *AddIPLayout = new QHBoxLayout();
    AddIPLayout->setSpacing(8);

    IPInput = new QLineEdit(this);
    IPInput->setPlaceholderText("e.g. 192.168.1.100");
    IPInput->setStyleSheet(
        QString("QLineEdit {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-family: Consolas;"
                "  font-size: 11px;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 6px 10px;"
                "}"
                "QLineEdit:focus { border: 1px solid %4; }")
            .arg(COLOR_PANEL, COLOR_TEXT, COLOR_BORDER, COLOR_ACCENT)
    );
    AddIPLayout->addWidget(IPInput, 1);

    AddIPButton = new QPushButton("+ Add IP", this);
    AddIPButton->setCursor(Qt::PointingHandCursor);
    AddIPButton->setFixedHeight(30);
    AddIPButton->setFixedWidth(110);
    AddIPButton->setStyleSheet(
        QString("QPushButton {"
                "  background-color: #2a3a35;"
                "  color: %1;"
                "  font-size: 11px;"
                "  font-weight: 600;"
                "  border: 1px solid %1;"
                "  border-radius: 4px;"
                "}"
                "QPushButton:hover { background-color: #3a4a45; }")
            .arg(COLOR_ACCENT)
    );
    AddIPLayout->addWidget(AddIPButton);

    MainLayout->addLayout(AddIPLayout);

    BlacklistList = new QListWidget(this);
    BlacklistList->setFixedHeight(110);
    BlacklistList->setContextMenuPolicy(Qt::CustomContextMenu);
    BlacklistList->setStyleSheet(
        QString("QListWidget {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-family: Consolas;"
                "  font-size: 11px;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 4px;"
                "  outline: none;"
                "}"
                "QListWidget::item {"
                "  padding: 5px 8px;"
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

    MainLayout->addWidget(BlacklistList);

    // ============================================================
    //  Suspicious Ports
    // ============================================================
    MainLayout->addWidget(MakeSectionHeader("Suspicious Ports", this));

    QHBoxLayout *AddPortLayout = new QHBoxLayout();
    AddPortLayout->setSpacing(8);

    PortInput = new QLineEdit(this);
    PortInput->setPlaceholderText("e.g. 4444");
    PortInput->setStyleSheet(
        QString("QLineEdit {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-family: Consolas;"
                "  font-size: 11px;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 6px 10px;"
                "}"
                "QLineEdit:focus { border: 1px solid %4; }")
            .arg(COLOR_PANEL, COLOR_TEXT, COLOR_BORDER, COLOR_ACCENT)
    );
    AddPortLayout->addWidget(PortInput, 1);

    QPushButton *AddPortBtn = new QPushButton("+ Add Port", this);
    AddPortBtn->setCursor(Qt::PointingHandCursor);
    AddPortBtn->setFixedHeight(30);
    AddPortBtn->setFixedWidth(110);
    AddPortBtn->setStyleSheet(
        QString("QPushButton {"
                "  background-color: #2a3a35;"
                "  color: %1;"
                "  font-size: 11px;"
                "  font-weight: 600;"
                "  border: 1px solid %1;"
                "  border-radius: 4px;"
                "}"
                "QPushButton:hover { background-color: #3a4a45; }")
            .arg(COLOR_ACCENT)
    );
    AddPortLayout->addWidget(AddPortBtn);

    connect(AddPortBtn, &QPushButton::clicked, this, &ManagementPage::OnAddPortClicked);

    MainLayout->addLayout(AddPortLayout);

    // ---- Ports container (chips) ----
    PortsContainer = new QWidget(this);
    PortsContainer->setStyleSheet(
        QString("background-color: %1;"
                "border: 1px solid %2;"
                "border-radius: 4px;").arg(COLOR_BG, COLOR_BORDER)
    );
    PortsLayout = new QHBoxLayout(PortsContainer);
    PortsLayout->setContentsMargins(10, 8, 10, 8);
    PortsLayout->setSpacing(8);

    PortsContainer->setFixedHeight(50);
    MainLayout->addWidget(PortsContainer);

    // ============================================================
    //  Clear All Button
    // ============================================================
    ClearButton = new QPushButton("🗑   Clear All Rules", this);
    ClearButton->setCursor(Qt::PointingHandCursor);
    ClearButton->setFixedHeight(36);
    ClearButton->setStyleSheet(
        QString("QPushButton {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-size: 12px;"
                "  font-weight: 600;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
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
    connect(AddIPButton,          &QPushButton::clicked, this, &ManagementPage::OnAddBlacklistClicked);
    connect(ClearButton,          &QPushButton::clicked, this, &ManagementPage::OnClearRulesClicked);
    connect(ApplyThresholdButton, &QPushButton::clicked, this, &ManagementPage::OnApplyThresholdClicked);

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
//  Add Blacklist
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
//  Apply Threshold
// ================================================================
void ManagementPage::OnApplyThresholdClicked() {
    int val = RateThresholdInput->value();
    PreFilterSetRateThreshold(val);

    QMessageBox::information(this, "Applied",
        QString("Rate threshold set to %1 pps").arg(val));
}

// ================================================================
//  Add Port
// ================================================================
void ManagementPage::OnAddPortClicked() {
    QString text = PortInput->text().trimmed();
    if (text.isEmpty()) return;

    bool ok = false;
    int port = text.toInt(&ok);
    if (!ok || port < 1 || port > 65535) {
        QMessageBox::warning(this, "Invalid Port",
            "กรุณากรอก Port 1-65535");
        return;
    }

    // ⚠ ตัวอย่าง: ยังไม่ save จริง — ต้องมี API เพิ่ม
    // PreFilterAddSuspiciousPort((unsigned short)port);

    PortInput->clear();
    RefreshUI();
}

// ================================================================
//  Remove Port (chip)
// ================================================================
void ManagementPage::OnRemovePortClicked() {
    // ใช้ lambda ใน RebuildPortChips แทน
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
//  Refresh: Blacklist
// ================================================================
void ManagementPage::RefreshBlacklist() {
    BlacklistList->clear();
    int Count = PreFilterGetBlacklistCount();
    for (int i = 0; i < Count; i++) {
        unsigned int ip = PreFilterGetBlacklistIP(i);
        QListWidgetItem *Item = new QListWidgetItem("  " + IPToString(ip));
        Item->setData(Qt::UserRole, ip);
        BlacklistList->addItem(Item);
    }
}

// ================================================================
//  Refresh: Ports (chips)
// ================================================================
void ManagementPage::RefreshPorts() {
    // ---- ลบ chip เก่า ----
    QLayoutItem *item;
    while ((item = PortsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    // ---- สร้าง chip ใหม่ ----
    int portCount = PreFilterGetSuspiciousPortCount();
    for (int i = 0; i < portCount; i++) {
        unsigned short port = PreFilterGetSuspiciousPort(i);

        QWidget *chip = new QWidget(PortsContainer);
        chip->setStyleSheet(
            QString("background-color: %1;"
                    "border: 1px solid %2;"
                    "border-radius: 12px;").arg(COLOR_PANEL, COLOR_BORDER)
        );
        QHBoxLayout *chipLayout = new QHBoxLayout(chip);
        chipLayout->setContentsMargins(10, 2, 6, 2);
        chipLayout->setSpacing(4);

        QLabel *portLabel = new QLabel(QString::number(port), chip);
        portLabel->setStyleSheet(
            QString("color: %1; font-size: 11px; font-family: Consolas;"
                    "background: transparent; border: none;").arg(COLOR_WARNING)
        );
        chipLayout->addWidget(portLabel);

        QPushButton *removeBtn = new QPushButton("×", chip);
        removeBtn->setFixedSize(16, 16);
        removeBtn->setCursor(Qt::PointingHandCursor);
        removeBtn->setStyleSheet(
            QString("QPushButton {"
                    "  color: %1; background: transparent;"
                    "  border: none; font-size: 14px;"
                    "}"
                    "QPushButton:hover { color: %2; }")
                .arg(COLOR_DIM, COLOR_ERROR)
        );
        chipLayout->addWidget(removeBtn);

        unsigned short portVal = port;
        connect(removeBtn, &QPushButton::clicked, this, [this, portVal]() {
            // ⚠ ต้องมี API ลบ port จริง
            // PreFilterRemoveSuspiciousPort(portVal);
            QMessageBox::information(this, "Info",
                QString("Remove port %1 — ต้องเพิ่ม API ก่อน").arg(portVal));
        });

        PortsLayout->addWidget(chip);
    }

    PortsLayout->addStretch();
}

// ================================================================
//  Refresh UI (หลัก)
// ================================================================
void ManagementPage::RefreshUI() {
    // ---- Stats ----
    int TotalPacket   = GetPacketCount() + GetBlockedPacketCount();
    int BlockedPacket = GetBlockedPacketCount();
    int AllowedPacket = GetPacketCount();

    TotalPacketsLabel->setText(QString::number(TotalPacket));
    AllowedPacketsLabel->setText(QString::number(AllowedPacket));
    BlockedPacketsLabel->setText(QString::number(BlockedPacket));

    // ---- Blacklist ----
    RefreshBlacklist();

    // ---- Ports ----
    RefreshPorts();
}