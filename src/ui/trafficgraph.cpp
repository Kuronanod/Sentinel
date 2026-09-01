#include "trafficgraph.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QByteArray>
#include <QString>
#include <QDebug>
#include <QPainter>
#include <QTimer>
#include <algorithm>
#include <windows.h>
#include <psapi.h>

TrafficGraph::TrafficGraph(QWidget *parent) : QWidget(parent){

    QVBoxLayout *MainLayout = new QVBoxLayout(this);

    TrafficWidget = new QWidget(this);
    TrafficWidget->setFixedHeight(400);
    QVBoxLayout *TrafficLayout = new QVBoxLayout(TrafficWidget);
    TrafficLayout->setAlignment(Qt::AlignTop);
    MainSparkLine = new SparkLineWidget(this);
    MainSparkLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    TrafficLayout->addWidget(MainSparkLine);

    MainLayout->addWidget(TrafficWidget);

    MainResourceWidget = new ResourceWidget(this);
    MainLayout->addWidget(MainResourceWidget);

    this->setStyleSheet("background-color: white;");

    MainResourceTimer = new QTimer(this);
    connect(MainResourceTimer, &QTimer::timeout, this, [this]() {
        MainResourceWidget->UpdateResources();
    });
    MainResourceTimer->start(1000);

}

void TrafficGraph::UpdatePacketCount(int count){

    if (MainFirst) {
        MainLastCount = count;
        MainFirst = false;
        return;
    }

    int different = count - MainLastCount;
    int rate = different * 10;
    MainLastCount = count;
    MainSparkLine->AddValue(rate);

}

SparkLineWidget::SparkLineWidget(QWidget *parent) : QWidget(parent){

    setMinimumHeight(100);
    MainData.reserve(MaxPoints);

}

void SparkLineWidget::AddValue(int rate){

    MainData.push_back(rate);
    if(MainData.size() > MaxPoints){
        MainData.pop_front();
    }
    update();

}

void SparkLineWidget::Clear(){

    MainData.clear();
    update();

}

void SparkLineWidget::paintEvent(QPaintEvent *){

    QPainter Paint(this);
    Paint.setRenderHint(QPainter::Antialiasing, true);
    Paint.fillRect(rect(), QColor("#1a1a1a"));

    if(MainData.size() < 2){
        return;
    }
    int Width = width();
    int Height = height();
    if(Width <= 0 || Height <= 0){
        return;
    }

    int LookBack = std::min(60, int(MainData.size()));
    int MaxValue = *std::max_element((MainData.end() - LookBack), MainData.end());
    if(MaxValue < 10){
        MaxValue = 10;
    }

    QPen Pen(QColor("#00ffcc"), 2);
    Paint.setPen(Pen);

    double StepPositionX = (double)Width / (MainData.size() - 1);
    QPointF Previous;
    for (int Index = 0; Index < MainData.size(); ++Index) {
        double PositionX = Index * StepPositionX;
        double PositionY = Height - ((double)MainData[Index] / MaxValue) * (Height - 10);
        QPointF pt(PositionX, PositionY);
        if (Index > 0) Paint.drawLine(Previous, pt);
        Previous = pt;
    }

    Paint.setPen(QColor("#aaaaaa"));
    Paint.setFont(QFont("Consolas", 9));
    Paint.drawText(5, 14, QString::number(MaxValue) + " pps");

}

ResourceWidget::ResourceWidget(QWidget *parent) : QWidget(parent) {
    QHBoxLayout *MainLayout = new QHBoxLayout(this);
    MainLayout->setContentsMargins(5,5,5,5);
    MainLayout->setSpacing(10);

    auto BoxSetup = [this, &MainLayout](const QString &title) -> QLabel* {
        
        QWidget *Box = new QWidget(this);
        Box->setStyleSheet("background-color: #2a2a2a; border-radius:5px;");

        QVBoxLayout *VBox = new QVBoxLayout(Box);
        VBox->setContentsMargins(8, 8, 8, 8);

        QLabel *TitleLabel = new QLabel(title, Box);
        TitleLabel->setStyleSheet("color: #888; font-size:11px;");

        QLabel *ValueLabel = new QLabel("...", Box);
        ValueLabel->setStyleSheet("color: white; font-size:16px; font-weight:bold;");

        VBox->addWidget(TitleLabel);
        VBox->addWidget(ValueLabel);
        MainLayout->addWidget(Box, 1);

        return ValueLabel;
    };

    CPULabel  = BoxSetup("CPU");
    RamLabel  = BoxSetup("RAM");
    StorageLabel = BoxSetup("DISK");
    GPULabel  = BoxSetup("GPU");
    GPULabel->setText("N/A");
}

void ResourceWidget::UpdateResources() {
    // --- CPU ---
    static FILETIME prevIdle, prevKernel, prevUser;
    FILETIME idle, kernel, user;
    if (GetSystemTimes(&idle, &kernel, &user)) {
        ULARGE_INTEGER idleDiff, kernelDiff, userDiff;
        idleDiff.QuadPart = idle.dwLowDateTime | ((ULONGLONG)idle.dwHighDateTime << 32);
        kernelDiff.QuadPart = kernel.dwLowDateTime | ((ULONGLONG)kernel.dwHighDateTime << 32);
        userDiff.QuadPart = user.dwLowDateTime | ((ULONGLONG)user.dwHighDateTime << 32);

        if (prevIdle.dwLowDateTime || prevIdle.dwHighDateTime) {
            ULONGLONG idleDelta = idleDiff.QuadPart - (prevIdle.dwLowDateTime | ((ULONGLONG)prevIdle.dwHighDateTime << 32));
            ULONGLONG kernelDelta = kernelDiff.QuadPart - (prevKernel.dwLowDateTime | ((ULONGLONG)prevKernel.dwHighDateTime << 32));
            ULONGLONG userDelta = userDiff.QuadPart - (prevUser.dwLowDateTime | ((ULONGLONG)prevUser.dwHighDateTime << 32));
            ULONGLONG totalDelta = kernelDelta + userDelta;
            double cpuUsage = (totalDelta > 0) ? (1.0 - (double)idleDelta / totalDelta) * 100.0 : 0.0;
            CPULabel->setText(QString::number(cpuUsage, 'f', 1) + " %");
        }
        prevIdle = idle; prevKernel = kernel; prevUser = user;
    }

    // --- RAM ---
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        int usedPercent = memStatus.dwMemoryLoad; // 0-100
        double totalGB = memStatus.ullTotalPhys / (1024.0 * 1024 * 1024);
        double usedGB = totalGB * usedPercent / 100.0;
        RamLabel->setText(QString("%1 / %2 GB (%3%)")
            .arg(usedGB, 0, 'f', 1)
            .arg(totalGB, 0, 'f', 1)
            .arg(usedPercent));
    }

    // --- DISK (C:) ---
    ULARGE_INTEGER freeBytesAvailable, totalBytes, freeBytesTotal;
    if (GetDiskFreeSpaceExW(L"C:\\", &freeBytesAvailable, &totalBytes, &freeBytesTotal)) {
        double totalGB = totalBytes.QuadPart / (1024.0 * 1024 * 1024);
        double freeGB = freeBytesAvailable.QuadPart / (1024.0 * 1024 * 1024);
        double usedGB = totalGB - freeGB;
        int usedPercent = totalGB > 0 ? (int)(usedGB / totalGB * 100) : 0;
        StorageLabel->setText(QString("%1 / %2 GB (%3%)")
            .arg(usedGB, 0, 'f', 1)
            .arg(totalGB, 0, 'f', 1)
            .arg(usedPercent));
    }
}