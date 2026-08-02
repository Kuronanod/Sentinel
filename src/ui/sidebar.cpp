#include "sidebar.h"

#include <QVBoxLayout>


SideBar::SideBar(QWidget *parent) 

    : QWidget(parent)

{

    QVBoxLayout *Main_Layout = new QVBoxLayout(this);

    this->setStyleSheet("background-color: black;");

}