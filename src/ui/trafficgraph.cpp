#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QFrame>

#include <winsock2.h>
#include <windows.h>
#include <psapi.h>
#include <iphlpapi.h>
#include <netioapi.h>
#include <pdh.h>
#include <pdhmsg.h>

#include "trafficgraph.h"
#include "receiver/packet_counter.h"

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

static bool GetNetSpeed(double &inMbps, double &outMbps) {
    static PDH_HQUERY   hQuery = nullptr;
    static PDH_HCOUNTER hIn    = nullptr;
    static PDH_HCOUNTER hOut   = nullptr;
    static bool init = false;

    inMbps  = 0;
    outMbps = 0;

    if (!init) {
        init = true;
        if (PdhOpenQuery(NULL, 0, &hQuery) != ERROR_SUCCESS) return false;

        PdhAddEnglishCounterW(hQuery,
            L"\\Network Interface(*)\\Bytes Received/sec",
            0, &hIn);
        PdhAddEnglishCounterW(hQuery,
            L"\\Network Interface(*)\\Bytes Sent/sec",
            0, &hOut);

        PdhCollectQueryData(hQuery);
        return false;   // รอบแรก ยังไม่มีข้อมูล
    }

    if (!hQuery) return false;
    if (PdhCollectQueryData(hQuery) != ERROR_SUCCESS) return false;

    // ---- ดึงค่า array ของทุก adapter ----
    auto SumArray = [](PDH_HCOUNTER h) -> double {
        DWORD bufSize = 0;
        DWORD itemCount = 0;
        PDH_STATUS st = PdhGetFormattedCounterArrayW(
            h, PDH_FMT_DOUBLE, &bufSize, &itemCount, NULL);

        if (st != PDH_MORE_DATA || bufSize == 0) return 0.0;

        QByteArray buf(bufSize, 0);
        PDH_FMT_COUNTERVALUE_ITEM_W *items =
            (PDH_FMT_COUNTERVALUE_ITEM_W*)buf.data();

        st = PdhGetFormattedCounterArrayW(
            h, PDH_FMT_DOUBLE, &bufSize, &itemCount, items);

        if (st != ERROR_SUCCESS) return 0.0;

        double sum = 0.0;
        for (DWORD i = 0; i < itemCount; ++i) {
            if (items[i].FmtValue.CStatus == ERROR_SUCCESS) {
                sum += items[i].FmtValue.doubleValue;
            }
        }
        return sum;
    };

    double bytesIn  = SumArray(hIn);
    double bytesOut = SumArray(hOut);

    inMbps  = bytesIn  * 8.0 / 1'000'000.0;
    outMbps = bytesOut * 8.0 / 1'000'000.0;

    return true;
}

static int GetGPUUsage() {
    static PDH_HQUERY   hQuery      = nullptr;
    static bool         initialized = false;
    static QVector<PDH_HCOUNTER> counters;

    if (!initialized) {
        initialized = true;
        if (PdhOpenQuery(NULL, 0, &hQuery) != ERROR_SUCCESS) {
            qDebug() << "[GPU] PdhOpenQuery failed";
            return -1;
        }

        DWORD bufSize = 0;
        PDH_STATUS status = PdhExpandWildCardPathW(
            NULL,
            L"\\GPU Engine(*)\\Utilization Percentage",
            NULL,
            &bufSize,
            0
        );

        qDebug() << "[GPU] Expand status:" << status << "bufSize:" << bufSize;

        if (status != PDH_MORE_DATA || bufSize == 0) {
            PdhCloseQuery(hQuery);
            hQuery = nullptr;
            qDebug() << "[GPU] No GPU Engine counter available";
            return -1;
        }

        QByteArray buf(bufSize * sizeof(wchar_t), 0);
        wchar_t *paths = (wchar_t*)buf.data();

        status = PdhExpandWildCardPathW(
            NULL,
            L"\\GPU Engine(*)\\Utilization Percentage",
            paths,
            &bufSize,
            0
        );

        if (status != ERROR_SUCCESS) {
            PdhCloseQuery(hQuery);
            hQuery = nullptr;
            qDebug() << "[GPU] Expand #2 failed";
            return -1;
        }

        // ---- เพิ่ม counter ----
        int count = 0;
        wchar_t *p = paths;
        while (*p) {
            PDH_HCOUNTER hCounter = nullptr;
            if (PdhAddCounterW(hQuery, p, 0, &hCounter) == ERROR_SUCCESS) {
                counters.append(hCounter);
                count++;
            }
            p += wcslen(p) + 1;
        }

        qDebug() << "[GPU] Added counters:" << count;

        if (counters.isEmpty()) {
            PdhCloseQuery(hQuery);
            hQuery = nullptr;
            return -1;
        }

        PdhCollectQueryData(hQuery);
        return -1;
    }

    if (!hQuery || counters.isEmpty()) return -1;
    if (PdhCollectQueryData(hQuery) != ERROR_SUCCESS) return -1;

    // ============================================================
    //  ✅ เปลี่ยนจาก SUM → MAX (busiest engine)
    // ============================================================
    double maxVal = 0.0;
    int validCount = 0;

    for (PDH_HCOUNTER h : counters) {
        PDH_FMT_COUNTERVALUE value;
        if (PdhGetFormattedCounterValue(h, PDH_FMT_DOUBLE, NULL, &value) == ERROR_SUCCESS) {
            if (value.CStatus == ERROR_SUCCESS) {
                if (value.doubleValue > maxVal) maxVal = value.doubleValue;
                validCount++;
            }
        }
    }

    if (validCount == 0) return -1;
    if (maxVal > 100.0) maxVal = 100.0;
    return (int)maxVal;
}

CoreBar::CoreBar(int coreIndex, QWidget *parent)
    : QWidget(parent), CoreIndex(coreIndex)
{
    setFixedHeight(22);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void CoreBar::SetPercent(int percent) {
    Percent = qBound(0, percent, 100);
    update();
}

void CoreBar::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    int W = width();
    int H = height();

    // ✅ Label ที่ x=25 เหมือน ResourceBar
    p.setPen(QColor("#e0e0e0"));
    p.setFont(QFont("Consolas", 9));
    p.drawText(QRect(25, 0, 65, H), Qt::AlignLeft | Qt::AlignVCenter,
               QString("Core %1").arg(CoreIndex));

    // ✅ Bar ที่ x=95 เหมือน ResourceBar
    int barX = 95;
    int barW = W - 95 - 55;
    int barH = 6;
    int barY = (H - barH) / 2;

    p.fillRect(barX, barY, barW, barH, QColor("#2d2d30"));

    int fillW = (barW * Percent) / 100;

    QColor barColor;
    if (Percent > 80)      barColor = QColor("#f48771");
    else if (Percent > 60) barColor = QColor("#dcdcaa");
    else                   barColor = QColor("#4ec9b0");

    p.fillRect(barX, barY, fillW, barH, barColor);

    // ✅ % ชิดขวา
    p.setPen(QColor("#707070"));
    p.setFont(QFont("Consolas", 9));
    p.drawText(QRect(barX + barW + 4, 0, 50, H),
               Qt::AlignRight | Qt::AlignVCenter,
               QString("%1%").arg(Percent));
}

// ================================================================
//  CPUWidget
// ================================================================
CPUWidget::CPUWidget(QWidget *parent) : QWidget(parent) {

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);   // ← เพิ่ม
    setMinimumHeight(250);                                            // ← เพิ่ม

    QVBoxLayout *MainLayout = new QVBoxLayout(this);
    MainLayout->setContentsMargins(0, 0, 0, 0);
    MainLayout->setSpacing(4);

    // ---- Title ----
    QLabel *Title = new QLabel("CPU", this);
    Title->setStyleSheet("color: #e0e0e0; font-size: 11px; font-weight: 600;");
    MainLayout->addWidget(Title);

    // ---- Line ----
    QFrame *Line = new QFrame(this);
    Line->setFixedHeight(1);
    Line->setStyleSheet("background-color: #2d2d2d;");
    MainLayout->addWidget(Line);

    // ---- Scroll Area ----
    ScrollArea = new QScrollArea(this);
    ScrollArea->setWidgetResizable(true);
    ScrollArea->setFrameShape(QFrame::NoFrame);
    ScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    ScrollArea->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical {"
        "  background: #1a1a1a; width: 6px; border: none;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #3e3e42; border-radius: 3px; min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover { background: #505054; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0px;"
        "}"
    );

    BarsContainer = new QWidget();
    BarsContainer->setStyleSheet("background: transparent;");
    QVBoxLayout *BarsLayout = new QVBoxLayout(BarsContainer);
    BarsLayout->setContentsMargins(0, 0, 0, 0);
    BarsLayout->setSpacing(2);

    // ---- ดึงจำนวน cores ----
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    CoreCount = si.dwNumberOfProcessors;
    if (CoreCount < 1) CoreCount = 1;
    if (CoreCount > 64) CoreCount = 64;   // กัน overflow

    // ---- สร้าง CoreBar ----
    CoreBars.reserve(CoreCount);
    for (int i = 0; i < CoreCount; ++i) {
        CoreBar *bar = new CoreBar(i, BarsContainer);
        BarsLayout->addWidget(bar);
        CoreBars.append(bar);
    }
    BarsLayout->addStretch();

    ScrollArea->setWidget(BarsContainer);
    MainLayout->addWidget(ScrollArea, 1);
}

void CPUWidget::UpdateCPU() {
    // ============================================================
    //  ใช้ NtQuerySystemInformation ดึงข้อมูลแต่ละ core
    // ============================================================
    typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION {
        LARGE_INTEGER IdleTime;
        LARGE_INTEGER KernelTime;
        LARGE_INTEGER UserTime;
        LARGE_INTEGER DpcTime;
        LARGE_INTEGER InterruptTime;
        ULONG         InterruptCount;
    } SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

    typedef LONG (WINAPI *PROCNTQSIP)(ULONG, PVOID, ULONG, PULONG);
    static PROCNTQSIP NtQuerySystemInformation = nullptr;

    if (!NtQuerySystemInformation) {
        HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
        if (hMod) {
            NtQuerySystemInformation = (PROCNTQSIP)
                GetProcAddress(hMod, "NtQuerySystemInformation");
        }
        if (!NtQuerySystemInformation) return;
    }

    // SystemProcessorPerformanceInformation = 8
    QVector<SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION> info(CoreCount);
    ULONG retLen = 0;
    LONG status = NtQuerySystemInformation(
        8,
        info.data(),
        sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * CoreCount,
        &retLen
    );

    if (status != 0) return;

    // ---- เก็บค่าเก่าไว้เทียบ ----
    static QVector<SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION> prev(CoreCount);
    static bool initialized = false;

    if (!initialized) {
        prev = info;
        initialized = true;
        return;
    }

    for (int i = 0; i < CoreCount; ++i) {
        ULONGLONG prevIdle   = prev[i].IdleTime.QuadPart;
        ULONGLONG prevKernel = prev[i].KernelTime.QuadPart;
        ULONGLONG prevUser   = prev[i].UserTime.QuadPart;

        ULONGLONG currIdle   = info[i].IdleTime.QuadPart;
        ULONGLONG currKernel = info[i].KernelTime.QuadPart;
        ULONGLONG currUser   = info[i].UserTime.QuadPart;

        ULONGLONG idleDelta   = currIdle   - prevIdle;
        ULONGLONG kernelDelta = currKernel - prevKernel;
        ULONGLONG userDelta   = currUser   - prevUser;
        ULONGLONG totalDelta  = kernelDelta + userDelta;

        int pct = 0;
        if (totalDelta > 0) {
            pct = (int)((1.0 - (double)idleDelta / totalDelta) * 100.0);
            if (pct < 0)   pct = 0;
            if (pct > 100) pct = 100;
        }

        CoreBars[i]->SetPercent(pct);
    }

    prev = info;
}

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

void SparkLineWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    int W = width();
    int H = height();
    if (W <= 0 || H <= 0) return;

    p.fillRect(rect(), QColor(COLOR_BG));

    // ============================================================
    //  กำหนดพื้นที่สำหรับแกน
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
    setFixedHeight(62);   // ← จาก 28 → 62 (3 แถว)
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

    // ---- Layout ----
    const int TitleY   = 4;      // แถว 1: Title
    const int Row2Y    = 24;     // แถว 2: Usage + Bar + %
    const int Row3Y    = 44;     // แถว 3: SubText

    const int LabelX   = 0;      // "Usage" label
    const int BarX     = 60;     // Bar
    const int BarW     = W - 60 - 55;
    const int BarH     = 8;

    // ============================================================
    //  แถว 1: Title (RAM / DISK / GPU / NET)
    // ============================================================
    p.setPen(QColor(COLOR_TEXT));
    p.setFont(QFont("Segoe UI", 10, QFont::DemiBold));
    p.drawText(QRect(20, TitleY, W, 18),
               Qt::AlignLeft | Qt::AlignVCenter, LabelText);

    // ============================================================
    //  แถว 2: "Usage" + Bar + %
    // ============================================================
    // ---- "Usage" ----
    p.setPen(QColor(COLOR_DIM));
    p.setFont(QFont("Segoe UI", 9));
    p.drawText(QRect(20, Row2Y, BarX - 4, BarH + 8),
               Qt::AlignLeft | Qt::AlignVCenter, "Usage");

    // ---- Bar ----
    int barY = Row2Y + (BarH + 8 - BarH) / 2;

    // Background
    p.fillRect(BarX, barY, BarW, BarH, QColor("#2d2d30"));

    // Fill
    int fillW = (BarW * Percent) / 100;
    QColor barColor(COLOR_ACCENT);
    if (Percent > 80)      barColor = QColor(COLOR_ERROR);
    else if (Percent > 60) barColor = QColor(COLOR_WARNING);
    else                   barColor = QColor(COLOR_SUCCESS);
    p.fillRect(BarX, barY, fillW, BarH, barColor);

    // ---- % ----
    p.setPen(QColor(COLOR_DIM));
    p.setFont(QFont("Consolas", 9));
    p.drawText(QRect(BarX + BarW + 4, Row2Y, 50, BarH + 8),
               Qt::AlignRight | Qt::AlignVCenter,
               QString("%1%").arg(Percent));

    // ============================================================
    //  แถว 3: SubText (ด้านล่าง)
    // ============================================================
    if (!SubText.isEmpty()) {
        p.setPen(QColor(COLOR_DIM));
        p.setFont(QFont("Consolas", 8));
        p.drawText(QRect(BarX, Row3Y, BarW, 14),
                   Qt::AlignLeft | Qt::AlignVCenter, SubText);
    }
}

// ================================================================
//  ResourceWidget
// ================================================================
ResourceWidget::ResourceWidget(QWidget *parent) : QWidget(parent) {

    QHBoxLayout *MainLayout = new QHBoxLayout(this);
    MainLayout->setContentsMargins(0, 0, 0, 0);
    MainLayout->setSpacing(20);

    // ============================================
    //  Column 1: CPU
    // ============================================
    CPUColumn = new CPUWidget(this);
    CPUColumn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    MainLayout->addWidget(CPUColumn, 3);

    // ============================================
    //  Column 2: RAM, DISK, GPU, NET
    // ============================================
    QWidget *RightCol = new QWidget(this);
    RightCol->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *RightLayout = new QVBoxLayout(RightCol);
    RightLayout->setContentsMargins(0, 0, 0, 0);
    RightLayout->setSpacing(10);

    RAMBar  = new ResourceBar("RAM",  RightCol);
    DiskBar = new ResourceBar("DISK", RightCol);
    GPUBar  = new ResourceBar("GPU",  RightCol);
    NETBar  = new ResourceBar("NET",  RightCol);

    RightLayout->addWidget(RAMBar);
    RightLayout->addWidget(DiskBar);
    RightLayout->addWidget(GPUBar);
    RightLayout->addWidget(NETBar);
    RightLayout->addStretch();
    CPUColumn->setFixedHeight(320);
    RightCol->setFixedHeight(320);

    MainLayout->addWidget(RightCol, 4);
}

void ResourceWidget::UpdateResources() {

    qDebug() << "=== SIZE DEBUG ==="
             << "CPU:" << CPUColumn->height()
             << "Resource:" << this->height();

    // ---- CPU (per-core) ----
    CPUColumn->UpdateCPU();

    // ---- RAM ----
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    if (GlobalMemoryStatusEx(&mem)) {
        int pct = mem.dwMemoryLoad;
        double totalGB = mem.ullTotalPhys / (1024.0 * 1024 * 1024);
        double usedGB  = totalGB * pct / 100.0;
        RAMBar->SetPercent(pct);
        RAMBar->SetSubText(QString("%1 / %2G")
            .arg(usedGB, 0, 'f', 1).arg(totalGB, 0, 'f', 0));
    }

    // ---- DISK ----
    ULARGE_INTEGER freeB, totalB, totalFreeB;
    if (GetDiskFreeSpaceExW(L"C:\\", &freeB, &totalB, &totalFreeB)) {
        double totalGB = totalB.QuadPart / (1024.0 * 1024 * 1024);
        double freeGB  = freeB.QuadPart  / (1024.0 * 1024 * 1024);
        double usedGB  = totalGB - freeGB;
        int pct = totalGB > 0 ? (int)(usedGB / totalGB * 100) : 0;
        DiskBar->SetPercent(pct);
        DiskBar->SetSubText(QString("%1 / %2G")
            .arg(usedGB, 0, 'f', 0).arg(totalGB, 0, 'f', 0));
    }

    // ---- GPU ----
    int gpu = GetGPUUsage();
    if (gpu < 0) {
        GPUBar->SetPercent(0);
        GPUBar->SetSubText("N/A");
    } else {
        GPUBar->SetPercent(gpu);
        GPUBar->SetSubText(QString("%1%").arg(gpu));
    }

    // ---- NET ----
    double inMbps = 0, outMbps = 0;
    if (GetNetSpeed(inMbps, outMbps)) {
        double total = inMbps + outMbps;

        // คำนวณ % — สมมติ bandwidth = 100 Mbps
        int pct = qMin(100, (int)(total));

        NETBar->SetPercent(pct);
        NETBar->SetSubText(QString("↓%1 ↑%2 Mbps")
            .arg(inMbps,  0, 'f', 1)
            .arg(outMbps, 0, 'f', 1));
    } else {
        NETBar->SetPercent(0);
        NETBar->SetSubText("...");
    }
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
    Main->addWidget(MainSparkLine, 2);   // ใช้พื้นที่ 3 ส่วน

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
    MainResourceWidget->setFixedHeight(320);
    Main->addWidget(MainResourceWidget, 0);

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