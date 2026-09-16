#ifndef TERMINAL_H
#define TERMINAL_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QProcess>
#include <QPushButton>
#include <QLabel>

class Terminal : public QWidget {
    Q_OBJECT

public:
    explicit Terminal(QWidget *parent = nullptr);
    ~Terminal();

private slots:
    void OnCommandEntered();
    void OnReadyReadStandardOutput();
    void OnReadyReadStandardError();
    void OnProcessFinished(int exitCode, QProcess::ExitStatus status);
    void OnClearClicked();
    void OnStopClicked();

private:
    QTextEdit   *OutputView;
    QLineEdit   *InputLine;
    QPushButton *ClearBtn;
    QPushButton *StopBtn;
    QLabel      *StatusLabel;
    QProcess    *Shell;

    void AppendText(const QString &text, const QString &color = "");
    void StartShell();
    void StopShell();
};

#endif