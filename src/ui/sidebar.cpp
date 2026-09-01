#include "sidebar.h"

#include <QVBoxLayout>
#include <QLabel>

SideBar::SideBar(QWidget *parent) : QWidget(parent){

    
    setFixedWidth(100);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setStyleSheet("background-color: #2E2B2A;");
    
    QVBoxLayout *Main_Layout = new QVBoxLayout(this);
    Main_Layout->setContentsMargins(0, 0, 0, 0);
    Main_Layout->setSpacing(0);

    // Trafficgraph
    DashBoardButton = new QPushButton("⧉");
    connect(DashBoardButton, &QPushButton::clicked, this, [this](){ emit pageChangeRequested(0); });
    DashBoardButton->setStyleSheet("color: white; font-size: 20px; padding: 35px; font-weight: bold;");
    Main_Layout->addWidget(DashBoardButton);

    // Terminal
    TerminalButton = new QPushButton(">_");
    TerminalButton->setStyleSheet("color: white; font-size: 20px; padding: 35px; font-weight: bold;");
    Main_Layout->addWidget(TerminalButton);

    // Management
    ManagementButton = new QPushButton("⊞");
    ManagementButton->setStyleSheet("color: white; font-size: 40px; padding: 35px; font-weight: bold;");
    Main_Layout->addWidget(ManagementButton);

    // Log Information
    LogInformationButton = new QPushButton("▥");
    LogInformationButton->setStyleSheet("color: white; font-size: 40px; padding: 35px; font-weight: bold;");
    Main_Layout->addWidget(LogInformationButton);

    // Setting Button
    SettingButton = new QPushButton("⚙");
    SettingButton->setStyleSheet("color: white; font-size: 20px; padding: 35px; font-weight: bold;");
    Main_Layout->addWidget(SettingButton);

    QFrame *background = new QFrame(this);
    background->setStyleSheet("background-color: #2E2B2A;");
    background->setFrameShape(QFrame::NoFrame);
    Main_Layout->addWidget(background, 1);

    Main_Layout->addStretch();

}