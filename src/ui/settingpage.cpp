#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDebug>

#include "settingpage.h"

// ================================================================
//  Palette (VSCode Dark)
// ================================================================
#define COLOR_BG      "#1a1a1a"
#define COLOR_PANEL   "#212121"
#define COLOR_BORDER  "#2d2d2d"
#define COLOR_TEXT    "#e0e0e0"
#define COLOR_DIM     "#707070"
#define COLOR_ACCENT  "#4ec9b0"
#define COLOR_WARNING "#dcdcaa"
#define COLOR_ERROR   "#f48771"
#define COLOR_INFO    "#6cb6ff"

// ================================================================
//  Helper: สร้าง Section (Title + Line)
// ================================================================
QWidget* SettingPage::MakeSection(const QString &title) {
    QWidget *Section = new QWidget();
    QVBoxLayout *Layout = new QVBoxLayout(Section);
    Layout->setContentsMargins(0, 0, 0, 0);
    Layout->setSpacing(6);

    QLabel *TitleLabel = new QLabel(title, Section);
    TitleLabel->setStyleSheet(
        QString("color: %1; font-size: 12px; font-weight: 600;"
                "background: transparent;").arg(COLOR_TEXT)
    );
    Layout->addWidget(TitleLabel);

    QFrame *Line = new QFrame(Section);
    Line->setFixedHeight(1);
    Line->setStyleSheet(QString("background-color: %1;").arg(COLOR_BORDER));
    Layout->addWidget(Line);

    return Section;
}

// ================================================================
//  Constructor
// ================================================================
SettingPage::SettingPage(QWidget *parent) : QWidget(parent) {

    this->setStyleSheet(
        QString("SettingPage { background-color: %1; }").arg(COLOR_BG)
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

    QLabel *TitleLabel = new QLabel("⚙  Settings", TitleBar);
    TitleLabel->setStyleSheet(
        QString("color: %1; font-size: 14px; font-weight: 600;"
                "background: transparent; border: none;").arg(COLOR_TEXT)
    );
    TitleLayout->addWidget(TitleLabel);
    TitleLayout->addStretch();

    RootLayout->addWidget(TitleBar);

    // ============================================================
    //  Scroll Area
    // ============================================================
    QScrollArea *Scroll = new QScrollArea(this);
    Scroll->setWidgetResizable(true);
    Scroll->setFrameShape(QFrame::NoFrame);
    Scroll->setStyleSheet(
        QString("QScrollArea { background: %1; border: none; }"
                "QScrollBar:vertical {"
                "  background: %1; width: 8px; border: none;"
                "}"
                "QScrollBar::handle:vertical {"
                "  background: #3e3e42; border-radius: 4px; min-height: 30px;"
                "}"
                "QScrollBar::handle:vertical:hover { background: #505054; }"
                "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
                "  height: 0px;"
                "}").arg(COLOR_BG)
    );

    QWidget *Content = new QWidget();
    Content->setStyleSheet(QString("background-color: %1;").arg(COLOR_BG));
    QVBoxLayout *ContentLayout = new QVBoxLayout(Content);
    ContentLayout->setContentsMargins(24, 20, 24, 20);
    ContentLayout->setSpacing(22);

    // ============================================================
    //  SECTION: Network
    // ============================================================
    QWidget *NetworkSection = MakeSection("Network");
    QVBoxLayout *NetLayout =
        qobject_cast<QVBoxLayout*>(NetworkSection->layout());
    NetLayout->addSpacing(8);

    // ---- Interface ----
    QHBoxLayout *IfRow = new QHBoxLayout();
    IfRow->setSpacing(12);

    QLabel *IfLabel = new QLabel("Interface", NetworkSection);
    IfLabel->setFixedWidth(160);
    IfLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; background: transparent;")
            .arg(COLOR_DIM)
    );
    IfRow->addWidget(IfLabel);

    InterfaceCombo = new QComboBox(NetworkSection);
    InterfaceCombo->addItem("eth0 (auto-detect)");
    InterfaceCombo->addItem("wlan0 (auto-detect)");
    InterfaceCombo->setFixedWidth(220);
    InterfaceCombo->setStyleSheet(
        QString("QComboBox {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-family: Consolas;"
                "  font-size: 11px;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 5px 10px;"
                "}"
                "QComboBox:focus { border: 1px solid %4; }"
                "QComboBox::drop-down { border: none; width: 20px; }"
                "QComboBox QAbstractItemView {"
                "  background-color: %1;"
                "  color: %2;"
                "  border: 1px solid %3;"
                "  selection-background-color: #2a3a35;"
                "  selection-color: %4;"
                "}").arg(COLOR_PANEL, COLOR_TEXT, COLOR_BORDER, COLOR_ACCENT)
    );
    IfRow->addWidget(InterfaceCombo);
    IfRow->addStretch();

    NetLayout->addLayout(IfRow);

    // ---- Refresh Rate ----
    QHBoxLayout *RefreshRow = new QHBoxLayout();
    RefreshRow->setSpacing(12);

    QLabel *RefreshLabel = new QLabel("Refresh Rate", NetworkSection);
    RefreshLabel->setFixedWidth(160);
    RefreshLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; background: transparent;")
            .arg(COLOR_DIM)
    );
    RefreshRow->addWidget(RefreshLabel);

    RefreshRateInput = new QSpinBox(NetworkSection);
    RefreshRateInput->setRange(100, 2000);
    RefreshRateInput->setValue(200);
    RefreshRateInput->setSuffix(" ms");
    RefreshRateInput->setFixedWidth(120);
    RefreshRateInput->setStyleSheet(
        QString("QSpinBox {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-family: Consolas;"
                "  font-size: 11px;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 5px 8px;"
                "}"
                "QSpinBox:focus { border: 1px solid %4; }")
            .arg(COLOR_PANEL, COLOR_TEXT, COLOR_BORDER, COLOR_ACCENT)
    );
    RefreshRow->addWidget(RefreshRateInput);
    RefreshRow->addStretch();

    NetLayout->addLayout(RefreshRow);

    ContentLayout->addWidget(NetworkSection);

    // ============================================================
    //  SECTION: Alert Preferences
    // ============================================================
    QWidget *AlertSection = MakeSection("Alert Preferences");
    QVBoxLayout *AlertLayout =
        qobject_cast<QVBoxLayout*>(AlertSection->layout());
    AlertLayout->addSpacing(8);

    auto MakeCheckBox = [](const QString &text, bool checked, QWidget *parent) -> QCheckBox* {
        QCheckBox *cb = new QCheckBox(text, parent);
        cb->setChecked(checked);
        cb->setCursor(Qt::PointingHandCursor);
        cb->setStyleSheet(
            QString("QCheckBox {"
                    "  color: %1; font-size: 11px;"
                    "  background: transparent; spacing: 8px;"
                    "}"
                    "QCheckBox::indicator {"
                    "  width: 14px; height: 14px;"
                    "  border: 1px solid %2;"
                    "  border-radius: 3px;"
                    "  background: %3;"
                    "}"
                    "QCheckBox::indicator:checked {"
                    "  background-color: %4;"
                    "  border: 1px solid %4;"
                    "}")
                .arg(COLOR_TEXT, COLOR_BORDER, COLOR_PANEL, COLOR_ACCENT)
        );
        return cb;
    };

    SoundCheck  = MakeCheckBox("Sound alerts for critical events", true, AlertSection);
    NotifyCheck = MakeCheckBox("Desktop notifications",            true, AlertSection);

    AlertLayout->addWidget(SoundCheck);
    AlertLayout->addWidget(NotifyCheck);

    // ---- Max Alerts ----
    QHBoxLayout *MaxRow = new QHBoxLayout();
    MaxRow->setSpacing(12);

    QLabel *MaxLabel = new QLabel("Max alerts stored", AlertSection);
    MaxLabel->setFixedWidth(160);
    MaxLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; background: transparent;")
            .arg(COLOR_DIM)
    );
    MaxRow->addWidget(MaxLabel);

    MaxAlertsInput = new QSpinBox(AlertSection);
    MaxAlertsInput->setRange(100, 10000);
    MaxAlertsInput->setValue(500);
    MaxAlertsInput->setFixedWidth(120);
    MaxAlertsInput->setStyleSheet(
        QString("QSpinBox {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-family: Consolas;"
                "  font-size: 11px;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 5px 8px;"
                "}"
                "QSpinBox:focus { border: 1px solid %4; }")
            .arg(COLOR_PANEL, COLOR_TEXT, COLOR_BORDER, COLOR_ACCENT)
    );
    MaxRow->addWidget(MaxAlertsInput);
    MaxRow->addStretch();

    AlertLayout->addLayout(MaxRow);

    ContentLayout->addWidget(AlertSection);

    // ============================================================
    //  SECTION: Logging
    // ============================================================
    QWidget *LogSection = MakeSection("Logging");
    QVBoxLayout *LogLayout =
        qobject_cast<QVBoxLayout*>(LogSection->layout());
    LogLayout->addSpacing(8);

    LogToFileCheck = MakeCheckBox("Enable log to file", false, LogSection);
    LogLayout->addWidget(LogToFileCheck);

    // ---- Log Path ----
    QHBoxLayout *PathRow = new QHBoxLayout();
    PathRow->setSpacing(8);

    QLabel *PathLabel = new QLabel("Log path", LogSection);
    PathLabel->setFixedWidth(80);
    PathLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; background: transparent;")
            .arg(COLOR_DIM)
    );
    PathRow->addWidget(PathLabel);

    LogPathInput = new QLineEdit(LogSection);
    LogPathInput->setText(
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
        + "/Sentinel/logs"
    );
    LogPathInput->setReadOnly(true);
    LogPathInput->setStyleSheet(
        QString("QLineEdit {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-family: Consolas;"
                "  font-size: 10px;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 5px 10px;"
                "}").arg(COLOR_PANEL, COLOR_DIM, COLOR_BORDER)
    );
    PathRow->addWidget(LogPathInput, 1);

    BrowseBtn = new QPushButton("Browse...", LogSection);
    BrowseBtn->setCursor(Qt::PointingHandCursor);
    BrowseBtn->setFixedHeight(28);
    BrowseBtn->setStyleSheet(
        QString("QPushButton {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-size: 11px;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 0 16px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #2a3a35;"
                "  border: 1px solid %4;"
                "  color: %4;"
                "}").arg(COLOR_PANEL, COLOR_TEXT, COLOR_BORDER, COLOR_ACCENT)
    );
    PathRow->addWidget(BrowseBtn);

    LogLayout->addLayout(PathRow);

    // ---- Export / Import ----
    QHBoxLayout *ExportRow = new QHBoxLayout();
    ExportRow->setSpacing(8);

    QPushButton *ExportBtn = new QPushButton("Export Data", LogSection);
    ExportBtn->setCursor(Qt::PointingHandCursor);
    ExportBtn->setFixedHeight(30);
    ExportBtn->setStyleSheet(
        QString("QPushButton {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-size: 11px;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 0 16px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #2a3a35;"
                "  border: 1px solid %4;"
                "  color: %4;"
                "}").arg(COLOR_PANEL, COLOR_TEXT, COLOR_BORDER, COLOR_ACCENT)
    );
    ExportRow->addWidget(ExportBtn);

    QPushButton *ImportBtn = new QPushButton("Import Settings", LogSection);
    ImportBtn->setCursor(Qt::PointingHandCursor);
    ImportBtn->setFixedHeight(30);
    ImportBtn->setStyleSheet(
        QString("QPushButton {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-size: 11px;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 0 16px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #2a3a35;"
                "  border: 1px solid %4;"
                "  color: %4;"
                "}").arg(COLOR_PANEL, COLOR_TEXT, COLOR_BORDER, COLOR_ACCENT)
    );
    ExportRow->addWidget(ImportBtn);

    ExportRow->addStretch();
    LogLayout->addLayout(ExportRow);

    ContentLayout->addWidget(LogSection);

    // ============================================================
    //  SECTION: About
    // ============================================================
    QWidget *AboutSection = MakeSection("About");
    QVBoxLayout *AboutLayout =
        qobject_cast<QVBoxLayout*>(AboutSection->layout());
    AboutLayout->addSpacing(8);

    QLabel *AppName = new QLabel("Sentinel v1.0.0", AboutSection);
    AppName->setStyleSheet(
        QString("color: %1; font-size: 12px; font-weight: 600;"
                "background: transparent;").arg(COLOR_ACCENT)
    );
    AboutLayout->addWidget(AppName);

    QLabel *AppDesc = new QLabel(
        "Adaptive Network Anomaly Detection System\n"
        "Rule-based IDS with Firewall Integration",
        AboutSection);
    AppDesc->setStyleSheet(
        QString("color: %1; font-size: 11px; line-height: 1.5;"
                "background: transparent;").arg(COLOR_DIM)
    );
    AboutLayout->addWidget(AppDesc);

    ContentLayout->addWidget(AboutSection);

    // ============================================================
    //  Action Buttons
    // ============================================================
    ContentLayout->addSpacing(10);

    QHBoxLayout *ActionRow = new QHBoxLayout();
    ActionRow->setSpacing(10);

    QPushButton *SaveBtn = new QPushButton("Save Settings", Content);
    SaveBtn->setCursor(Qt::PointingHandCursor);
    SaveBtn->setFixedHeight(36);
    SaveBtn->setFixedWidth(180);
    SaveBtn->setStyleSheet(
        QString("QPushButton {"
                "  background-color: #2a3a35;"
                "  color: %1;"
                "  font-size: 12px;"
                "  font-weight: 600;"
                "  border: 1px solid %1;"
                "  border-radius: 4px;"
                "}"
                "QPushButton:hover { background-color: #3a4a45; }")
            .arg(COLOR_ACCENT)
    );
    ActionRow->addWidget(SaveBtn);

    QPushButton *ResetBtn = new QPushButton("⟲  Reset to Default", Content);
    ResetBtn->setCursor(Qt::PointingHandCursor);
    ResetBtn->setFixedHeight(36);
    ResetBtn->setFixedWidth(180);
    ResetBtn->setStyleSheet(
        QString("QPushButton {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-size: 12px;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #3a2020;"
                "  border: 1px solid %4;"
                "  color: %4;"
                "}").arg(COLOR_PANEL, COLOR_TEXT, COLOR_BORDER, COLOR_ERROR)
    );
    ActionRow->addWidget(ResetBtn);

    ActionRow->addStretch();
    ContentLayout->addLayout(ActionRow);

    ContentLayout->addStretch();

    Scroll->setWidget(Content);
    RootLayout->addWidget(Scroll, 1);

    // ============================================================
    //  Connect
    // ============================================================
    connect(BrowseBtn, &QPushButton::clicked, this, &SettingPage::OnBrowseLogPath);
    connect(SaveBtn,   &QPushButton::clicked, this, &SettingPage::OnSaveSettings);
    connect(ResetBtn,  &QPushButton::clicked, this, &SettingPage::OnResetSettings);

    connect(ExportBtn, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "Export",
            "Export Data — feature will be available soon");
    });

    connect(ImportBtn, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "Import",
            "Import Settings — feature will be available soon");
    });

    // ---- Enable/Disable path when Logging toggled ----
    connect(LogToFileCheck, &QCheckBox::toggled, this, [this](bool checked) {
        LogPathInput->setEnabled(checked);
        BrowseBtn->setEnabled(checked);
    });

    LogPathInput->setEnabled(false);
    BrowseBtn->setEnabled(false);
}

SettingPage::~SettingPage() {}

// ================================================================
//  Browse Log Path
// ================================================================
void SettingPage::OnBrowseLogPath() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Select Log Directory",
        LogPathInput->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!dir.isEmpty()) {
        LogPathInput->setText(dir);
    }
}

// ================================================================
//  Save Settings
// ================================================================
void SettingPage::OnSaveSettings() {
    QMessageBox::information(this, "Saved",
        "Settings saved successfully!");
}

// ================================================================
//  Reset Settings
// ================================================================
void SettingPage::OnResetSettings() {
    auto reply = QMessageBox::question(this, "Reset",
        "Reset all settings to default?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    InterfaceCombo->setCurrentIndex(0);
    RefreshRateInput->setValue(200);
    SoundCheck->setChecked(true);
    NotifyCheck->setChecked(true);
    MaxAlertsInput->setValue(500);
    LogToFileCheck->setChecked(false);

    QMessageBox::information(this, "Reset",
        "Settings reset to default values");
}