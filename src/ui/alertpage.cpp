#include "alertpage.h"
#include "receiver/alert_queue.h"
#include <QVBoxLayout>
#include <QDateTime>
#include <QLabel>

AlertPage::AlertPage(QWidget *parent) : QWidget(parent) {
    this->setStyleSheet("background-color: #1a2127;");

    QVBoxLayout *Layout = new QVBoxLayout(this);
    Layout->setContentsMargins(10, 10, 10, 10);

    QLabel *Title = new QLabel("Security Alerts", this);
    Title->setStyleSheet("color: #ff5555; font-size: 18px; font-weight: bold; padding: 5px;");
    Layout->addWidget(Title);

    AlertList = new QListWidget(this);
    AlertList->setStyleSheet(
        "QListWidget {"
        "  background-color: #1a1a1a;"
        "  color: #ff8888;"
        "  font-family: Consolas;"
        "  font-size: 12px;"
        "  border: 1px solid #333;"
        "}"
        "QListWidget::item { padding: 6px; border-bottom: 1px solid #222; }"
        "QListWidget::item:selected { background-color: #442222; }"
    );
    Layout->addWidget(AlertList);

    UpdateTimer = new QTimer(this);
    connect(UpdateTimer, &QTimer::timeout, this, &AlertPage::UpdateAlerts);
    UpdateTimer->start(500);
}

AlertPage::~AlertPage() {}

void AlertPage::UpdateAlerts() {
    char Message[256];
    int MessageAdded = 0;

    while (PopAlert(Message, sizeof(Message))) {
        QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
        QString line = QString("[%1]  %2").arg(timeStr).arg(Message);

        AlertList->insertItem(0, line);
        MessageAdded++;

        if (AlertList->count() > MaxAlerts) {
            delete AlertList->takeItem(AlertList->count() - 1);
        }
    }

    if (MessageAdded > 0) {
        AlertList->scrollToTop();
    }
}