#include "sidebar.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QColor>
#include <QFrame>
#include <QEvent>

SideBar::SideBar(QWidget *parent) : QWidget(parent) {

    // ============ Sidebar Container ============
    setFixedWidth(68);                                 // ← กว้างขึ้น
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setObjectName("SideBar");
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("QWidget#SideBar { background-color: #181818; }");

    QVBoxLayout *Main_Layout = new QVBoxLayout(this);
    Main_Layout->setContentsMargins(10, 16, 10, 16);   // ← margin รอบข้าง 10px
    Main_Layout->setSpacing(10);                       // ← spacing 10px

    // ============ Button Group ============
    ButtonGroup = new QButtonGroup(this);
    ButtonGroup->setExclusive(true);

    // ============ สร้างปุ่มทั้งหมด ============
    DashBoardButton  = CreateButton("◆", "Dashboard",  0);
    Main_Layout->addWidget(DashBoardButton, 0, Qt::AlignHCenter);

    PacketPageButton = CreateButton("⬢", "Packets",    1);
    Main_Layout->addWidget(PacketPageButton, 0, Qt::AlignHCenter);

    TerminalButton   = CreateButton("⌨", "Terminal",  2);
    Main_Layout->addWidget(TerminalButton, 0, Qt::AlignHCenter);

    AlertpageButton  = CreateButton("▲", "Alerts",     3);
    Main_Layout->addWidget(AlertpageButton, 0, Qt::AlignHCenter);

    ManagementButton = CreateButton("⛭", "Management", 4);
    Main_Layout->addWidget(ManagementButton, 0, Qt::AlignHCenter);

    LogButton = CreateButton("☰", "Logs", 5);
    Main_Layout->addWidget(LogButton, 0, Qt::AlignHCenter);

    AIButton = CreateButton("✦", "AI Assistant", 6);
    Main_Layout->addWidget(AIButton, 0, Qt::AlignHCenter);

    // ============ Divider ============
    QFrame *Line = new QFrame(this);
    Line->setFixedHeight(1);
    Line->setStyleSheet("background-color: #2d3238; margin: 6px 4px;");
    Main_Layout->addWidget(Line);

    // ============ ดันปุ่มล่าง ============
    Main_Layout->addStretch();

    NotificationButton = CreateButton("⬤", "Notifications", 7);
    Main_Layout->addWidget(NotificationButton, 0, Qt::AlignHCenter);

    // ============ Settings ============
    SettingButton = CreateButton("✲", "Settings", 8);
    Main_Layout->addWidget(SettingButton, 0, Qt::AlignHCenter);

    // ============ Exit (ไม่นับใน group + มี glow แดง) ============
    ExitButton = new QPushButton("⏻", this);
    ExitButton->setFixedSize(44, 44);
    ExitButton->setToolTip("Exit");
    ExitButton->setCursor(Qt::PointingHandCursor);
    ExitButton->setStyleSheet(
        "QPushButton {"
        "  color: #6c757d;"
        "  font-size: 20px;"
        "  background: transparent;"
        "  border: 1px solid transparent;"
        "  border-radius: 10px;"
        "}"
        "QPushButton:hover {"
        "  color: #ffffff;"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #e74c3c, stop:1 #c0392b);"
        "  border: 1px solid #ff5555;"
        "}"
        "QPushButton:pressed {"
        "  background: #a93226;"
        "}"
    );

    // ✅ Glow แดงให้ปุ่ม Exit
    QGraphicsDropShadowEffect *ExitShadow = new QGraphicsDropShadowEffect(ExitButton);
    ExitShadow->setBlurRadius(15);
    ExitShadow->setColor(QColor(231, 76, 60, 0));
    ExitShadow->setOffset(0, 0);
    ExitButton->setGraphicsEffect(ExitShadow);

    // ✅ อัปเดต glow เมื่อ hover
    ExitButton->installEventFilter(this);

    connect(ExitButton, &QPushButton::clicked, qApp, &QApplication::quit);
    Main_Layout->addWidget(ExitButton, 0, Qt::AlignHCenter);

    // ============ ตั้งค่าเริ่มต้น ============
    DashBoardButton->setChecked(true);
    // ⚠️ ห้ามลบ effect — ให้ CreateButton จัดการเอง
}

// ============ Helper: สร้างปุ่ม Modern + Glow ============
QPushButton* SideBar::CreateButton(const QString &Icon, const QString &Tooltip, int Index) {

    QPushButton *Button = new QPushButton(Icon, this);
    Button->setFixedSize(44, 44);
    Button->setCheckable(true);
    Button->setToolTip(Tooltip);
    Button->setCursor(Qt::PointingHandCursor);

    Button->setStyleSheet(
        "QPushButton {"
        "  color: #6c757d;"
        "  font-size: 19px;"
        "  font-weight: 500;"
        "  background: transparent;"
        "  border: 1px solid transparent;"
        "  border-radius: 10px;"
        "  padding: 0px;"
        "}"
        "QPushButton:hover {"
        "  color: #ffffff;"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #2d3748, stop:1 #1f2937);"
        "  border: 1px solid #4a5568;"
        "}"
        "QPushButton:pressed {"
        "  background: #374151;"
        "}"
        "QPushButton:checked {"
        "  color: #00ffcc;"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #1a4d4a, stop:1 #0f2e2c);"
        "  border: 1px solid #00ffcc;"
        "}"
    );

    // ============ ✅ Glow Effect ============
    QGraphicsDropShadowEffect *Shadow = new QGraphicsDropShadowEffect(Button);
    Shadow->setBlurRadius(18);
    Shadow->setColor(QColor(0, 255, 204, 0));   // เริ่มที่ 0 (ไม่มี glow)
    Shadow->setOffset(0, 0);
    Button->setGraphicsEffect(Shadow);

    // ✅ อัปเดต glow ตามสถานะ checked
    QObject::connect(Button, &QPushButton::toggled, Button, [Shadow](bool checked) {
        if (checked) {
            Shadow->setColor(QColor(0, 255, 204, 180));   // glow เขียว
            Shadow->setBlurRadius(22);
        } else {
            Shadow->setColor(QColor(0, 255, 204, 0));     // ปิด glow
            Shadow->setBlurRadius(18);
        }
    });

    // ============ Connect ============
    connect(Button, &QPushButton::clicked, this, [this, Index]() {
        emit pageChangeRequested(Index);
    });

    ButtonGroup->addButton(Button, Index);

    return Button;
}

bool SideBar::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ExitButton) {
        QGraphicsDropShadowEffect *effect =
            qobject_cast<QGraphicsDropShadowEffect*>(ExitButton->graphicsEffect());

        if (effect) {
            if (event->type() == QEvent::Enter) {
                effect->setColor(QColor(231, 76, 60, 180));   // glow แดง
                effect->setBlurRadius(22);
            } else if (event->type() == QEvent::Leave) {
                effect->setColor(QColor(231, 76, 60, 0));     // ปิด
                effect->setBlurRadius(15);
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}