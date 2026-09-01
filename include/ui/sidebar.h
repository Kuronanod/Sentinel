#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

class SideBar : public QWidget{

    Q_OBJECT

public:
    explicit SideBar(QWidget *parent = nullptr);

signals:
    void pageChangeRequested(int index);

private:
    QPushButton *DashBoardButton;
    QPushButton *TerminalButton;
    QPushButton *ManagementButton;
    QPushButton *LogInformationButton;
    QPushButton *SettingButton;

};

#endif