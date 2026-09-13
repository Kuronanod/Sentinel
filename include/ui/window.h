#ifndef WINDOW_H
#define WINDOW_H

#include "sidebar.h"
#include "trafficgraph.h"
#include "packetpage.h"
#include "terminal.h"
#include "alertpage.h"
#include "managementpage.h"

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QTimer>
#include <thread>
#include <atomic>
#include <QStackedWidget>

class Window : public QMainWindow{

    Q_OBJECT

public: 
    explicit Window(QWidget *parent = nullptr);
    ~Window();

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void OnMinimizeClicked();
    void OnMaximizeClicked();
    void OnCloseClicked();
    void SwitchPage(int index);

private:

    QWidget *Main_TitleBar;
    QLabel *Main_TitleLogo;
    QLabel *SentinelTitle;
    QPushButton *Main_MinimizeButton;
    QPushButton *Main_MaximizeButton;
    QPushButton *Main_CloseButton;

    void SetupUI();

    QTimer *MainTimer;
    std::thread *MainReceiverThread;
    std::atomic<bool> StopReceiverThread{false};
    void startReceiver();

    QStackedWidget *MainWidget;
    TrafficGraph *DashBoardPage;
    PacketPage *PacketInfoPage;
    Terminal *TerminalPage;
    AlertPage *AlertInfoPage;
    SideBar *MainSideBar;
    ManagementPage *ManagementInfoPage;

};

#endif