#ifndef TRAFFICGRAPH_H
#define TRAFFICGRAPH_H

#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QVector>
#include <QTimer>
#include <algorithm>

class SparkLineWidget : public QWidget{

    Q_OBJECT

public:
    explicit SparkLineWidget(QWidget *parent = nullptr);
    void AddValue(int rate);
    void Clear();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<int> MainData;
    static const int MaxPoints = 120;

};

class ResourceWidget : public QWidget {

    Q_OBJECT

public:
    explicit ResourceWidget(QWidget *parent = nullptr);
    void UpdateResources();

private:
    QLabel *CPULabel;
    QLabel *RamLabel;
    QLabel *StorageLabel;
    QLabel *GPULabel;
};

class TrafficGraph : public QWidget{

    Q_OBJECT

public:
    explicit TrafficGraph(QWidget *parent = nullptr);

public slots:
    void UpdatePacketCount(int count);

private:
    QWidget *TrafficWidget;
    SparkLineWidget *MainSparkLine;
    ResourceWidget *MainResourceWidget;
    QTimer *MainResourceTimer;

    int MainLastCount = 0;
    bool MainFirst = true;

};

#endif