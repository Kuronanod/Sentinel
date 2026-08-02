#ifndef WINDOW_H
#define WINDOW_H

#include "trafficgraph.h"

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QWidget>

class Window : public QMainWindow{

    Q_OBJECT

public: 
    explicit Window(QWidget *parent = nullptr);

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void OnMinimizeClicked();
    void OnMaximizeClicked();
    void OnCloseClicked();

private:

    QWidget *Main_TitleBar;
    QLabel *Main_TitleLogo;
    QPushButton *Main_MinimizeButton;
    QPushButton *Main_MaximizeButton;
    QPushButton *Main_CloseButton;

    void SetupUI();

    TrafficGraph *mainGraph;

};

#endif