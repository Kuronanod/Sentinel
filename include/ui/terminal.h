#ifndef TERMINAL_H
#define TERMINAL_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QProcess>
#include <QVBoxLayout>

class Terminal : public QWidget{

    Q_OBJECT

public:
    explicit Terminal(QWidget *parent = nullptr); 

private slots:
    void ExecuteCommand();
    void ReadOutput();
    void ReadError();

private:
    QTextEdit *OutputArea;
    QLineEdit *InputArea;
    QProcess *Process;

};

#endif