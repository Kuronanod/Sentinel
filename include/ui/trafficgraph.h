#ifndef TRAFFICGRAPH_H
#define TRAFFICGRAPH_H

#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QVector>
#include <QTimer>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <algorithm>

// ================================================================
//  CoreBar — แสดง CPU แต่ละ core
// ================================================================
class CoreBar : public QWidget {
    Q_OBJECT
public:
    explicit CoreBar(int coreIndex, QWidget *parent = nullptr);
    void SetPercent(int percent);
protected:
    void paintEvent(QPaintEvent *) override;
private:
    int CoreIndex;
    int Percent = 0;
};

// ================================================================
//  CPUWidget — รวม CoreBar ทั้งหมด + ScrollArea
// ================================================================
class CPUWidget : public QWidget {
    Q_OBJECT
public:
    explicit CPUWidget(QWidget *parent = nullptr);
    void UpdateCPU();
private:
    QVector<CoreBar*> CoreBars;
    QScrollArea      *ScrollArea;
    QWidget          *BarsContainer;
    int CoreCount = 0;
};

// ================================================================
//  ResourceBar — RAM, DISK, GPU, NET (เดิม)
// ================================================================
class ResourceBar : public QWidget {
    Q_OBJECT
public:
    explicit ResourceBar(const QString &Label, QWidget *parent = nullptr);
    void SetPercent(int percent);
    void SetSubText(const QString &text);
protected:
    void paintEvent(QPaintEvent *) override;
private:
    QString LabelText;
    QString SubText;
    int     Percent = 0;
};

// ================================================================
//  ResourceWidget — 2 columns (CPU | RAM/DISK/GPU/NET)
// ================================================================
class ResourceWidget : public QWidget {
    Q_OBJECT
public:
    explicit ResourceWidget(QWidget *parent = nullptr);
    void UpdateResources();
private:
    CPUWidget   *CPUColumn;
    ResourceBar *RAMBar;
    ResourceBar *DiskBar;
    ResourceBar *GPUBar;
    ResourceBar *NETBar;
};

// ================================================================
//  SparkLineWidget (เดิม)
// ================================================================
class SparkLineWidget : public QWidget {
    Q_OBJECT
public:
    explicit SparkLineWidget(QWidget *parent = nullptr);
    void AddValue(int inRate, int outRate);
    void Clear();
    int GetPeakIn()  const { return PeakIn; }
    int GetPeakOut() const { return PeakOut; }
protected:
    void paintEvent(QPaintEvent *) override;
private:
    QVector<int> InData;
    QVector<int> OutData;
    int PeakIn  = 0;
    int PeakOut = 0;
    int StickyMax = 10;
    static const int MaxPoints = 120;
};

// ================================================================
//  TrafficGraph (Dashboard)
// ================================================================
class TrafficGraph : public QWidget {
    Q_OBJECT
public:
    explicit TrafficGraph(QWidget *parent = nullptr);
public slots:
    void UpdatePacketCount(int inCount, int outCount);
private:
    SparkLineWidget *MainSparkLine;
    ResourceWidget  *MainResourceWidget;
    QTimer          *MainResourceTimer;

    QLabel *InRateLabel;
    QLabel *InPeakLabel;
    QLabel *OutRateLabel;
    QLabel *OutPeakLabel;

    int MainLastIn  = 0;
    int MainLastOut = 0;
    int MainFirst   = 1;
};

#endif