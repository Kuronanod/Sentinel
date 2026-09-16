#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QGraphicsDropShadowEffect>

class SideBar : public QWidget {

    Q_OBJECT

public:
    explicit SideBar(QWidget *parent = nullptr);

signals:
    void pageChangeRequested(int index);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;   // ← สำหรับ Exit hover

private:
    QPushButton *DashBoardButton;
    QPushButton *PacketPageButton;
    QPushButton *TerminalButton;
    QPushButton *ManagementButton;
    QPushButton *AlertpageButton;
    QPushButton *LogButton;
    QPushButton *AIButton;
    QPushButton *NotificationButton;
    QPushButton *SettingButton;
    QPushButton *ExitButton;

    QButtonGroup *ButtonGroup;

    QPushButton *CreateButton(const QString &Icon, const QString &Tooltip, int Index);
};

#endif