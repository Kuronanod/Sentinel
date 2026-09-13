#include "window.h"
#include "topbar.h"
#include "receiver/receiver.h"
#include "receiver/packet_counter.h"
#include "receiver/request_queue.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QMenuBar>
#include <QAction>
#include <QToolBar>
#include <QLabel>
#include <QDebug>
#include <QPixmap>
#include <QMouseEvent>
#include <QWindow>
#include <thread>
#include <chrono>
#include <atomic>

// Window API
#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
#endif

Window::Window(QWidget *parent) : QMainWindow(parent) {

    SetupUI();

    setWindowTitle("Sentinel");
    resize(800, 600);

    //TopBar::setup(this);

}

void Window::SetupUI(){

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_NativeWindow);

    HWND hwnd = reinterpret_cast<HWND>(winId());
    LONG_PTR Style = GetWindowLongPtr(hwnd, GWL_STYLE);
    Style |= WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
    SetWindowLongPtr(hwnd, GWL_STYLE, Style);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    Main_TitleBar = new QWidget(this);
    Main_TitleBar->setFixedHeight(40);
    Main_TitleBar->setStyleSheet("background-color: #2c3e50;");

    Main_MinimizeButton = new QPushButton("─");
    Main_MaximizeButton = new QPushButton("☐");
    Main_CloseButton = new QPushButton("✕");

    Main_MinimizeButton->setFixedSize(45, 30);
    Main_MaximizeButton->setFixedSize(45, 30);
    Main_CloseButton->setFixedSize(45, 30);

    Main_MinimizeButton->setStyleSheet("QPushButton { border: none; color: white; background: transparent; } QPushButton:hover { background-color: #34495e; }");
    Main_MaximizeButton->setStyleSheet("QPushButton { border: none; color: white; background: transparent; } QPushButton:hover { background-color: #34495e; }");
    Main_CloseButton->setStyleSheet("QPushButton { border: none; color: white; background: transparent; } QPushButton:hover { background-color: #e74c3c; }");    

    //Title Layout
    QHBoxLayout *TitleLayout = new QHBoxLayout(Main_TitleBar);
    TitleLayout->setContentsMargins(0, 0, 10, 0);

    //Logo
    Main_TitleLogo = new QLabel(this);

    SentinelTitle = new QLabel("Sentinel",this);
    SentinelTitle->setStyleSheet("color: white; font-weight: bold;");

    //Layout Widget Add
    TitleLayout->addWidget(Main_TitleLogo);
    TitleLayout->addWidget(SentinelTitle);
    TitleLayout->addStretch();
    TitleLayout->addWidget(Main_MinimizeButton);
    TitleLayout->addWidget(Main_MaximizeButton);
    TitleLayout->addWidget(Main_CloseButton);

    //Main Layout
    QWidget *CentralWidget = new QWidget(this);
    CentralWidget->setContentsMargins(0, 0, 0, 0);
    CentralWidget->setStyleSheet("background-color: #1a2127");
    QVBoxLayout *MainLayout = new QVBoxLayout(CentralWidget);
    MainLayout->setContentsMargins(0, 0, 0, 0);
    MainLayout->setSpacing(0);
    MainLayout->addWidget(Main_TitleBar);

    //Content
    QWidget *Content = new QWidget(this);
    Content->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *ContentLayout = new QHBoxLayout(Content);
    ContentLayout->setContentsMargins(0, 0, 0, 0);
    ContentLayout->setSpacing(0);

    MainSideBar = new SideBar(this);
    ContentLayout->addWidget(MainSideBar, 0);
    connect(MainSideBar, &SideBar::pageChangeRequested, this, &Window::SwitchPage);

    MainWidget = new QStackedWidget(this);
    DashBoardPage = new TrafficGraph(MainWidget);
    PacketInfoPage = new PacketPage(MainWidget);
    TerminalPage = new Terminal(MainWidget);
    AlertInfoPage = new AlertPage(MainWidget);
    ManagementInfoPage = new ManagementPage(MainWidget);

    MainWidget->addWidget(DashBoardPage);
    MainWidget->addWidget(PacketInfoPage);
    MainWidget->addWidget(TerminalPage);
    MainWidget->addWidget(AlertInfoPage);
    MainWidget->addWidget(ManagementInfoPage);

    ContentLayout->addWidget(MainWidget, 1);
    MainLayout->addWidget(Content, 1);

    setCentralWidget(CentralWidget);

    //Connect Button
    connect(Main_MinimizeButton, &QPushButton::clicked, this, &Window::OnMinimizeClicked);
    connect(Main_MaximizeButton, &QPushButton::clicked, this, &Window::OnMaximizeClicked);
    connect(Main_CloseButton, &QPushButton::clicked, this, &Window::OnCloseClicked);

    MainReceiverThread = new std::thread([this]() {
    while (!StopReceiverThread) {
            receiver("192.168.1.103");  // Ip ตรงนี้นะ bro
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    // Timer อัปเดต UI ทุก 100 ms
    MainTimer = new QTimer(this);
    connect(MainTimer, &QTimer::timeout, this, [this]() {
        int count = GetPacketCount();
        DashBoardPage->UpdatePacketCount(count);

        int queueSize = GetPacketQueueSize();
        qDebug() << "Queue size:" << queueSize;

    });
    MainTimer->start(1000);

}

Window::~Window() {
    StopReceiverThread = true; 
    if (MainReceiverThread && MainReceiverThread->joinable()) {
        MainReceiverThread->join();
    }
    delete MainReceiverThread;
    delete MainTimer;
}

bool Window::nativeEvent(const QByteArray &eventType, void *message, qintptr *result){

    #ifdef Q_OS_WIN

        MSG *msg = static_cast<MSG *>(message);

        if (msg->message == WM_NCCALCSIZE) {
                if (msg->wParam == TRUE) {
                    *result = 0;
                    return true;
                }
                return false;
            }

        if(msg->message == WM_NCHITTEST){
            
            QRect WindowRect = this->frameGeometry();
            int Position_X = GET_X_LPARAM(msg->lParam) - WindowRect.x();
            int Position_Y = GET_Y_LPARAM(msg->lParam) - WindowRect.y();
            int Width = WindowRect.width();
            int Height = WindowRect.height();
            int Border = 8;

            //Skip Is Size = 0
            if(Width <= 0 || Height <= 0){
                return QMainWindow::nativeEvent(eventType, message, result);
            }

            //Section 1
            if(Position_X < Border && Position_Y < Border){
                *result = HTTOPLEFT;
                return true;
            }
            if(Position_X > Width - Border && Position_Y < Border){
                *result = HTTOPRIGHT;
                return true;
            }
            if(Position_X < Border && Position_Y > Height - Border){
                *result = HTBOTTOMLEFT;
                return true;
            }
            if(Position_X > Width - Border && Position_Y > Height - Border){
                *result = HTBOTTOMRIGHT;
                return true;
            }

            //Section 2
            if(Position_X < Border){
                *result = HTLEFT;
                return true;
            }
            if(Position_X > Width - Border){
                *result = HTRIGHT;
                return true;
            }
            if(Position_Y < Border){
                *result = HTTOP;
                return true;
            }
            if(Position_Y > Height - Border){
                *result = HTBOTTOM;
                return true;
            }

            //Section 3
            POINT Position;
            Position.x = GET_X_LPARAM(msg->lParam);
            Position.y = GET_Y_LPARAM(msg->lParam);
            ::ScreenToClient((HWND)winId(), &Position);
            QPoint Pos(Position.x, Position.y);
            QRect TitleRect = Main_TitleBar->geometry();
            TitleRect.moveTopLeft(Main_TitleBar->mapTo(this, QPoint(0,0)));
            if(TitleRect.contains(Pos)){
                QRect MinRect = Main_MinimizeButton->geometry();
                MinRect.moveTopLeft(Main_MinimizeButton->mapTo(this, QPoint(0,0)));
                QRect MaxRect = Main_MaximizeButton->geometry();
                MaxRect.moveTopLeft(Main_MaximizeButton->mapTo(this, QPoint(0,0)));
                QRect CloseRect = Main_CloseButton->geometry();
                CloseRect.moveTopLeft(Main_CloseButton->mapTo(this, QPoint(0,0)));

                if(MinRect.contains(Pos) || MaxRect.contains(Pos) || CloseRect.contains(Pos)){
                    *result = HTCLIENT;
                    return true;
                }

                *result = HTCAPTION;
                return true;

            }

        }

    #endif

    return QMainWindow::nativeEvent(eventType, message, result);

}

void Window::resizeEvent(QResizeEvent *event){

    QMainWindow::resizeEvent(event);

    #ifdef Q_OS_WIN

        if(this->isMaximized()){
            this->setContentsMargins(7, 7, 7, 7);
        }else{
            this->setContentsMargins(0, 0, 0, 0);
        }

    #endif

}

void Window::mousePressEvent(QMouseEvent *event){

    QMainWindow::mousePressEvent(event);

}

void Window::mouseMoveEvent(QMouseEvent *event){

    QMainWindow::mouseMoveEvent(event);

}

void Window::OnMinimizeClicked(){

    this->showMinimized();

}

void Window::OnMaximizeClicked(){

    if(this->isMaximized()){
        this->showNormal();
    }else{
        this->showMaximized();
    }

}

void Window::OnCloseClicked(){

    this->close();

}

void Window::SwitchPage(int index){

    if(MainWidget && index >= 0 && index < MainWidget->count()){
        MainWidget->setCurrentIndex(index);
    }

}
