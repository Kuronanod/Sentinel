#include "trafficgraph.h"
#include "receiver/packet_counter.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <windows.h>
#include <psapi.h>

// ================================================================
//  Palette (VSCode Dark)
// ================================================================
#define COLOR_BG "#1a1a1a"     // พื้นหลังหลัก — เทาเข้ม
#define COLOR_PANEL "#212121"     // Panel
#define COLOR_BORDER "#2d2d2d"     // เส้นกรอบ
#define COLOR_TEXT "#e0e0e0"     // ตัวอักษรขาว
#define COLOR_DIM "#707070"     // ตัวอักษรเทา
#define COLOR_ACCENT "#4ec9b0"     // เขียว (แทนฟ้า)
#define COLOR_SUCCESS "#4ec9b0"     // เขียว
#define COLOR_WARNING "#dcdcaa"     // เหลือง
#define COLOR_ERROR "#f48771"

// ================================================================
//  SparkLineWidget
// ================================================================
SparkLineWidget::SparkLineWidget(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    InData.reserve(MaxPoints);
    OutData.reserve(MaxPoints);
}

void SparkLineWidget::AddValue(int inRate, int outRate) {
    InData.push_back(inRate);
    if (InData.size() > MaxPoints) InData.pop_front();

    OutData.push_back(outRate);
    if (OutData.size() > MaxPoints) OutData.pop_front();

    if (inRate > PeakIn)   PeakIn = inRate;
    if (outRate > PeakOut) PeakOut = outRate;

    int currentMax = qMax(inRate, outRate);

    if (currentMax > StickyMax) {
        // Peak ใหม่ → ขึ้นทันที
        StickyMax = currentMax;
    } else {
        // ค่อย ๆ ลดลง 1% ต่อรอบ (ที่ 10Hz = ~10 วินาที จาก 1000→370)
        int newMax = (int)(StickyMax * 0.99f);
        if (newMax < 10) newMax = 10;
        StickyMax = newMax;
    }


    update();
}

void SparkLineWidget::Clear() {
    InData.clear();
    OutData.clear();
    PeakIn = PeakOut = 0;
    update();
}

int SparkLineWidget::GetPeakValue() const {
    return qMax(PeakIn, PeakOut);
}

void SparkLineWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    int W = width();
    int H = height();
    if (W <= 0 || H <= 0) return;

    p.fillRect(rect(), QColor(COLOR_BG));

    // ============================================================
    //  ⭐ กำหนดพื้นที่สำหรับแกน
    // ============================================================
    const int MarginLeft   = 35;   // ← ที่ให้ตัวเลข Y-axis
    const int MarginBottom = 18;   // ← ที่ให้ label X-axis
    const int MarginTop    = 6;

    int GraphX = MarginLeft;
    int GraphY = MarginTop;
    int GraphW = W - MarginLeft - 6;
    int GraphH = H - MarginTop - MarginBottom;

    if (GraphW <= 0 || GraphH <= 0) return;

    if (InData.size() < 2) {
        p.setPen(QColor(COLOR_DIM));
        p.setFont(QFont("Segoe UI", 10));
        p.drawText(rect(), Qt::AlignCenter, "Waiting for traffic...");
        return;
    }

    // ============================================================
    //  ค่า Max (ใช้ StickyMax)
    // ============================================================
    int MaxValue = StickyMax;
    if (MaxValue < 10) MaxValue = 10;

    static const unsigned char BRAILLE_BIT[4][2] = {
        {0x01, 0x08}, {0x02, 0x10},
        {0x04, 0x20}, {0x40, 0x80},
    };

    const int FontSize = 11;
    QFont BrailleFont("Consolas", FontSize);
    p.setFont(BrailleFont);
    QFontMetrics fm(BrailleFont);

    int CharW = fm.horizontalAdvance(QChar(0x28FF));
    int CharH = fm.height();
    if (CharW < 1 || CharH < 1) return;

    // ---- คำนวณ grid ภายในพื้นที่กราฟ ----
    int CharRows = GraphH / CharH;
    if (CharRows < 1) return;
    int CharCols = GraphW / CharW;
    if (CharCols < 1) return;

    int SubCols   = CharCols * 2;
    int SubRows   = CharRows * 4;
    int CenterRow = SubRows / 2;
    int Baseline  = fm.ascent();

    // ============================================================
    //  ⭐ Grid แนวนอน (5 เส้น)
    // ============================================================
    QPen gridPen(QColor(COLOR_BORDER), 1, Qt::DotLine);
    p.setPen(gridPen);
    for (int i = 0; i < 5; ++i) {
        int y = GraphY + (GraphH * i) / 4;
        p.drawLine(GraphX, y, GraphX + GraphW, y);
    }

    // ============================================================
    //  Interpolate
    // ============================================================
    int DataCount = InData.size();
    QVector<float> Smooth(SubCols);

    for (int sc = 0; sc < SubCols; ++sc) {
        float t = (float)sc / (SubCols - 1) * (DataCount - 1);
        int i0 = (int)t;
        int i1 = qMin(i0 + 1, DataCount - 1);
        float frac = t - i0;
        Smooth[sc] = InData[i0] * (1.0f - frac) + InData[i1] * frac;
    }

    // ============================================================
    //  วาดอักขระ Braille
    // ============================================================
    p.save();
    p.setClipRect(GraphX, GraphY, GraphW, GraphH);
    p.translate(GraphX, GraphY);

    for (int c = 0; c < CharCols; ++c) {
        for (int r = 0; r < CharRows; ++r) {

            unsigned char mask = 0;

            for (int dr = 0; dr < 4; ++dr) {
                for (int dc = 0; dc < 2; ++dc) {
                    int sc = c * 2 + dc;
                    if (sc >= SubCols) continue;

                    int sr = r * 4 + dr;
                    int dist = qAbs(sr - CenterRow);

                    float val = Smooth[sc] / MaxValue;
                    int reach = (int)(val * CenterRow);

                    if (dist <= reach) mask |= BRAILLE_BIT[dr][dc];
                }
            }

            if (mask != 0) {
                float rowRatio = 1.0f - (float)r / CharRows;
                QColor color;
                if (rowRatio < 0.3)      color = QColor(78, 201, 176);
                else if (rowRatio < 0.6) color = QColor(140, 220, 190);
                else                     color = QColor(0, 212, 255);

                p.setPen(color);
                p.drawText(c * CharW, r * CharH + Baseline,
                           QString(QChar(0x2800 + mask)));
            }
        }
    }

    p.restore();

    // ============================================================
    //  ⭐ Y-axis Labels (ซ้าย) — Max, Mid, 0, -Mid, -Max
    // ============================================================
    p.setFont(QFont("Consolas", 8));
    p.setPen(QColor(COLOR_DIM));

    int centerY = GraphY + GraphH / 2;
    int stepY   = GraphH / 4;

    QString yLabels[5] = {
        QString::number(MaxValue),            // บนสุด
        QString::number(MaxValue / 2),        // Q1
        "0",                                   // กลาง
        "-" + QString::number(MaxValue / 2),  // Q3
        "-" + QString::number(MaxValue)       // ล่างสุด
    };

    for (int i = 0; i < 5; ++i) {
        int y = GraphY + stepY * i;
        p.drawText(QRect(0, y - 7, MarginLeft - 4, 14),
                   Qt::AlignRight | Qt::AlignVCenter,
                   yLabels[i]);
    }

    // ============================================================
    //  ⭐ X-axis Labels (ล่าง) — -60s, -30s, now
    // ============================================================
    p.setPen(QColor(COLOR_DIM));
    int labelY = H - 4;

    // คำนวณเวลาจริง — 100ms ต่อ 1 จุด
    int totalSec = (DataCount * 100) / 1000;

    p.drawText(QRect(GraphX, labelY - 12, 50, 14),
               Qt::AlignLeft | Qt::AlignVCenter,
               QString("-%1s").arg(totalSec));

    p.drawText(QRect(GraphX + GraphW/2 - 25, labelY - 12, 50, 14),
               Qt::AlignCenter,
               QString("-%1s").arg(totalSec / 2));

    p.drawText(QRect(GraphX + GraphW - 50, labelY - 12, 50, 14),
               Qt::AlignRight | Qt::AlignVCenter,
               "now");


}

// ================================================================
//  ResourceBar
// ================================================================
ResourceBar::ResourceBar(const QString &Label, QWidget *parent)
    : QWidget(parent), LabelText(Label)
{
    setFixedHeight(28);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void ResourceBar::SetPercent(int percent) {
    Percent = qBound(0, percent, 100);
    update();
}

void ResourceBar::SetSubText(const QString &text) {
    SubText = text;
    update();
}

void ResourceBar::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    int W = width();
    int H = height();

    // ---- Label ----
    p.setPen(QColor(COLOR_TEXT));
    p.setFont(QFont("Segoe UI", 10));
    p.drawText(QRect(0, 0, 55, H), Qt::AlignLeft | Qt::AlignVCenter, LabelText);

    // ---- Progress Bar ----
    int barX = 60;
    int barW = W - 60 - 110;   // เหลือที่ให้ %
    int barH = 8;
    int barY = (H - barH) / 2;

    // พื้นหลัง bar
    p.fillRect(barX, barY, barW, barH, QColor("#2d2d30"));

    // เติมสีตามค่า
    int fillW = (barW * Percent) / 100;
    QColor barColor(COLOR_ACCENT);
    if (Percent > 80)      barColor = QColor(COLOR_ERROR);
    else if (Percent > 60) barColor = QColor(COLOR_WARNING);
    else                   barColor = QColor(COLOR_SUCCESS);

    p.fillRect(barX, barY, fillW, barH, barColor);

    // ---- SubText + Percent (ชิดขวา) ----
    p.setPen(QColor(COLOR_DIM));
    p.setFont(QFont("Consolas", 9));
    QString right = SubText.isEmpty()
        ? QString("%1%").arg(Percent)
        : QString("%1  %2%").arg(SubText).arg(Percent);
    p.drawText(QRect(barX + barW + 5, 0, 105, H),
               Qt::AlignRight | Qt::AlignVCenter, right);
}

// ================================================================
//  ResourceWidget
// ================================================================
ResourceWidget::ResourceWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *L = new QVBoxLayout(this);
    L->setContentsMargins(0, 0, 0, 0);
    L->setSpacing(2);

    CPUBar  = new ResourceBar("CPU",  this);
    RAMBar  = new ResourceBar("RAM",  this);
    DiskBar = new ResourceBar("DISK", this);
    GPUBar  = new ResourceBar("GPU",  this);

    L->addWidget(CPUBar);
    L->addWidget(RAMBar);
    L->addWidget(DiskBar);
    L->addWidget(GPUBar);
}

void ResourceWidget::UpdateResources() {
    // ---------- CPU ----------
    static FILETIME prevIdle, prevKernel, prevUser;
    FILETIME idle, kernel, user;

    if (GetSystemTimes(&idle, &kernel, &user)) {
        ULARGE_INTEGER i, k, u, pi, pk, pu;
        i.LowPart = idle.dwLowDateTime;    i.HighPart = idle.dwHighDateTime;
        k.LowPart = kernel.dwLowDateTime;  k.HighPart = kernel.dwHighDateTime;
        u.LowPart = user.dwLowDateTime;    u.HighPart = user.dwHighDateTime;
        pi.LowPart = prevIdle.dwLowDateTime;    pi.HighPart = prevIdle.dwHighDateTime;
        pk.LowPart = prevKernel.dwLowDateTime;  pk.HighPart = prevKernel.dwHighDateTime;
        pu.LowPart = prevUser.dwLowDateTime;    pu.HighPart = prevUser.dwHighDateTime;

        if (prevIdle.dwLowDateTime || prevIdle.dwHighDateTime) {
            ULONGLONG idleD   = i.QuadPart - pi.QuadPart;
            ULONGLONG kernelD = k.QuadPart - pk.QuadPart;
            ULONGLONG userD   = u.QuadPart - pu.QuadPart;
            ULONGLONG total   = kernelD + userD;

            if (total > 0) {
                double cpu = (1.0 - (double)idleD / total) * 100.0;
                int pct = (int)cpu;
                CPUBar->SetPercent(pct);
                CPUBar->SetSubText("");
            }
        }
        prevIdle = idle; prevKernel = kernel; prevUser = user;
    }

    // ---------- RAM ----------
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    if (GlobalMemoryStatusEx(&mem)) {
        int pct = mem.dwMemoryLoad;
        double totalGB = mem.ullTotalPhys / (1024.0 * 1024 * 1024);
        double usedGB  = totalGB * pct / 100.0;

        RAMBar->SetPercent(pct);
        RAMBar->SetSubText(QString("%1/%2G")
            .arg(usedGB, 0, 'f', 1).arg(totalGB, 0, 'f', 0));
    }

    // ---------- DISK ----------
    ULARGE_INTEGER freeB, totalB, totalFreeB;
    if (GetDiskFreeSpaceExW(L"C:\\", &freeB, &totalB, &totalFreeB)) {
        double totalGB = totalB.QuadPart / (1024.0 * 1024 * 1024);
        double freeGB  = freeB.QuadPart  / (1024.0 * 1024 * 1024);
        double usedGB  = totalGB - freeGB;
        int pct = totalGB > 0 ? (int)(usedGB / totalGB * 100) : 0;

        DiskBar->SetPercent(pct);
        DiskBar->SetSubText(QString("%1/%2G")
            .arg(usedGB, 0, 'f', 0).arg(totalGB, 0, 'f', 0));
    }

    // ---------- GPU ----------
    GPUBar->SetPercent(0);
    GPUBar->SetSubText("N/A");
}

// ================================================================
//  TrafficGraph
// ================================================================
TrafficGraph::TrafficGraph(QWidget *parent) : QWidget(parent) {
    this->setStyleSheet("background-color: " COLOR_BG ";");

    QVBoxLayout *Main = new QVBoxLayout(this);
    Main->setContentsMargins(16, 16, 16, 16);
    Main->setSpacing(12);

    // ============================================
    //  Traffic Section
    // ============================================

    QHBoxLayout *TrafficHeader = new QHBoxLayout();
    TrafficHeader->setContentsMargins(0, 4, 0, 0);
    TrafficHeader->setSpacing(16);

    // ============ ซ้าย: packets/sec ============
    QLabel *Label = new QLabel("packets/sec", this);
    Label->setStyleSheet("color: " COLOR_DIM "; font-size: 11px; background: transparent;");
    TrafficHeader->addWidget(Label);

    // ⭐ ดันทุกอย่างที่ตามมาไปทางขวา
    TrafficHeader->addStretch();

    // ============ ขวา: IN ============
    QLabel *InTag = new QLabel("▼ IN", this);
    InTag->setStyleSheet("color: #00d4ff; font-size: 12px; font-weight: 600; background: transparent;");
    TrafficHeader->addWidget(InTag);

    InRateLabel = new QLabel("0 pps", this);
    InRateLabel->setStyleSheet("color: #00d4ff; font-size: 12px; font-weight: 600; background: transparent;");
    TrafficHeader->addWidget(InRateLabel);

    InPeakLabel = new QLabel("peak 0", this);
    InPeakLabel->setStyleSheet("color: " COLOR_DIM "; font-size: 10px; background: transparent;");
    TrafficHeader->addWidget(InPeakLabel);

    // ============ ขวา: OUT ============
    QLabel *OutTag = new QLabel("▲ OUT", this);
    OutTag->setStyleSheet("color: #4ec9b0; font-size: 12px; font-weight: 600; background: transparent; padding-left: 16px;");
    TrafficHeader->addWidget(OutTag);

    OutRateLabel = new QLabel("0 pps", this);
    OutRateLabel->setStyleSheet("color: #4ec9b0; font-size: 12px; font-weight: 600; background: transparent;");
    TrafficHeader->addWidget(OutRateLabel);

    OutPeakLabel = new QLabel("peak 0", this);
    OutPeakLabel->setStyleSheet("color: " COLOR_DIM "; font-size: 10px; background: transparent;");
    TrafficHeader->addWidget(OutPeakLabel);

    Main->addLayout(TrafficHeader);

    // กราฟ
    MainSparkLine = new SparkLineWidget(this);
    Main->addWidget(MainSparkLine, 3);   // ใช้พื้นที่ 3 ส่วน

    // ============================================
    //  System Resources Section
    // ============================================
    QLabel *ResourceTitle = new QLabel("System Resources", this);
    ResourceTitle->setStyleSheet(
        "color: " COLOR_TEXT "; font-size: 13px; font-weight: 600;"
        "background: transparent; padding-top: 8px;"
    );
    Main->addWidget(ResourceTitle);

    QFrame *Line2 = new QFrame(this);
    Line2->setFixedHeight(1);
    Line2->setStyleSheet("background-color: " COLOR_BORDER ";");
    Main->addWidget(Line2);

    MainResourceWidget = new ResourceWidget(this);
    Main->addWidget(MainResourceWidget, 1);

    // ============================================
    //  Timer
    // ============================================
    MainResourceTimer = new QTimer(this);
    connect(MainResourceTimer, &QTimer::timeout, this, [this]() {
        MainResourceWidget->UpdateResources();
    });
    MainResourceTimer->start(1000);
    MainResourceWidget->UpdateResources();
}

void TrafficGraph::UpdatePacketCount(int inCount, int outCount) {
    if (MainFirst) {
        MainLastIn  = inCount;
        MainLastOut = outCount;
        MainFirst = 0;
        return;
    }

    int inDiff  = inCount  - MainLastIn;
    int outDiff = outCount - MainLastOut;
    if (inDiff  < 0) inDiff  = 0;
    if (outDiff < 0) outDiff = 0;

    int inRate  = inDiff  * 10;
    int outRate = outDiff * 10;

    MainLastIn  = inCount;
    MainLastOut = outCount;

    MainSparkLine->AddValue(inRate, outRate);

    // ---- Smooth ----
    static float smoothIn  = 0;
    static float smoothOut = 0;
    smoothIn  = smoothIn  * 0.6f + inRate  * 0.4f;
    smoothOut = smoothOut * 0.6f + outRate * 0.4f;

    // ---- อัปเดต Labels ----
    InRateLabel->setText(QString("%1 pps").arg((int)smoothIn));
    OutRateLabel->setText(QString("%1 pps").arg((int)smoothOut));

    InPeakLabel->setText(QString("peak %1").arg(MainSparkLine->GetPeakIn()));
    OutPeakLabel->setText(QString("peak %1").arg(MainSparkLine->GetPeakOut()));
}