#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QDateTime>
#include <QFrame>
#include <QVector>
#include <QStringList>
#include <QFont>
#include <QDebug>

#include "alertpage.h"
#include "receiver/alert_queue.h"

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
//  Helper: แยกประเภทจากข้อความ alert
// ================================================================
AlertItem AlertPage::ParseAlert(const QString &raw) {
    AlertItem item;
    item.Timestamp = QDateTime::currentDateTime();
    item.Time = item.Timestamp.toString("hh:mm:ss");
    item.Message = raw;

    // ---- ตรวจจับ Severity จาก keyword ----
    QString upper = raw.toUpper();

    if (upper.contains("CRITICAL") ||
        upper.contains("BLOCKED") ||
        upper.contains("FLOOD") ||
        upper.contains("ATTACK")) {
        item.Severity = "CRITICAL";
    }
    else if (upper.contains("WARNING") ||
             upper.contains("SUSPICIOUS") ||
             upper.contains("RATE") ||
             upper.contains("ALERT")) {
        item.Severity = "WARNING";
    }
    else {
        item.Severity = "INFO";
    }

    // ---- ดึง IP Source จากข้อความ ----
    // format: "... from 192.168.1.103 to port 4444"
    int fromIdx = raw.indexOf("from ");
    if (fromIdx >= 0) {
        QString temp = raw.mid(fromIdx + 5).trimmed();
        int spaceIdx = temp.indexOf(' ');
        if (spaceIdx > 0) {
            item.Source = temp.left(spaceIdx);
        }
    }

    // ---- ดึง Port ----
    int portIdx = raw.indexOf("port ");
    if (portIdx >= 0) {
        QString temp = raw.mid(portIdx + 5).trimmed();
        int spaceIdx = temp.indexOf(' ');
        if (spaceIdx > 0) {
            item.Port = temp.left(spaceIdx);
        } else {
            item.Port = temp;
        }
    }

    // ---- Reason ----
    if (item.Severity == "CRITICAL") {
        item.Reason = "Blocked packet detected";
        item.Action = "BLOCKED via Firewall";
    } else if (item.Severity == "WARNING") {
        item.Reason = "Anomaly threshold exceeded";
        item.Action = "Alerted";
    } else {
        item.Reason = "Information";
        item.Action = "Logged";
    }

    return item;
}

// ================================================================
//  Helper: สีตาม Severity
// ================================================================
QString AlertPage::GetSeverityColor(const QString &severity) {
    if (severity == "CRITICAL") return COLOR_ERROR;
    if (severity == "WARNING")  return COLOR_WARNING;
    return COLOR_INFO;
}

// ================================================================
//  Constructor
// ================================================================
AlertPage::AlertPage(QWidget *parent) : QWidget(parent) {

    this->setStyleSheet(
        QString("AlertPage { background-color: %1; }").arg(COLOR_BG)
    );

    CurrentFilter = "ALL";

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
    TitleLayout->setSpacing(10);

    QLabel *TitleLabel = new QLabel("Security Alerts", TitleBar);
    TitleLabel->setStyleSheet(
        QString("color: %1; font-size: 14px; font-weight: 600;"
                "background: transparent; border: none;").arg(COLOR_TEXT)
    );
    TitleLayout->addWidget(TitleLabel);

    TitleLayout->addStretch();

    ClearButton = new QPushButton("Clear All", TitleBar);
    ClearButton->setCursor(Qt::PointingHandCursor);
    ClearButton->setFixedHeight(26);
    ClearButton->setStyleSheet(
        QString("QPushButton {"
                "  background-color: transparent;"
                "  color: %1;"
                "  font-size: 11px;"
                "  border: 1px solid %2;"
                "  border-radius: 4px;"
                "  padding: 0 12px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #3a2020;"
                "  border: 1px solid %1;"
                "}").arg(COLOR_ERROR, COLOR_BORDER)
    );
    TitleLayout->addWidget(ClearButton);

    MainLayout->addWidget(TitleBar);

    // ============================================================
    //  Splitter
    // ============================================================
    QSplitter *Splitter = new QSplitter(Qt::Horizontal, this);
    Splitter->setStyleSheet(
        QString("QSplitter::handle {"
                "  background-color: %1;"
                "  width: 1px;"
                "}").arg(COLOR_BORDER)
    );

    // ============================================================
    //  LEFT: List Panel
    // ============================================================
    QWidget *LeftPanel = new QWidget(Splitter);
    LeftPanel->setStyleSheet(
        QString("background-color: %1;").arg(COLOR_BG)
    );
    QVBoxLayout *LeftLayout = new QVBoxLayout(LeftPanel);
    LeftLayout->setContentsMargins(0, 0, 0, 0);
    LeftLayout->setSpacing(0);

    // ---- Search bar ----
    QWidget *SearchBar = new QWidget(LeftPanel);
    SearchBar->setFixedHeight(42);
    SearchBar->setStyleSheet(
        QString("background-color: %1;"
                "border-bottom: 1px solid %2;").arg(COLOR_BG, COLOR_BORDER)
    );
    QHBoxLayout *SearchLayout = new QHBoxLayout(SearchBar);
    SearchLayout->setContentsMargins(10, 6, 10, 6);

    SearchInput = new QLineEdit(SearchBar);
    SearchInput->setPlaceholderText("Search Alerts Message");
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

    LeftLayout->addWidget(SearchBar);

    // ---- Filter buttons ----
    QWidget *FilterBar = new QWidget(LeftPanel);
    FilterBar->setFixedHeight(38);
    FilterBar->setStyleSheet(
        QString("background-color: %1;"
                "border-bottom: 1px solid %2;").arg(COLOR_BG, COLOR_BORDER)
    );
    QHBoxLayout *FilterLayout = new QHBoxLayout(FilterBar);
    FilterLayout->setContentsMargins(10, 4, 10, 4);
    FilterLayout->setSpacing(4);

    auto MakeFilterBtn = [](const QString &label, QWidget *parent) -> QPushButton* {
        QPushButton *btn = new QPushButton(label, parent);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(24);
        btn->setStyleSheet(
            QString("QPushButton {"
                    "  background-color: transparent;"
                    "  color: %1;"
                    "  font-size: 10px;"
                    "  border: 1px solid %2;"
                    "  border-radius: 3px;"
                    "  padding: 0 10px;"
                    "}"
                    "QPushButton:hover {"
                    "  background-color: %3;"
                    "}"
                    "QPushButton:checked {"
                    "  background-color: #2a3a35;"
                    "  color: %4;"
                    "  border: 1px solid %4;"
                    "}").arg(COLOR_DIM, COLOR_BORDER, COLOR_PANEL, COLOR_ACCENT)
        );
        return btn;
    };

    FilterAll      = MakeFilterBtn("All",      FilterBar);
    FilterCritical = MakeFilterBtn("🚨",       FilterBar);
    FilterWarning  = MakeFilterBtn("⚠",        FilterBar);
    FilterInfo     = MakeFilterBtn("ℹ",        FilterBar);

    FilterAll->setChecked(true);

    FilterLayout->addWidget(FilterAll);
    FilterLayout->addWidget(FilterCritical);
    FilterLayout->addWidget(FilterWarning);
    FilterLayout->addWidget(FilterInfo);
    FilterLayout->addStretch();

    LeftLayout->addWidget(FilterBar);

    // ---- List ----
    AlertList = new QListWidget(LeftPanel);
    AlertList->setStyleSheet(
        QString("QListWidget {"
                "  background-color: %1;"
                "  color: %2;"
                "  border: none;"
                "  outline: none;"
                "  padding: 4px;"
                "}"
                "QListWidget::item {"
                "  padding: 8px 10px;"
                "  border-bottom: 1px solid %3;"
                "  border-radius: 3px;"
                "}"
                "QListWidget::item:hover {"
                "  background-color: %4;"
                "}"
                "QListWidget::item:selected {"
                "  background-color: #2a3a35;"
                "  border-left: 2px solid %5;"
                "}").arg(COLOR_BG, COLOR_TEXT, COLOR_BORDER, COLOR_PANEL, COLOR_ACCENT)
    );
    AlertList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    LeftLayout->addWidget(AlertList, 1);

    // ============================================================
    //  RIGHT: Detail Panel
    // ============================================================
    QWidget *RightPanel = new QWidget(Splitter);
    RightPanel->setStyleSheet(
        QString("background-color: %1;").arg(COLOR_BG)
    );
    QVBoxLayout *RightLayout = new QVBoxLayout(RightPanel);
    RightLayout->setContentsMargins(24, 20, 24, 20);
    RightLayout->setSpacing(14);

    // ---- Placeholder (แสดงเมื่อยังไม่เลือก) ----
    DetailPlaceholder = new QLabel("Select an alert to view details", RightPanel);
    DetailPlaceholder->setAlignment(Qt::AlignCenter);
    DetailPlaceholder->setStyleSheet(
        QString("color: %1; font-size: 13px; background: transparent;").arg(COLOR_DIM)
    );
    RightLayout->addWidget(DetailPlaceholder, 1);

    // ---- Detail Header (ซ่อนไว้ก่อน) ----
    DetailHeader = new QLabel(RightPanel);
    DetailHeader->setStyleSheet(
        QString("color: %1; font-size: 18px; font-weight: bold;"
                "background: transparent;").arg(COLOR_TEXT)
    );
    DetailHeader->hide();
    RightLayout->addWidget(DetailHeader);

    // ---- Timestamp ----
    DetailTime = new QLabel(RightPanel);
    DetailTime->setStyleSheet(
        QString("color: %1; font-size: 11px; font-family: Consolas;"
                "background: transparent;").arg(COLOR_DIM)
    );
    DetailTime->hide();
    RightLayout->addWidget(DetailTime);

    // ---- Divider ----
    QFrame *Div1 = new QFrame(RightPanel);
    Div1->setFixedHeight(1);
    Div1->setStyleSheet(QString("background-color: %1;").arg(COLOR_BORDER));
    Div1->hide();
    RightLayout->addWidget(Div1);

    // ---- Detail Fields ----
    auto MakeDetailRow = [RightPanel](const QString &label, QLabel *&outValue) {
        QWidget *Row = new QWidget(RightPanel);
        QHBoxLayout *RLayout = new QHBoxLayout(Row);
        RLayout->setContentsMargins(0, 4, 0, 4);
        RLayout->setSpacing(12);

        QLabel *Lbl = new QLabel(label, Row);
        Lbl->setFixedWidth(110);
        Lbl->setStyleSheet(
            QString("color: %1; font-size: 11px; background: transparent;")
                .arg(COLOR_DIM)
        );
        RLayout->addWidget(Lbl);

        outValue = new QLabel("—", Row);
        outValue->setStyleSheet(
            QString("color: %1; font-size: 12px; font-family: Consolas;"
                    "background: transparent;").arg(COLOR_TEXT)
        );
        outValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
        RLayout->addWidget(outValue, 1);

        Row->hide();
        return Row;
    };

    QWidget *RowSeverity = MakeDetailRow("Severity",  DetailSeverity);
    QWidget *RowSource   = MakeDetailRow("Source",    DetailSource);
    QWidget *RowDest     = MakeDetailRow("Destination", DetailDest);
    QWidget *RowPort     = MakeDetailRow("Port",      DetailPort);
    QWidget *RowReason   = MakeDetailRow("Reason",    DetailReason);
    QWidget *RowAction   = MakeDetailRow("Action",    DetailAction);

    RightLayout->addWidget(RowSeverity);
    RightLayout->addWidget(RowSource);
    RightLayout->addWidget(RowDest);
    RightLayout->addWidget(RowPort);
    RightLayout->addWidget(RowReason);
    RightLayout->addWidget(RowAction);

    RightLayout->addStretch();

    // ============================================================
    //  Splitter sizes
    // ============================================================
    Splitter->addWidget(LeftPanel);
    Splitter->addWidget(RightPanel);
    Splitter->setStretchFactor(0, 2);
    Splitter->setStretchFactor(1, 3);
    Splitter->setSizes({300, 500});

    MainLayout->addWidget(Splitter, 1);

    // ============================================================
    //  Connect
    // ============================================================
    connect(AlertList, &QListWidget::itemSelectionChanged,
            this, &AlertPage::OnAlertSelected);

    connect(ClearButton, &QPushButton::clicked, this, &AlertPage::OnClearAll);

    connect(FilterAll,      &QPushButton::clicked, this, [this]() {
        CurrentFilter = "ALL";      FilterAll->setChecked(true);
        FilterCritical->setChecked(false);
        FilterWarning->setChecked(false);
        FilterInfo->setChecked(false);
        RefreshList();
    });
    connect(FilterCritical, &QPushButton::clicked, this, [this]() {
        CurrentFilter = "CRITICAL"; FilterCritical->setChecked(true);
        FilterAll->setChecked(false);
        FilterWarning->setChecked(false);
        FilterInfo->setChecked(false);
        RefreshList();
    });
    connect(FilterWarning,  &QPushButton::clicked, this, [this]() {
        CurrentFilter = "WARNING";  FilterWarning->setChecked(true);
        FilterAll->setChecked(false);
        FilterCritical->setChecked(false);
        FilterInfo->setChecked(false);
        RefreshList();
    });
    connect(FilterInfo,     &QPushButton::clicked, this, [this]() {
        CurrentFilter = "INFO";     FilterInfo->setChecked(true);
        FilterAll->setChecked(false);
        FilterCritical->setChecked(false);
        FilterWarning->setChecked(false);
        RefreshList();
    });

    connect(SearchInput, &QLineEdit::textChanged,
            this, &AlertPage::OnSearchChanged);

    // ============================================================
    //  Timer
    // ============================================================
    UpdateTimer = new QTimer(this);
    connect(UpdateTimer, &QTimer::timeout, this, &AlertPage::UpdateAlerts);
    UpdateTimer->start(500);
}

AlertPage::~AlertPage() {}

// ================================================================
//  Update: ดึง alert ใหม่จาก queue
// ================================================================
void AlertPage::UpdateAlerts() {
    char msg[512];
    int added = 0;

    while (PopAlert(msg, sizeof(msg))) {
        AlertItem item = ParseAlert(QString::fromUtf8(msg));
        Alerts.append(item);
        added++;

        if (Alerts.size() > 500) {
            Alerts.removeFirst();
        }
    }

    if (added > 0) {
        RefreshList();
    }
}

// ================================================================
//  Refresh: สร้าง list ใหม่ตาม filter + search
// ================================================================
void AlertPage::RefreshList() {
    AlertList->clear();

    QString search = SearchInput->text().toLower();

    // ---- ไล่จากใหม่ → เก่า ----
    for (int i = Alerts.size() - 1; i >= 0; --i) {
        const AlertItem &item = Alerts[i];

        if (!MatchesFilter(item)) continue;

        if (!search.isEmpty()) {
            bool found = item.Message.toLower().contains(search) ||
                         item.Source.toLower().contains(search) ||
                         item.Port.contains(search);
            if (!found) continue;
        }

        // ---- Icon ----
        QString icon = "ℹ";
        if (item.Severity == "CRITICAL") icon = "🚨";
        else if (item.Severity == "WARNING") icon = "⚠";

        // ---- สร้าง item text ----
        QString text = QString("%1  [%2]  %3\n     %4")
            .arg(icon)
            .arg(item.Time)
            .arg(item.Severity)
            .arg(item.Message.left(80));

        QListWidgetItem *lwi = new QListWidgetItem(text, AlertList);

        // ---- เก็บ index ----
        lwi->setData(Qt::UserRole, i);

        // ---- สีตาม Severity ----
        if (item.Severity == "CRITICAL") {
            lwi->setForeground(QColor(COLOR_ERROR));
        } else if (item.Severity == "WARNING") {
            lwi->setForeground(QColor(COLOR_WARNING));
        } else {
            lwi->setForeground(QColor(COLOR_INFO));
        }

        // ---- Font Monospace ----
        QFont f("Consolas", 9);
        lwi->setFont(f);
    }
}

// ================================================================
//  Filter check
// ================================================================
bool AlertPage::MatchesFilter(const AlertItem &item) {
    if (CurrentFilter == "ALL") return true;
    return item.Severity == CurrentFilter;
}

// ================================================================
//  Search
// ================================================================
void AlertPage::OnSearchChanged(const QString &) {
    RefreshList();
}

// ================================================================
//  Filter change
// ================================================================
void AlertPage::OnFilterChanged(int) {
    RefreshList();
}

// ================================================================
//  Alert selected → แสดง detail
// ================================================================
void AlertPage::OnAlertSelected() {
    QListWidgetItem *lwi = AlertList->currentItem();
    if (!lwi) return;

    int idx = lwi->data(Qt::UserRole).toInt();
    if (idx < 0 || idx >= Alerts.size()) return;

    UpdateDetailPanel(idx);
}

// ================================================================
//  Update Detail Panel
// ================================================================
void AlertPage::UpdateDetailPanel(int index) {
    const AlertItem &item = Alerts[index];

    // ---- ซ่อน placeholder ----
    DetailPlaceholder->hide();

    // ---- แสดง header ----
    DetailHeader->setText(QString("🚨  %1 Alert").arg(item.Severity));
    DetailHeader->setStyleSheet(
        QString("color: %1; font-size: 18px; font-weight: bold;"
                "background: transparent;").arg(GetSeverityColor(item.Severity))
    );
    DetailHeader->show();

    DetailTime->setText(QString("📅  %1").arg(item.Timestamp.toString("dd/MM/yyyy  hh:mm:ss")));
    DetailTime->show();

    // ---- เก็บ widget ของ row ที่ show ----
    // ค้นหา row ทั้งหมด
    QList<QWidget*> allRows = this->findChildren<QWidget*>();

    DetailSeverity->setText(item.Severity);
    DetailSeverity->setStyleSheet(
        QString("color: %1; font-size: 12px; font-family: Consolas;"
                "font-weight: bold; background: transparent;")
            .arg(GetSeverityColor(item.Severity))
    );
    DetailSource->setText(item.Source.isEmpty() ? "—" : item.Source);
    DetailDest->setText(item.Destination.isEmpty() ? "—" : item.Destination);
    DetailPort->setText(item.Port.isEmpty() ? "—" : item.Port);
    DetailReason->setText(item.Reason);
    DetailAction->setText(item.Action);

    // ---- แสดง row ทุกตัว ----
    for (QWidget *w : allRows) {
        QHBoxLayout *layout = qobject_cast<QHBoxLayout*>(w->layout());
        if (!layout) continue;
        if (layout->count() == 2) {
            QLabel *lbl = qobject_cast<QLabel*>(layout->itemAt(0)->widget());
            if (lbl && (lbl->text() == "Severity" ||
                        lbl->text() == "Source" ||
                        lbl->text() == "Destination" ||
                        lbl->text() == "Port" ||
                        lbl->text() == "Reason" ||
                        lbl->text() == "Action")) {
                w->show();
            }
        }
    }
}

// ================================================================
//  Clear All
// ================================================================
void AlertPage::OnClearAll() {
    Alerts.clear();
    AlertList->clear();

    // ---- ซ่อน detail ----
    DetailHeader->hide();
    DetailTime->hide();
    DetailSeverity->parentWidget()->hide();
    DetailSource->parentWidget()->hide();
    DetailDest->parentWidget()->hide();
    DetailPort->parentWidget()->hide();
    DetailReason->parentWidget()->hide();
    DetailAction->parentWidget()->hide();
    DetailPlaceholder->show();
}