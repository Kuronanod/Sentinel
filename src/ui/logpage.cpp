#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QDateTime>
#include <QTextStream>
#include <QFile>
#include <QDebug>

#include "logpage.h"
#include "receiver/log_queue.h"

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
#define COLOR_DEBUG   "#808080"

// ================================================================
//  Constructor
// ================================================================
LogPage::LogPage(QWidget *parent) : QWidget(parent) {

    this->setStyleSheet(
        QString("LogPage { background-color: %1; }").arg(COLOR_BG)
    );

    QVBoxLayout *RootLayout = new QVBoxLayout(this);
    RootLayout->setContentsMargins(0, 0, 0, 0);
    RootLayout->setSpacing(0);

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

    QLabel *TitleLabel = new QLabel("Terminal History Logs", TitleBar);
    TitleLabel->setStyleSheet(
        QString("color: %1; font-size: 14px; font-weight: 600;"
                "background: transparent; border: none;").arg(COLOR_TEXT)
    );
    TitleLayout->addWidget(TitleLabel);
    TitleLayout->addStretch();

    // ---- Status ----
    StatusLabel = new QLabel("0 lines", TitleBar);
    StatusLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; font-family: Consolas;"
                "background: transparent; border: none;").arg(COLOR_DIM)
    );
    TitleLayout->addWidget(StatusLabel);

    // ---- Pause ----
    PauseBtn = new QPushButton("Pause", TitleBar);
    PauseBtn->setCheckable(true);
    PauseBtn->setCursor(Qt::PointingHandCursor);
    PauseBtn->setFixedHeight(26);
    PauseBtn->setStyleSheet(
        QString("QPushButton {"
                "  background-color: transparent;"
                "  color: %1;"
                "  font-size: 11px;"
                "  border: 1px solid %2;"
                "  border-radius: 4px;"
                "  padding: 0 12px;"
                "}"
                "QPushButton:hover { background-color: #2a3a35; }"
                "QPushButton:checked {"
                "  background-color: #2a3a35;"
                "  color: %3;"
                "  border: 1px solid %3;"
                "}").arg(COLOR_DIM, COLOR_BORDER, COLOR_WARNING)
    );
    TitleLayout->addWidget(PauseBtn);

    // ---- Export ----
    ExportBtn = new QPushButton("Export", TitleBar);
    ExportBtn->setCursor(Qt::PointingHandCursor);
    ExportBtn->setFixedHeight(26);
    ExportBtn->setStyleSheet(
        QString("QPushButton {"
                "  background-color: transparent;"
                "  color: %1;"
                "  font-size: 11px;"
                "  border: 1px solid %2;"
                "  border-radius: 4px;"
                "  padding: 0 12px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #2a3a35;"
                "  border: 1px solid %3;"
                "  color: %3;"
                "}").arg(COLOR_DIM, COLOR_BORDER, COLOR_ACCENT)
    );
    TitleLayout->addWidget(ExportBtn);

    // ---- Clear ----
    ClearBtn = new QPushButton("Clear", TitleBar);
    ClearBtn->setCursor(Qt::PointingHandCursor);
    ClearBtn->setFixedHeight(26);
    ClearBtn->setStyleSheet(
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
    TitleLayout->addWidget(ClearBtn);

    RootLayout->addWidget(TitleBar);

    // ============================================================
    //  Search + Filter Bar
    // ============================================================
    QWidget *FilterBar = new QWidget(this);
    FilterBar->setFixedHeight(42);
    FilterBar->setStyleSheet(
        QString("background-color: %1;"
                "border-bottom: 1px solid %2;").arg(COLOR_BG, COLOR_BORDER)
    );
    QHBoxLayout *FilterLayout = new QHBoxLayout(FilterBar);
    FilterLayout->setContentsMargins(10, 6, 10, 6);
    FilterLayout->setSpacing(8);

    SearchInput = new QLineEdit(FilterBar);
    SearchInput->setPlaceholderText("Search Historic Logs Message");
    SearchInput->setStyleSheet(
        QString("QLineEdit {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-size: 11px;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 5px 10px;"
                "}"
                "QLineEdit:focus { border: 1px solid %4; }")
            .arg(COLOR_PANEL, COLOR_TEXT, COLOR_BORDER, COLOR_ACCENT)
    );
    FilterLayout->addWidget(SearchInput, 1);

    RootLayout->addWidget(FilterBar);

    // ============================================================
    //  Log View
    // ============================================================
    LogView = new QTextEdit(this);
    LogView->setReadOnly(true);
    LogView->setStyleSheet(
        QString("QTextEdit {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-family: Consolas;"
                "  font-size: 11px;"
                "  border: none;"
                "  padding: 8px 12px;"
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
                "}").arg(COLOR_BG, COLOR_TEXT)
    );
    RootLayout->addWidget(LogView, 1);

    // ============================================================
    //  Timer + Connect
    // ============================================================
    UpdateTimer = new QTimer(this);
    connect(UpdateTimer, &QTimer::timeout, this, &LogPage::UpdateLogs);
    UpdateTimer->start(200);

    connect(SearchInput, &QLineEdit::textChanged, this, &LogPage::OnSearchChanged);
    connect(ClearBtn,    &QPushButton::clicked,    this, &LogPage::OnClearClicked);
    connect(ExportBtn,   &QPushButton::clicked,    this, &LogPage::OnExportClicked);
    connect(PauseBtn,    &QPushButton::toggled,    this, &LogPage::OnPauseToggled);

    // ---- Welcome ----
    LogView->append(
        QString("<span style='color:%1;'>═══ Sentinel Log Viewer ═══</span>")
            .arg(COLOR_DIM)
    );
}

LogPage::~LogPage() {}

// ================================================================
//  Build HTML
// ================================================================
QString LogPage::BuildHtml(int level, const QString &time, const QString &msg) {
    QString color;
    QString label;

    switch (level) {
        case 0: color = COLOR_DEBUG;   label = "DBG"; break;
        case 1: color = COLOR_INFO;    label = "INF"; break;
        case 2: color = COLOR_WARNING; label = "WRN"; break;
        case 3: color = COLOR_ERROR;   label = "CRT"; break;
        default: color = COLOR_TEXT;    label = "???"; break;
    }

    // Escape HTML
    QString safeMsg = msg.toHtmlEscaped();

    return QString(
        "<span style='color:%1;'>[%2]</span>"
        " <span style='color:%3;'>[%4]</span>"
        " <span style='color:%5;'>%6</span>")
        .arg(COLOR_DIM, time, color, label, COLOR_TEXT, safeMsg);
}

// ================================================================
//  Update Logs
// ================================================================
void LogPage::UpdateLogs() {
    if (Paused) return;

    char msg[512];
    char timeStr[32];
    int  level = 0;
    int  added = 0;

    QString search = SearchInput->text().toLower();

    while (LogPop(msg, sizeof(msg), &level, timeStr)) {
        // ---- Filter by level ----
        if (level == 0 && !ShowDebug) continue;
        if (level == 1 && !ShowInfo)  continue;
        if (level == 2 && !ShowWarn)  continue;
        if (level == 3 && !ShowCrit)  continue;

        QString qmsg = QString::fromUtf8(msg);

        // ---- Filter by search ----
        if (!search.isEmpty() && !qmsg.toLower().contains(search)) continue;

        // ---- Append ----
        LogView->append(BuildHtml(level, QString(timeStr), qmsg));
        added++;
        TotalLines++;
    }

    if (added > 0) {
        // ---- Auto-scroll ----
        QScrollBar *bar = LogView->verticalScrollBar();
        bar->setValue(bar->maximum());

        StatusLabel->setText(
            QString("%1 lines").arg(TotalLines)
        );
    }
}

// ================================================================
//  Search
// ================================================================
void LogPage::OnSearchChanged(const QString &) {
    // Repaint existing? (ง่ายสุด: แค่ปล่อยให้ log ใหม่ถูก filter)
    // ถ้าอยาก search ย้อนหลัง → ต้องเก็บ log ใน memory แล้ว rebuild
}

// ================================================================
//  Clear
// ================================================================
void LogPage::OnClearClicked() {
    auto reply = QMessageBox::question(this, "Clear Logs",
        "Clear all displayed logs? (file จะไม่ถูกลบ)",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    LogView->clear();
    LogClearQueue();
    TotalLines = 0;
    StatusLabel->setText("0 lines");
}

// ================================================================
//  Export
// ================================================================
void LogPage::OnExportClicked() {
    QString path = QFileDialog::getSaveFileName(
        this, "Export Logs",
        QDateTime::currentDateTime().toString("'sentinel_'yyyy-MM-dd_hh-mm-ss'.log'"),
        "Log Files (*.log *.txt)"
    );

    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error",
            "Cannot open file for writing");
        return;
    }

    QTextStream out(&file);
    out << LogView->toPlainText();
    file.close();

    QMessageBox::information(this, "Exported",
        QString("Logs exported to:\n%1").arg(path));
}

// ================================================================
//  Pause
// ================================================================
void LogPage::OnPauseToggled(bool paused) {
    Paused = paused;
    PauseBtn->setText(paused ? "Resume" : "Pause");
}