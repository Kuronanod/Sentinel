#include "notificationpage.h"
#include "receiver/notification_queue.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QDateTime>

// ================================================================
//  Palette
// ================================================================
#define COLOR_BG       "#1a1a1a"
#define COLOR_PANEL    "#212121"
#define COLOR_BORDER   "#2d2d2d"
#define COLOR_TEXT     "#e0e0e0"
#define COLOR_DIM      "#707070"
#define COLOR_ACCENT   "#4ec9b0"
#define COLOR_INFO     "#6cb6ff"
#define COLOR_WARNING  "#dcdcaa"
#define COLOR_ERROR    "#f48771"

// ================================================================
//  Constructor
// ================================================================
NotificationPage::NotificationPage(QWidget *parent) : QWidget(parent) {

    this->setStyleSheet(
        QString("NotificationPage { background-color: %1; }").arg(COLOR_BG)
    );

    QVBoxLayout *Root = new QVBoxLayout(this);
    Root->setContentsMargins(0, 0, 0, 0);
    Root->setSpacing(0);

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

    QLabel *TitleLabel = new QLabel("🔔  Notifications", TitleBar);
    TitleLabel->setStyleSheet(
        QString("color: %1; font-size: 14px; font-weight: 600;"
                "background: transparent; border: none;").arg(COLOR_TEXT)
    );
    TitleLayout->addWidget(TitleLabel);
    TitleLayout->addStretch();

    CountLabel = new QLabel("0 notifications", TitleBar);
    CountLabel->setStyleSheet(
        QString("color: %1; font-size: 11px;"
                "background: transparent; border: none;").arg(COLOR_DIM)
    );
    TitleLayout->addWidget(CountLabel);

    // ---- Mark All ----
    QPushButton *MarkBtn = new QPushButton("✓✓  Mark All Read", TitleBar);
    MarkBtn->setCursor(Qt::PointingHandCursor);
    MarkBtn->setFixedHeight(26);
    MarkBtn->setStyleSheet(
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
    TitleLayout->addWidget(MarkBtn);

    // ---- Clear ----
    QPushButton *ClearBtn = new QPushButton("🗑  Clear All", TitleBar);
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

    Root->addWidget(TitleBar);

    // ============================================================
    //  Scroll List
    // ============================================================
    QScrollArea *Scroll = new QScrollArea(this);
    Scroll->setWidgetResizable(true);
    Scroll->setFrameShape(QFrame::NoFrame);
    Scroll->setStyleSheet(
        QString("QScrollArea {"
                "  background: %1;"
                "  border: none;"
                "}"
                "QScrollBar:vertical {"
                "  background: %1;"
                "  width: 10px;"
                "  border: none;"
                "}"
                "QScrollBar::handle:vertical {"
                "  background: #3e3e42;"
                "  border-radius: 5px;"
                "  min-height: 40px;"
                "}"
                "QScrollBar::handle:vertical:hover {"
                "  background: #505054;"
                "}"
                "QScrollBar::add-line:vertical,"
                "QScrollBar::sub-line:vertical {"
                "  height: 0px;"
                "}").arg(COLOR_BG)
    );

    QWidget *ListWidget = new QWidget();
    ListWidget->setStyleSheet(QString("background-color: %1;").arg(COLOR_BG));

    ListLayout = new QVBoxLayout(ListWidget);
    ListLayout->setContentsMargins(20, 20, 20, 20);
    ListLayout->setSpacing(10);

    // ---- Empty state ----
    EmptyLabel = new QLabel("🔕\n\nNo notifications yet", ListWidget);
    EmptyLabel->setAlignment(Qt::AlignCenter);
    EmptyLabel->setStyleSheet(
        QString("color: %1; font-size: 14px; padding: 100px 0;"
                "background: transparent;").arg(COLOR_DIM)
    );
    ListLayout->addWidget(EmptyLabel);
    ListLayout->addStretch();

    Scroll->setWidget(ListWidget);
    Root->addWidget(Scroll, 1);

    // ============================================================
    //  Connect
    // ============================================================
    connect(MarkBtn,  &QPushButton::clicked, this, &NotificationPage::OnMarkAllRead);
    connect(ClearBtn, &QPushButton::clicked, this, &NotificationPage::OnClearAll);

    // ============================================================
    //  Poll Timer
    // ============================================================
    QTimer *Timer = new QTimer(this);
    connect(Timer, &QTimer::timeout, this, &NotificationPage::PollNotifications);
    Timer->start(500);
}

NotificationPage::~NotificationPage() {}

// ================================================================
//  Poll Notifications
// ================================================================
void NotificationPage::PollNotifications() {
    NotificationData data;
    int added = 0;

    while (PopNotification(&data)) {
        AddNotification(
            data.type,
            QString::fromUtf8(data.title),
            QString::fromUtf8(data.message),
            QString::fromUtf8(data.time)
        );
        added++;
    }

    if (added > 0) {
        RebuildList();
    }
}

// ================================================================
//  Add Notification
// ================================================================
void NotificationPage::AddNotification(int type,
                                       const QString &title,
                                       const QString &message,
                                       const QString &time)
{
    NotifItem item;
    item.type    = type;
    item.title   = title;
    item.message = message;
    item.time    = time;
    item.isNew   = true;

    Items.prepend(item);

    while (Items.size() > MAX_ITEMS) {
        Items.removeLast();
    }

    EmptyLabel->hide();
}

// ================================================================
//  Rebuild List
// ================================================================
void NotificationPage::RebuildList() {
    // ---- ลบของเก่า ----
    QLayoutItem *child;
    int safety = 0;
    while ((child = ListLayout->takeAt(0)) != nullptr) {
        if (child->widget() && child->widget() != EmptyLabel) {
            delete child->widget();
        }
        delete child;
        if (++safety > 500) break;
    }

    // ---- Empty? ----
    if (Items.isEmpty()) {
        EmptyLabel->show();
        ListLayout->addWidget(EmptyLabel);
        ListLayout->addStretch();
        CountLabel->setText("0 notifications");
        return;
    }

    // ---- สร้างใหม่ ----
    for (int i = 0; i < Items.size(); i++) {
        ListLayout->addWidget(BuildItemWidget(Items[i]));
    }
    ListLayout->addStretch();

    // ---- Update count ----
    int unread = 0;
    for (const auto &it : Items) if (it.isNew) unread++;

    if (unread > 0) {
        CountLabel->setText(
            QString("%1 notifications  ·  %2 unread")
                .arg(Items.size()).arg(unread)
        );
    } else {
        CountLabel->setText(QString("%1 notifications").arg(Items.size()));
    }
}

// ================================================================
//  Build Item Widget
// ================================================================
QWidget* NotificationPage::BuildItemWidget(const NotifItem &item) {
    QString icon, accent;
    if (item.type == 0) {       // ALERT
        icon = "🚨"; accent = COLOR_ERROR;
    } else if (item.type == 1) { // AI
        icon = "🤖"; accent = COLOR_INFO;
    } else {                     // SYSTEM
        icon = "✓";  accent = COLOR_ACCENT;
    }

    QWidget *W = new QWidget();
    W->setStyleSheet(
        QString("background-color: %1;"
                "border: 1px solid %2;"
                "border-left: 3px solid %3;"
                "border-radius: 6px;")
            .arg(item.isNew ? "#1e2428" : COLOR_PANEL,
                 COLOR_BORDER,
                 item.isNew ? accent : COLOR_BORDER)
    );

    QHBoxLayout *H = new QHBoxLayout(W);
    H->setContentsMargins(14, 12, 14, 12);
    H->setSpacing(12);

    // ---- Icon ----
    QLabel *IconLabel = new QLabel(icon, W);
    IconLabel->setFixedSize(28, 28);
    IconLabel->setAlignment(Qt::AlignCenter);
    IconLabel->setStyleSheet(
        QString("font-size: 18px; background: transparent; border: none;")
    );
    H->addWidget(IconLabel);

    // ---- Content ----
    QVBoxLayout *V = new QVBoxLayout();
    V->setSpacing(4);

    QHBoxLayout *TopRow = new QHBoxLayout();
    TopRow->setSpacing(8);

    QLabel *TitleLabel = new QLabel(item.title, W);
    TitleLabel->setStyleSheet(
        QString("color: %1; font-size: 13px; font-weight: 600;"
                "background: transparent; border: none;").arg(COLOR_TEXT)
    );
    TopRow->addWidget(TitleLabel);
    TopRow->addStretch();

    QLabel *TimeLabel = new QLabel(item.time, W);
    TimeLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; font-family: Consolas;"
                "background: transparent; border: none;").arg(COLOR_DIM)
    );
    TopRow->addWidget(TimeLabel);

    V->addLayout(TopRow);

    QLabel *MsgLabel = new QLabel(item.message, W);
    MsgLabel->setWordWrap(true);
    MsgLabel->setStyleSheet(
        QString("color: %1; font-size: 12px;"
                "background: transparent; border: none;").arg(COLOR_DIM)
    );
    V->addWidget(MsgLabel);

    H->addLayout(V, 1);

    return W;
}

// ================================================================
//  Mark All Read
// ================================================================
void NotificationPage::OnMarkAllRead() {
    for (auto &item : Items) item.isNew = false;
    RebuildList();
}

// ================================================================
//  Clear All
// ================================================================
void NotificationPage::OnClearAll() {
    Items.clear();
    ClearNotifications();
    RebuildList();
}