#include "packetpage.h"
#include "receiver/request_queue.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFrame>
#include <QDebug>
#include <QTime>

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
//  Helper: แปลง IP
// ================================================================
static QString IPToString(unsigned int ip) {
    return QString("%1.%2.%3.%4")
        .arg(ip & 0xFF)
        .arg((ip >> 8) & 0xFF)
        .arg((ip >> 16) & 0xFF)
        .arg((ip >> 24) & 0xFF);
}

// ================================================================
//  Constructor
// ================================================================
PacketPage::PacketPage(QWidget *parent) : QWidget(parent) {

    this->setStyleSheet(
        QString("PacketPage { background-color: %1; }").arg(COLOR_BG)
    );

    QVBoxLayout *MainLayout = new QVBoxLayout(this);
    MainLayout->setContentsMargins(0, 0, 0, 0);
    MainLayout->setSpacing(0);

    // ============================================================
    //  Title Bar
    // ============================================================
    QWidget *TitleBar = new QWidget(this);
    TitleBar->setFixedHeight(40);
    TitleBar->setStyleSheet(
        QString("background-color: %1;"
                "border-bottom: 1px solid %2;").arg(COLOR_PANEL, COLOR_BORDER)
    );
    QHBoxLayout *TitleLayout = new QHBoxLayout(TitleBar);
    TitleLayout->setContentsMargins(16, 0, 16, 0);
    TitleLayout->setSpacing(12);

    QLabel *TitleLabel = new QLabel("📦  Packet Capture", TitleBar);
    TitleLabel->setStyleSheet(
        QString("color: %1; font-size: 14px; font-weight: 600;"
                "background: transparent; border: none;").arg(COLOR_TEXT)
    );
    TitleLayout->addWidget(TitleLabel);

    TitleLayout->addStretch();

    StatsLabel = new QLabel("0 packets", TitleBar);
    StatsLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; font-family: Consolas;"
                "background: transparent; border: none;").arg(COLOR_DIM)
    );
    TitleLayout->addWidget(StatsLabel);

    MainLayout->addWidget(TitleBar);

    // ============================================================
    //  Search bar
    // ============================================================
    QWidget *SearchBar = new QWidget(this);
    SearchBar->setFixedHeight(42);
    SearchBar->setStyleSheet(
        QString("background-color: %1;"
                "border-bottom: 1px solid %2;").arg(COLOR_BG, COLOR_BORDER)
    );
    QHBoxLayout *SearchLayout = new QHBoxLayout(SearchBar);
    SearchLayout->setContentsMargins(10, 6, 10, 6);

    SearchInput = new QLineEdit(SearchBar);
    SearchInput->setPlaceholderText("🔍  Filter by IP / Port / Protocol...");
    SearchInput->setStyleSheet(
        QString("QLineEdit {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-size: 11px;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 5px 10px;"
                "}"
                "QLineEdit:focus {"
                "  border: 1px solid %4;"
                "}").arg(COLOR_PANEL, COLOR_TEXT, COLOR_BORDER, COLOR_ACCENT)
    );
    SearchLayout->addWidget(SearchInput);

    MainLayout->addWidget(SearchBar);

    // ============================================================
    //  Table
    // ============================================================
    PacketTable = new QTableWidget(this);
    PacketTable->setColumnCount(6);
    PacketTable->setHorizontalHeaderLabels({
        "Time", "Source IP", "Destination IP", "Src Port", "Dst Port", "Protocol"
    });

    PacketTable->horizontalHeader()->setStretchLastSection(false);
    PacketTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    PacketTable->verticalHeader()->setVisible(false);
    PacketTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    PacketTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    PacketTable->setSelectionMode(QAbstractItemView::SingleSelection);
    PacketTable->setAlternatingRowColors(true);
    PacketTable->setShowGrid(false);
    PacketTable->verticalHeader()->setDefaultSectionSize(24);
    PacketTable->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    PacketTable->setStyleSheet(
        QString("QTableWidget {"
                "  background-color: %1;"
                "  alternate-background-color: #1f1f1f;"
                "  color: %2;"
                "  gridline-color: %3;"
                "  font-family: Consolas;"
                "  font-size: 11px;"
                "  border: none;"
                "  outline: none;"
                "}"
                "QTableWidget::item {"
                "  padding: 4px 8px;"
                "  border: none;"
                "}"
                "QTableWidget::item:selected {"
                "  background-color: #2a3a35;"
                "  color: %4;"
                "}"
                "QHeaderView::section {"
                "  background-color: %5;"
                "  color: %6;"
                "  padding: 8px;"
                "  border: none;"
                "  border-bottom: 1px solid %3;"
                "  font-weight: 600;"
                "  font-size: 11px;"
                "}"
                "QScrollBar:vertical {"
                "  background: %1;"
                "  width: 8px;"
                "  border: none;"
                "}"
                "QScrollBar::handle:vertical {"
                "  background: #3e3e42;"
                "  border-radius: 4px;"
                "  min-height: 30px;"
                "}"
                "QScrollBar::handle:vertical:hover {"
                "  background: #505054;"
                "}"
                "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
                "  height: 0px;"
                "}"
                "QScrollBar:horizontal {"
                "  background: %1;"
                "  height: 8px;"
                "  border: none;"
                "}"
                "QScrollBar::handle:horizontal {"
                "  background: #3e3e42;"
                "  border-radius: 4px;"
                "}")
            .arg(COLOR_BG, COLOR_TEXT, COLOR_BORDER, COLOR_ACCENT,
                 COLOR_PANEL, COLOR_DIM)
    );

    MainLayout->addWidget(PacketTable, 1);

    // ============================================================
    //  Search connection
    // ============================================================
    connect(SearchInput, &QLineEdit::textChanged,
            this, &PacketPage::OnSearchChanged);

    // ============================================================
    //  Timer
    // ============================================================
    UpdateTimer = new QTimer(this);
    connect(UpdateTimer, &QTimer::timeout, this, &PacketPage::UpdateTable);
    UpdateTimer->start(200);
}

PacketPage::~PacketPage() {}

// ================================================================
//  Add Row
// ================================================================
void PacketPage::AddPacketRow(const QString &src, const QString &dst,
                              int sport, int dport,
                              const QString &proto, int len)
{
    PacketTable->insertRow(0);

    // ---- Time ----
    QString time = QTime::currentTime().toString("hh:mm:ss.zzz");
    PacketTable->setItem(0, 0, new QTableWidgetItem(time));

    // ---- Src/Dst ----
    PacketTable->setItem(0, 1, new QTableWidgetItem(src));
    PacketTable->setItem(0, 2, new QTableWidgetItem(dst));

    // ---- Ports ----
    PacketTable->setItem(0, 3, new QTableWidgetItem(QString::number(sport)));
    PacketTable->setItem(0, 4, new QTableWidgetItem(QString::number(dport)));

    // ---- Protocol + Color ----
    QTableWidgetItem *protoItem = new QTableWidgetItem(proto);

    if (proto == "TCP") {
        protoItem->setForeground(QColor(COLOR_ACCENT));
    } else if (proto == "UDP") {
        protoItem->setForeground(QColor(COLOR_INFO));
    } else if (proto == "ICMP") {
        protoItem->setForeground(QColor(COLOR_WARNING));
    } else {
        protoItem->setForeground(QColor(COLOR_DIM));
    }

    PacketTable->setItem(0, 5, protoItem);

    // ---- จำกัดจำนวนแถว ----
    if (PacketTable->rowCount() > MaximumRows) {
        PacketTable->removeRow(PacketTable->rowCount() - 1);
    }
}

// ================================================================
//  Update Table
// ================================================================
void PacketPage::UpdateTable() {
    PacketRecord rec;
    int added = 0;

    while (PopPacket(&rec)) {
        QString src   = IPToString(rec.SourceIP);
        QString dst   = IPToString(rec.DestinationIP);
        QString proto = "OTHER";

        if (rec.Protocol == 6)       proto = "TCP";
        else if (rec.Protocol == 17) proto = "UDP";
        else if (rec.Protocol == 1)  proto = "ICMP";

        AddPacketRow(src, dst,
                     rec.SourcePort, rec.DestinationPort,
                     proto, rec.Length);
        added++;
    }

    if (added > 0) {
        PacketTable->scrollToTop();
    }

    // ---- Update stats ----
    StatsLabel->setText(
        QString("%1 packets  ·  %2 shown")
            .arg(PacketTable->rowCount())
            .arg(PacketTable->rowCount())
    );
}

// ================================================================
//  Search
// ================================================================
void PacketPage::OnSearchChanged(const QString &text) {
    QString query = text.trimmed().toLower();

    for (int row = 0; row < PacketTable->rowCount(); ++row) {
        bool match = query.isEmpty();

        if (!match) {
            for (int col = 0; col < PacketTable->columnCount(); ++col) {
                QTableWidgetItem *item = PacketTable->item(row, col);
                if (item && item->text().toLower().contains(query)) {
                    match = true;
                    break;
                }
            }
        }

        PacketTable->setRowHidden(row, !match);
    }
}