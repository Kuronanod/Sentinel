#ifndef TRAFFICGRAPH_H
#define TRAFFICGRAPH_H

#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QVector>
#include <QTimer>
#include <algorithm>

// ================================================================
//  SparkLineWidget — กราฟเส้น Minimal (VSCode style)
// ================================================================
class SparkLineWidget : public QWidget {
    Q_OBJECT

public:
    explicit SparkLineWidget(QWidget *parent = nullptr);
    void AddValue(int InboundRate, int OutboundRate);
    void Clear();
    int  GetPeakValue() const;
    int GetPeakIn()  const { return PeakIn; }
    int GetPeakOut() const { return PeakOut; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
     QVector<int> InData;
    QVector<int> OutData;
    int PeakIn  = 0;
    int PeakOut = 0;
    int StickyMax = 10;
    int MaxPoints = 60;
};

// ================================================================
//  ResourceBar — แถวเดียวแสดง Resource (CPU / RAM / DISK / GPU)
// ================================================================
class ResourceBar : public QWidget {
    Q_OBJECT

public:
    explicit ResourceBar(const QString &Label, QWidget *parent = nullptr);

    void SetPercent(int percent);
    void SetSubText(const QString &text);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString LabelText;
    QString SubText;
    int     Percent = 0;
};

// ================================================================
//  ResourceWidget — รวม 4 แถว
// ================================================================
class ResourceWidget : public QWidget {
    Q_OBJECT

public:
    explicit ResourceWidget(QWidget *parent = nullptr);
    void UpdateResources();

private:
    ResourceBar *CPUBar;
    ResourceBar *RAMBar;
    ResourceBar *DiskBar;
    ResourceBar *GPUBar;
};

// ================================================================
//  TrafficGraph — Dashboard Minimal
// ================================================================
class TrafficGraph : public QWidget {
    Q_OBJECT

public:
    explicit TrafficGraph(QWidget *parent = nullptr);

public slots:
    void UpdatePacketCount(int InboundCount, int OutboundCount);

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
    int PeakRate    = 0;
};

#endif