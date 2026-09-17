#include "terminal.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QDir>
#include <QScrollBar>
#include <QFile>

// ================================================================
//  Palette (VSCode Dark)
// ================================================================
#define COLOR_BG      "#1a1a1a"
#define COLOR_PANEL   "#212121"
#define COLOR_BORDER  "#2d2d2d"
#define COLOR_TEXT    "#e0e0e0"
#define COLOR_DIM     "#707070"
#define COLOR_ACCENT  "#4ec9b0"
#define COLOR_INFO    "#6cb6ff"
#define COLOR_ERROR   "#f48771"
#define COLOR_WARNING "#dcdcaa"

// ================================================================
//  OS-specific constants
// ================================================================
#ifdef Q_OS_WIN
    #define TERMINAL_TITLE    "Sentinel Terminal"
    #define TERMINAL_SHELL    "Shell: cmd.exe"
    #define PROMPT_SYMBOL     ">"
    #define SHELL_NAME        "cmd.exe"
    #define SHELL_LINE_END    "\r\n"
    #define TERMINAL_FONT     "Consolas"
#else
    #define TERMINAL_TITLE    "Sentinel Terminal"
    #define TERMINAL_SHELL    "Shell: /bin/bash"
    #define PROMPT_SYMBOL     "$"
    #define SHELL_NAME        "/bin/bash"
    #define SHELL_LINE_END    "\n"
    #define TERMINAL_FONT     "Ubuntu Mono"
#endif

// ================================================================
//  Constructor
// ================================================================
Terminal::Terminal(QWidget *parent) : QWidget(parent) {

    this->setStyleSheet(
        QString("Terminal { background-color: %1; }").arg(COLOR_BG)
    );

    QVBoxLayout *RootLayout = new QVBoxLayout(this);
    RootLayout->setContentsMargins(0, 0, 0, 0);
    RootLayout->setSpacing(0);

    // ============================================================
    //  Title Bar
    // ============================================================
    QWidget *TitleBar = new QWidget(this);
    TitleBar->setFixedHeight(40);
    TitleBar->setStyleSheet(
        QString("background-color: %1;"
                "border-bottom: 1px solid %2;").arg(COLOR_PANEL, COLOR_BORDER)
    );
    QHBoxLayout *TitleLayout = new QHBoxLayout(TitleBar);
    TitleLayout->setContentsMargins(16, 0, 16, 0);
    TitleLayout->setSpacing(10);

    QLabel *TitleLabel = new QLabel("Terminal", TitleBar);
    TitleLabel->setStyleSheet(
        QString("color: %1; font-size: 14px; font-weight: 600;"
                "background: transparent; border: none;").arg(COLOR_TEXT)
    );
    TitleLayout->addWidget(TitleLabel);
    TitleLayout->addStretch();

    // ---- Status ----
    StatusLabel = new QLabel("● Ready", TitleBar);
    StatusLabel->setStyleSheet(
        QString("color: %1; font-size: 11px;"
                "background: transparent; border: none;").arg(COLOR_ACCENT)
    );
    TitleLayout->addWidget(StatusLabel);

    // ---- Stop Button ----
    StopBtn = new QPushButton("Stop", TitleBar);
    StopBtn->setCursor(Qt::PointingHandCursor);
    StopBtn->setFixedHeight(26);
    StopBtn->setStyleSheet(
        QString("QPushButton {"
                "  background-color: transparent;"
                "  color: %1;"
                "  font-size: 11px;"
                "  border: 1px solid %2;"
                "  border-radius: 4px;"
                "  padding: 0 12px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #3a2020;"
                "  border: 1px solid %1;"
                "  color: %1;"
                "}").arg(COLOR_ERROR, COLOR_BORDER)
    );
    TitleLayout->addWidget(StopBtn);

    // ---- Clear Button ----
    ClearBtn = new QPushButton("Clear", TitleBar);
    ClearBtn->setCursor(Qt::PointingHandCursor);
    ClearBtn->setFixedHeight(26);
    ClearBtn->setStyleSheet(
        QString("QPushButton {"
                "  background-color: transparent;"
                "  color: %1;"
                "  font-size: 11px;"
                "  border: 1px solid %2;"
                "  border-radius: 4px;"
                "  padding: 0 12px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #2a3a35;"
                "  border: 1px solid %3;"
                "  color: %3;"
                "}").arg(COLOR_DIM, COLOR_BORDER, COLOR_ACCENT)
    );
    TitleLayout->addWidget(ClearBtn);

    RootLayout->addWidget(TitleBar);

    // ============================================================
    //  Output View
    // ============================================================
    OutputView = new QTextEdit(this);
    OutputView->setReadOnly(true);
    OutputView->document()->setMaximumBlockCount(2000);
    OutputView->setStyleSheet(
        QString("QTextEdit {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-family: '%3', monospace;"
                "  font-size: 12px;"
                "  border: none;"
                "  padding: 14px 18px;"
                "  selection-background-color: #2a3a35;"
                "  selection-color: %4;"
                "}"
                "QScrollBar:vertical {"
                "  background: %1;"
                "  width: 10px;"
                "  border: none;"
                "}"
                "QScrollBar::handle:vertical {"
                "  background: #3e3e42;"
                "  border-radius: 5px;"
                "  min-height: 40px;"
                "}"
                "QScrollBar::handle:vertical:hover {"
                "  background: %5;"
                "}"
                "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
                "  height: 0px;"
                "}"
                "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
                "  background: none;"
                "}").arg(COLOR_BG, COLOR_TEXT, TERMINAL_FONT,
                         COLOR_ACCENT, COLOR_ACCENT)
    );
    RootLayout->addWidget(OutputView, 1);

    // ============================================================
    //  Input Line
    // ============================================================
    QWidget *InputBar = new QWidget(this);
    InputBar->setFixedHeight(46);
    InputBar->setStyleSheet(
        QString("background-color: %1;"
                "border-top: 1px solid %2;").arg(COLOR_PANEL, COLOR_BORDER)
    );
    QHBoxLayout *InputLayout = new QHBoxLayout(InputBar);
    InputLayout->setContentsMargins(16, 8, 16, 8);
    InputLayout->setSpacing(10);

    QLabel *PromptLabel = new QLabel(PROMPT_SYMBOL, InputBar);
    PromptLabel->setStyleSheet(
        QString("color: %1; font-family: '%2', monospace; font-size: 14px;"
                "font-weight: bold; background: transparent;")
            .arg(COLOR_ACCENT, TERMINAL_FONT)
    );
    InputLayout->addWidget(PromptLabel);

    InputLine = new QLineEdit(InputBar);
    InputLine->setPlaceholderText("Type command and press Enter...");
    InputLine->setStyleSheet(
        QString("QLineEdit {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-family: '%3', monospace;"
                "  font-size: 12px;"
                "  border: 1px solid %4;"
                "  border-radius: 5px;"
                "  padding: 6px 12px;"
                "}"
                "QLineEdit:focus {"
                "  border: 1px solid %5;"
                "  background-color: #1e1e1e;"
                "}").arg(COLOR_BG, COLOR_TEXT, TERMINAL_FONT,
                         COLOR_BORDER, COLOR_ACCENT)
    );
    InputLayout->addWidget(InputLine, 1);

    RootLayout->addWidget(InputBar);

    // ============================================================
    //  Shell Process
    // ============================================================
    Shell = new QProcess(this);
    Shell->setProcessChannelMode(QProcess::MergedChannels);
    Shell->setWorkingDirectory(QDir::currentPath());

    connect(Shell, &QProcess::readyReadStandardOutput,
            this, &Terminal::OnReadyReadStandardOutput);
    connect(Shell, &QProcess::readyReadStandardError,
            this, &Terminal::OnReadyReadStandardError);
    connect(Shell, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &Terminal::OnProcessFinished);

    connect(InputLine, &QLineEdit::returnPressed,
            this, &Terminal::OnCommandEntered);
    connect(ClearBtn, &QPushButton::clicked,
            this, &Terminal::OnClearClicked);
    connect(StopBtn, &QPushButton::clicked,
            this, &Terminal::OnStopClicked);

    // ============================================================
    //  Welcome Header
    // ============================================================
    AppendText("", "");
    AppendText("  ╔══════════════════════════════════════════════╗", COLOR_ACCENT);
    AppendText("  ║                                              ║", COLOR_ACCENT);
    AppendText("  ║            S E N T I N E L                   ║", COLOR_ACCENT);
    AppendText("  ║              T E R M I N A L                 ║", COLOR_ACCENT);
    AppendText("  ║                                              ║", COLOR_ACCENT);
    AppendText("  ╚══════════════════════════════════════════════╝", COLOR_ACCENT);
    AppendText("", "");

    AppendText(QString("  %1").arg(TERMINAL_SHELL), COLOR_DIM);
    AppendText(QString("  Working Directory: %1").arg(QDir::currentPath()), COLOR_DIM);
    AppendText("", "");

    AppendText("  Tip: Type commands and press Enter", COLOR_INFO);
    AppendText("  Press [Stop] to interrupt a running command", COLOR_INFO);
    AppendText("", "");
    AppendText("  ─────────────────────────────────────────────", COLOR_BORDER);
    AppendText("", "");

    StartShell();
}

Terminal::~Terminal() {
    StopShell();
}

// ================================================================
//  StartShell
// ================================================================
void Terminal::StartShell() {

#ifdef Q_OS_WIN
    Shell->start(SHELL_NAME);
#else
    QString shellPath = "/bin/bash";
    if (!QFile::exists(shellPath)) {
        shellPath = "/bin/sh";
    }
    Shell->start(shellPath);
#endif

    if (!Shell->waitForStarted(3000)) {
        AppendText("  Failed to start shell", COLOR_ERROR);
        StatusLabel->setText("● Failed");
        StatusLabel->setStyleSheet(
            QString("color: %1; font-size: 11px;"
                    "background: transparent; border: none;").arg(COLOR_ERROR)
        );
        return;
    }

    StatusLabel->setText("● Connected");
    StatusLabel->setStyleSheet(
        QString("color: %1; font-size: 11px;"
                "background: transparent; border: none;").arg(COLOR_ACCENT)
    );
}

// ================================================================
//  StopShell
// ================================================================
void Terminal::StopShell() {
    if (Shell && Shell->state() != QProcess::NotRunning) {
        Shell->write("exit\n");
        Shell->waitForBytesWritten(500);
        Shell->closeWriteChannel();
        Shell->waitForFinished(1000);

        if (Shell->state() != QProcess::NotRunning) {
            Shell->kill();
            Shell->waitForFinished(500);
        }
    }
}

// ================================================================
//  OnCommandEntered
// ================================================================
void Terminal::OnCommandEntered() {
    QString cmd = InputLine->text().trimmed();
    if (cmd.isEmpty()) return;

    AppendText(QString("  %1 %2").arg(PROMPT_SYMBOL).arg(cmd), COLOR_ACCENT);
    AppendText("", "");

    if (Shell->state() == QProcess::Running) {
        Shell->write(cmd.toUtf8());
        Shell->write(SHELL_LINE_END);
        Shell->waitForBytesWritten(500);
    } else {
        AppendText("  Shell not running. Restarting...", COLOR_ERROR);
        StartShell();
    }

    InputLine->clear();
}

// ================================================================
//  Read Output
// ================================================================
void Terminal::OnReadyReadStandardOutput() {
    QByteArray data = Shell->readAllStandardOutput();
    if (data.isEmpty()) return;

    QString text;

#ifdef Q_OS_WIN
    text = QString::fromLocal8Bit(data);
#else
    text = QString::fromUtf8(data);
#endif

    text.replace("\r\n", "\n");
    text.replace("\r", "\n");

    AppendText(text, COLOR_TEXT);
}

void Terminal::OnReadyReadStandardError() {
    QByteArray data = Shell->readAllStandardError();
    if (data.isEmpty()) return;

    QString text;

#ifdef Q_OS_WIN
    text = QString::fromLocal8Bit(data);
#else
    text = QString::fromUtf8(data);
#endif

    text.replace("\r\n", "\n");
    text.replace("\r", "\n");

    AppendText(text, COLOR_ERROR);
}

// ================================================================
//  Process Finished
// ================================================================
void Terminal::OnProcessFinished(int exitCode, QProcess::ExitStatus) {
    AppendText("", "");
    AppendText(QString("  Shell exited with code %1").arg(exitCode), COLOR_WARNING);
    AppendText("", "");

    StatusLabel->setText("● Disconnected");
    StatusLabel->setStyleSheet(
        QString("color: %1; font-size: 11px;"
                "background: transparent; border: none;").arg(COLOR_ERROR)
    );
}

// ================================================================
//  OnStopClicked — ยกเลิกคำสั่งที่ค้างอยู่
// ================================================================
void Terminal::OnStopClicked() {
    AppendText("", "");
    AppendText("  ⏹ Interrupting...", COLOR_WARNING);

#ifdef Q_OS_WIN
    // Windows: taskkill คำสั่งที่ค้างบ่อย
    const QStringList hangingCommands = {
        "ping.exe",
        "tracert.exe",
        "nslookup.exe",
        "netstat.exe",
        "telnet.exe"
    };

    for (const QString &cmd : hangingCommands) {
        QProcess killer;
        killer.start("taskkill", QStringList() << "/F" << "/IM" << cmd);
        killer.waitForFinished(500);
    }

    // Restart shell (to clear state)
    StopShell();
    StartShell();

#else
    // Linux: ส่ง SIGINT
    qint64 pid = Shell->processId();
    if (pid > 0) {
        QProcess::execute("kill", QStringList()
            << "-INT" << "-" + QString::number(pid));
    }

    // ถ้ายังไม่ตาย → restart
    QThread::msleep(200);
    if (Shell->state() != QProcess::NotRunning) {
        StopShell();
        StartShell();
    }
#endif

    AppendText("  ✓ Command interrupted", COLOR_ACCENT);
    AppendText("", "");
}

// ================================================================
//  Append Text
// ================================================================
void Terminal::AppendText(const QString &text, const QString &color) {
    QString safe = text.toHtmlEscaped();
    safe.replace(" ", "&nbsp;");

    QString html;
    if (color.isEmpty()) {
        html = QString(
            "<pre style='margin:0; "
            "white-space:pre-wrap; "
            "font-family:\"%1\", monospace; "
            "line-height:1.4;'>%2</pre>")
            .arg(TERMINAL_FONT, safe);
    } else {
        html = QString(
            "<pre style='margin:0; "
            "white-space:pre-wrap; "
            "color:%1; "
            "font-family:\"%2\", monospace; "
            "line-height:1.4;'>%3</pre>")
            .arg(color, TERMINAL_FONT, safe);
    }

    OutputView->append(html);

    QScrollBar *bar = OutputView->verticalScrollBar();
    bar->setValue(bar->maximum());
}

// ================================================================
//  Clear
// ================================================================
void Terminal::OnClearClicked() {
    OutputView->clear();

    AppendText("", "");
    AppendText("  ╔══════════════════════════════════════════════╗", COLOR_ACCENT);
    AppendText("  ║            S E N T I N E L                   ║", COLOR_ACCENT);
    AppendText("  ║              T E R M I N A L                 ║", COLOR_ACCENT);
    AppendText("  ╚══════════════════════════════════════════════╝", COLOR_ACCENT);
    AppendText("", "");
}