#include "trafficgraph.h"

#include <QVBoxLayout>

TrafficGraph::TrafficGraph(QWidget *parent)

    : QWidget(parent)

{

    QVBoxLayout *layout =  new QVBoxLayout(this);

    main_label = new QLabel("Traffic Graph",this);
    main_label->setAlignment(Qt::AlignCenter | Qt::AlignTop);
    main_label->setFixedHeight(120);

    this->setStyleSheet("background-color: lightblue;");
    main_label->setStyleSheet("background_color: yellow;");

    layout->addWidget(main_label,Qt::AlignTop);

}