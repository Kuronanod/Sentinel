#include "topbar.h"
#include <QMainWindow>
#include <QMenuBar>
#include <QAction>

void TopBar::setup(QMainWindow *window){

    QMenuBar *menubar = window->menuBar();

    menubar->setStyleSheet("QMenuBar { font-size: 14px; padding: 2px;}" "QMenuBar::item { padding: 4px 12px; }");

    QMenu *fileMenu = menubar->addMenu("&File");
    fileMenu->addAction("Exit", window, &QWidget::close);

}