#ifndef TRAFFICGRAPH_H
#define TRAFFICGRAPH_H

#include <QWidget>
#include <QLabel>

class TrafficGraph : public QWidget{

    Q_OBJECT

public:
    explicit TrafficGraph(QWidget *parent = nullptr);

private:
    QLabel *main_label;

};

#endif