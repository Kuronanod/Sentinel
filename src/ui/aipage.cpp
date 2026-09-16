#include "aipage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QDateTime>
#include <QScrollBar>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QtMath>
#include <QRandomGenerator>
#include <algorithm>

#include "receiver/packet_counter.h"
#include "receiver/prefilter.h"

// ================================================================
//  Palette
// ================================================================
#define COLOR_BG          "#1a1a1a"
#define COLOR_PANEL       "#212121"
#define COLOR_BORDER      "#2d2d2d"
#define COLOR_TEXT        "#e0e0e0"
#define COLOR_DIM         "#707070"
#define COLOR_ACCENT      "#4ec9b0"
#define COLOR_INFO        "#6cb6ff"
#define COLOR_WARNING     "#dcdcaa"
#define COLOR_ERROR       "#f48771"

#define SPHERE_BG         "#0a0e27"
#define COLOR_NODE_DIM    "#3a5f5a"
#define COLOR_NODE_MID    "#4ec9b0"
#define COLOR_NODE_BRIGHT "#00ffcc"
#define COLOR_EDGE_DIM    "#2d4f4a"
#define COLOR_EDGE_BRIGHT "#00d4ff"

// ================================================================
//  AISphere
// ================================================================
AISphere::AISphere(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(280);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    Angle = 0.0f;
    Speaking = false;
    PulsePhase = 0.0f;

    InitPoints(100);

    Timer = new QTimer(this);
    connect(Timer, &QTimer::timeout, this, &AISphere::OnTick);
    Timer->start(16);
}

AISphere::~AISphere() {}

void AISphere::InitPoints(int count) {
    Points.clear();
    Points.reserve(count);

    const float golden = M_PI * (1.0f + qSqrt(5.0f));

    // ---- Monochrome palette ----
    QColor palette[] = {
        QColor("#ffffff"),
        QColor("#d0d0d0"),
        QColor("#a0a0a0"),
        QColor("#707070"),
        QColor("#404040"),
    };

    for (int i = 0; i < count; i++) {
        float y      = 1.0f - (i / (float)(count - 1)) * 2.0f;
        float radius = qSqrt(1.0f - y * y);
        float theta  = golden * i;

        Point3D p;
        p.x = qCos(theta) * radius;
        p.y = y;
        p.z = qSin(theta) * radius;
        p.color = palette[QRandomGenerator::global()->bounded(5)];
        p.size  = 1 + QRandomGenerator::global()->bounded(2);

        Points.append(p);
    }
}

QPointF AISphere::Project(const Point3D &p, float &outDepth, float &outScale) {
    float cosA = qCos(Angle);
    float sinA = qSin(Angle);

    float rotX = p.x * cosA - p.z * sinA;
    float rotZ = p.x * sinA + p.z * cosA;
    float rotY = p.y;

    float tilt = 0.3f;
    float cosT = qCos(tilt);
    float sinT = qSin(tilt);

    float finalY = rotY * cosT - rotZ * sinT;
    float finalZ = rotY * sinT + rotZ * cosT;

    float FOV     = 350.0f;
    float ZOffset = finalZ + 400.0f;
    if (ZOffset < 1.0f) ZOffset = 1.0f;

    outScale = FOV / ZOffset;
    outDepth = (finalZ + 1.0f) * 0.5f;

    float screenX = width()  / 2.0f + rotX   * outScale * 120.0f;
    float screenY = height() / 2.0f + finalY * outScale * 120.0f;

    return QPointF(screenX, screenY);
}

float AISphere::Distance3D(const Point3D &a, const Point3D &b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return qSqrt(dx*dx + dy*dy + dz*dz);
}

void AISphere::OnTick() {
    float speed = Speaking ? 0.025f : 0.008f;
    Angle += speed;
    if (Angle > 2 * M_PI) Angle -= 2 * M_PI;

    PulsePhase += 0.1f;
    if (PulsePhase > 2 * M_PI) PulsePhase -= 2 * M_PI;

    update();
}

void AISphere::SetSpeaking(bool speaking) {
    Speaking = speaking;
}

void AISphere::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // ---- Background (โทนเดียวกับหน้าอื่น) ----
    QRadialGradient bgGrad(width()/2.0, height()/2.0, width()*0.7);
    bgGrad.setColorAt(0.0, QColor("#1e1e1e"));
    bgGrad.setColorAt(1.0, QColor("#141414"));
    p.fillRect(rect(), bgGrad);

    if (Points.isEmpty()) return;

    int N = Points.size();
    QVector<QPointF> ScreenPos(N);
    QVector<float>   Depth(N);
    QVector<float>   Scale(N);

    for (int i = 0; i < N; i++) {
        ScreenPos[i] = Project(Points[i], Depth[i], Scale[i]);
    }

    // ---- Edges (ชัดขึ้น) ----
    const float MAX_DIST  = 0.55f;
    const float edgeAlpha = Speaking ? 220 : 100;
    const float pulse     = (qSin(PulsePhase) + 1.0f) * 0.5f;

    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            float d3 = Distance3D(Points[i], Points[j]);
            if (d3 < MAX_DIST) {
                float a = (1.0f - d3 / MAX_DIST) * edgeAlpha;
                a *= (Depth[i] + Depth[j]) * 0.5f;
                if (Speaking) a *= (0.7f + pulse * 0.3f);

                // ---- Monochrome edge ----
                QColor edgeColor = Speaking ? QColor("#ffffff")
                                            : QColor("#5a5a5a");
                edgeColor.setAlpha((int)qBound(0.0f, a, 255.0f));

                p.setPen(QPen(edgeColor, Speaking ? 1.2 : 0.8));
                p.drawLine(ScreenPos[i], ScreenPos[j]);
            }
        }
    }

    // ---- Nodes (ไกล → ใกล้) ----
    QVector<int> order(N);
    for (int i = 0; i < N; i++) order[i] = i;

    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return Depth[a] < Depth[b];
    });

    for (int idx : order) {
        const Point3D &pt = Points[idx];

        QColor baseColor = pt.color;
        if (Speaking) baseColor = baseColor.lighter(140);

        float alpha = 0.3f + Depth[idx] * 0.7f;
        baseColor.setAlpha((int)(alpha * 255));

        float radius = pt.size + Depth[idx] * 2.5f;
        if (Speaking) radius += pulse * 1.5f;

        QPointF pos = ScreenPos[idx];

        // ---- Glow (ขาว) ----
        if (Depth[idx] > 0.6f) {
            QRadialGradient glow(pos, radius * 3.5f);
            QColor glowColor = QColor("#ffffff");
            glowColor.setAlpha(60);
            glow.setColorAt(0.0, glowColor);
            glowColor.setAlpha(0);
            glow.setColorAt(1.0, glowColor);

            p.setPen(Qt::NoPen);
            p.setBrush(glow);
            p.drawEllipse(pos, radius * 3.5f, radius * 3.5f);
        }

        // ---- Dot ----
        p.setPen(Qt::NoPen);
        p.setBrush(baseColor);
        p.drawEllipse(pos, radius, radius);
    }

    // ---- Speaking Indicator (ขาว) ----
    if (Speaking) {
        QPointF center(width()/2.0, height() - 20);

        for (int i = 0; i < 5; i++) {
            float h = 4.0f + qSin(PulsePhase * 2.0f + i * 0.8f) * 6.0f;
            float x = center.x() - 20.0f + i * 10.0f;

            QColor c = QColor("#ffffff");
            c.setAlpha(180);
            p.setPen(QPen(c, 2.0f));
            p.drawLine(QPointF(x, center.y() - h),
                       QPointF(x, center.y() + h));
        }
    }
}

// ================================================================
//  Chatbot — Raphael
// ================================================================
Chatbot::Chatbot() {
    LastCategory = "";
    LastUserMsg = "";
    ConversationDepth = 0;
    InitRules();
}

void Chatbot::InitRules() {
    // ============ GREETING ============
    Rules.append({
        {"สวัสดี", "หวัดดี", "hello", "hi", "hey", "ดี", "ทักทาย"},
        "สวัสดีครับ ผมชื่อ Raphael\n"
        "ยินดีที่ได้รู้จักนะครับ มีอะไรให้ช่วยไหม?",
        "greeting"
    });

    // ============ STATUS ============
    Rules.append({
        {"สถานะ", "เป็นไง", "status", "stats", "ตอนนี้", "ข้อมูล"},
        "__STATUS__",
        "status"
    });

    Rules.append({
        {"blocked", "บล็อกไป", "บล็อกกี่", "ถูกบล็อก"},
        "__BLOCKED__",
        "status"
    });

    Rules.append({
        {"alert", "แจ้งเตือน", "เตือน"},
        "__ALERTS__",
        "status"
    });

    // ============ HOW-TO: BLOCK ============
    Rules.append({
        {"วิธีบล็อก", "เพิ่ม blacklist", "block ip", "บล็อก ip", "แบน ip"},
        "การบล็อก IP ทำได้ง่าย ๆ ครับ\n\n"
        "1. เข้าไปที่หน้า Management\n"
        "2. พิมพ์ IP ที่ต้องการในช่อง Add IP\n"
        "3. กดปุ่ม + Add IP\n\n"
        "ระบบจะทำ 3 อย่างให้อัตโนมัติ:\n"
        "  - สร้าง Firewall Rule ทั้ง inbound และ outbound\n"
        "  - เพิ่มค่า Blocked Counter\n"
        "  - บันทึก Log Event\n\n"
        "หลังจากนั้น IP นั้นจะไม่สามารถติดต่อได้อีกครับ",
        "howto"
    });

    // ============ HOW-TO: UNBLOCK ============
    Rules.append({
        {"วิธีปลด", "unblock", "remove blacklist", "ลบ blacklist", "เอาออก"},
        "การปลดบล็อก IP มีวิธีดังนี้ครับ\n\n"
        "1. เข้าไปที่หน้า Management\n"
        "2. คลิกขวาที่ IP ในรายการ Blacklist\n"
        "3. เลือกเมนู Remove\n\n"
        "ระบบจะลบ Firewall Rule ให้ทันที\n"
        "และ IP นั้นจะกลับมาเชื่อมต่อได้ตามปกติครับ",
        "howto"
    });

    // ============ HOW-TO: PACKET ============
    Rules.append({
        {"วิธีดู packet", "ดู packet", "ดูข้อมูล packet", "แพ็กเก็ต"},
        "หน้า Packet จะแสดงข้อมูลการรับส่งทั้งหมดครับ\n\n"
        "คอลัมน์ที่มี:\n"
        "  - Time: เวลาที่จับได้\n"
        "  - Source IP / Destination IP\n"
        "  - Source Port / Destination Port\n"
        "  - Protocol: TCP, UDP, ICMP\n\n"
        "เคล็ดลับ: ใช้ช่อง Search ด้านบน\n"
        "เพื่อกรองหา IP หรือ Port ที่ต้องการได้ครับ",
        "howto"
    });

    // ============ HOW-TO: EXPORT ============
    Rules.append({
        {"วิธี export", "export", "save log", "บันทึก log", "ส่งออก"},
        "การ Export Logs ทำได้ตามนี้ครับ\n\n"
        "1. เข้าไปที่หน้า Logs\n"
        "2. กดปุ่ม Export มุมขวาบน\n"
        "3. เลือกโฟลเดอร์และตั้งชื่อไฟล์\n"
        "4. กด Save\n\n"
        "ไฟล์ที่ได้เป็น .log ที่เปิดด้วย\n"
        "Notepad หรือ VS Code ได้เลยครับ",
        "howto"
    });

    // ============ AI EXPLANATION ============
    Rules.append({
        {"ai ทำงาน", "ai คือ", "อธิบาย ai", "raphael ทำงาน", "หลักการ ai"},
        "Raphael ใช้ Hybrid AI Model ครับ\n\n"
        "ส่วนที่ 1: EWMA + Z-Score\n"
        "  เรียนรู้ baseline ของ traffic\n"
        "  ถ้าค่าเบี่ยงเบนเกิน 3 sigma ถือว่า anomaly\n\n"
        "ส่วนที่ 2: Per-IP Profiling\n"
        "  ติดตามพฤติกรรมของแต่ละ IP\n"
        "  IP ใหม่หรือ IP ที่เปลี่ยน pattern จะถูกตั้งข้อสังเกต\n\n"
        "นำ 2 ส่วนมารวมกันเป็น Anomaly Score\n"
        "เพื่อตัดสินใจว่าจะแจ้งเตือนหรือไม่ครับ",
        "explain"
    });

    Rules.append({
        {"z-score", "zscore", "z score"},
        "Z-Score เป็นค่ามาตรฐานทางสถิติครับ\n\n"
        "สูตร: Z = (x - mean) / stddev\n\n"
        "  x      = ค่าที่วัดได้ปัจจุบัน\n"
        "  mean   = ค่าเฉลี่ย (baseline)\n"
        "  stddev = ส่วนเบี่ยงเบนมาตรฐาน\n\n"
        "การแปลผล:\n"
        "  |Z| < 2  = ปกติ\n"
        "  |Z| > 3  = ผิดปกติ (โอกาสน้อยกว่า 0.3%)\n\n"
        "Raphael ใช้ค่านี้เป็นตัวชี้วัดหลักครับ",
        "explain"
    });

    Rules.append({
        {"ewma", "exponential", "moving average"},
        "EWMA ย่อมาจาก Exponentially Weighted Moving Average ครับ\n\n"
        "เป็นเทคนิคที่ให้น้ำหนักกับข้อมูลใหม่มากกว่าข้อมูลเก่า\n"
        "ทำให้ระบบปรับตัวตามการเปลี่ยนแปลงได้เร็วขึ้น\n\n"
        "สูตร: mu_new = alpha * x + (1 - alpha) * mu_old\n\n"
        "Raphael ใช้ alpha = 0.1\n"
        "หมายความว่าให้น้ำหนักข้อมูลใหม่ 10% ครับ",
        "explain"
    });

    // ============ PREFILTER ============
    Rules.append({
        {"pre-filter", "prefilter", "pre filter", "กรอง"},
        "Pre-filter เป็นด่านแรกของระบบครับ\n\n"
        "ทำงาน 3 ขั้นตอน:\n"
        "  1. ตรวจ Blacklist IP\n"
        "  2. ตรวจ Suspicious Port\n"
        "  3. ตรวจ Rate Limit\n\n"
        "ถ้าผ่านทั้ง 3 ขั้นตอน จะส่งต่อให้ Raphael วิเคราะห์\n"
        "ถ้าไม่ผ่าน จะถูกบล็อกทันทีครับ",
        "explain"
    });

    // ============ FIREWALL ============
    Rules.append({
        {"firewall", "ไฟร์วอล", "windows firewall"},
        "Raphael ทำงานร่วมกับ Windows Firewall ครับ\n\n"
        "เมื่อบล็อก IP ระบบจะ:\n"
        "  - สร้าง Firewall Rule ผ่าน netsh\n"
        "  - Block ทั้ง inbound และ outbound\n"
        "  - ตั้งชื่อ Rule: Sentinel_Block_In_<IP>\n\n"
        "เมื่อปลดบล็อก Rule จะถูกลบอัตโนมัติ\n"
        "ไม่ทิ้งขยะไว้ในระบบครับ",
        "explain"
    });

    // ============ HELP ============
    Rules.append({
        {"ช่วย", "help", "คำสั่ง", "ทำอะไรได้", "แนะนำ", "ใช้ยังไง"},
        "__HELP__",
        "help"
    });

    // ============ THANKS ============
    Rules.append({
        {"ขอบคุณ", "thank", "thanks", "แต๊ง", "โอเค"},
        "ยินดีครับ ถ้ามีอะไรให้ช่วยเพิ่มเติม\n"
        "ถามได้เสมอนะครับ",
        "thanks"
    });

    // ============ FAREWELL ============
    Rules.append({
        {"บาย", "บ๊ายบาย", "bye", "goodbye", "ลาก่อน"},
        "ขอบคุณที่ใช้งานครับ\n"
        "Raphael พร้อมช่วยเสมอเมื่อคุณกลับมา",
        "farewell"
    });

    // ============ CAPABILITIES ============
    Rules.append({
        {"ทำอะไรได้บ้าง", "มีความสามารถ", "ability", "capability"},
        "Raphael มีความสามารถ 4 ด้านครับ\n\n"
        "1. วิเคราะห์ Traffic\n"
        "   ตรวจจับความผิดปกติด้วย EWMA + Z-Score\n\n"
        "2. จัดการ Rules\n"
        "   ช่วยเรื่อง Blacklist และ Threshold\n\n"
        "3. ตอบคำถาม\n"
        "   แนะนำการใช้งานระบบ\n\n"
        "4. อธิบายหลักการ\n"
        "   อธิบาย AI, Z-Score, Pre-filter\n\n"
        "ลองถามในเรื่องที่สนใจได้เลยครับ",
        "help"
    });

    // ============ SECURITY ============
    Rules.append({
        {"ปลอดภัย", "security", "ความปลอดภัย"},
        "ระบบ Sentinel มีความปลอดภัยหลายชั้นครับ\n\n"
        "ชั้นที่ 1: Pre-filter\n"
        "  กรองข้อมูลที่รู้จักแล้ว\n\n"
        "ชั้นที่ 2: Raphael AI\n"
        "  วิเคราะห์พฤติกรรมผิดปกติ\n\n"
        "ชั้นที่ 3: Windows Firewall\n"
        "  บล็อกในระดับ Network Layer\n\n"
        "ทั้ง 3 ชั้นทำงานร่วมกันครับ",
        "explain"
    });
}

// ================================================================
//  Respond — Main Logic
// ================================================================
QString Chatbot::Respond(const QString &input) {
    QString lower = input.toLower().trimmed();
    if (lower.isEmpty()) return QString();

    LastUserMsg = input;
    ConversationDepth++;

    // ============ SPECIAL CASES ============
    if (IsGreeting(lower)) {
        LastCategory = "greeting";
        return PickRandom({
            "สวัสดีครับ มีอะไรให้ช่วยไหม?",
            "หวัดดีครับ พร้อมช่วยเสมอ มีคำถามอะไรไหม?",
            "สวัสดีครับ ผม Raphael ยินดีให้บริการครับ"
        });
    }

    if (IsThanking(lower)) {
        LastCategory = "thanks";
        return PickRandom({
            "ยินดีครับ ถ้ามีอะไรถามเพิ่มได้เลย",
            "ด้วยความยินดีครับ",
            "ครับ มีอะไรให้ช่วยเพิ่มบอกได้นะครับ"
        });
    }

    if (IsFarewell(lower)) {
        LastCategory = "farewell";
        return "ขอบคุณที่ใช้งานครับ ไว้เจอกันใหม่";
    }

    // ============ MATCH RULES ============
    int bestScore = 0;
    const Rule *bestRule = nullptr;

    for (const Rule &r : Rules) {
        for (const QString &kw : r.keywords) {
            int score = KeywordScore(lower, kw);
            if (score > bestScore) {
                bestScore = score;
                bestRule = &r;
            }
        }
    }

    // ---- Threshold ผ่าน? ----
    if (bestRule && bestScore > 0) {
        LastCategory = bestRule->category;

        // ---- Special Responses ----
        if (bestRule->response == "__STATUS__") {
            return BuildStatusResponse();
        }
        if (bestRule->response == "__BLOCKED__") {
            int blocked = GetBlockedPacketCount();
            int ips = GetBlockedIPCount();
            return QString(
                "ตอนนี้ระบบบล็อกไปแล้วครับ\n\n"
                "- Blocked Packets: %1\n"
                "- Blocked IPs: %2\n\n"
                "ถ้าต้องการดูรายละเอียด ไปที่หน้า Management ได้เลยครับ"
            ).arg(blocked).arg(ips);
        }
        if (bestRule->response == "__ALERTS__") {
            int total = GetPacketCount() + GetBlockedPacketCount();
            int blocked = GetBlockedPacketCount();
            return QString(
                "มี Alert ในระบบตอนนี้ประมาณ %1 รายการครับ\n"
                "ส่วนใหญ่เป็น Critical จาก %2 packet ที่ถูกบล็อก\n\n"
                "ดูรายละเอียดได้ที่หน้า Alerts"
            ).arg(blocked).arg(blocked);
        }
        if (bestRule->response == "__HELP__") {
            return BuildHelpResponse();
        }

        return bestRule->response;
    }

    // ============ FALLBACK ============
    return BuildFallback(input);
}

// ================================================================
//  Keyword Score — Fuzzy Matching
// ================================================================
int Chatbot::KeywordScore(const QString &input, const QString &keyword) {
    QString k = keyword.toLower();

    // Exact match
    if (input.contains(k)) return 100;

    // Partial match (keyword ยาว)
    if (k.length() >= 4) {
        for (int i = 0; i <= k.length() - 4; i++) {
            if (input.contains(k.mid(i, 4))) return 60;
        }
    }

    // First char match
    if (k.length() >= 3 && input.contains(k.left(3))) return 30;

    return 0;
}

// ================================================================
//  Special Checks
// ================================================================
bool Chatbot::IsGreeting(const QString &s) {
    return s.contains("สวัสดี") || s.contains("หวัดดี") ||
           s == "hi" || s == "hello" || s == "hey" ||
           s.contains("ดีครับ") || s.contains("ดีค่ะ");
}

bool Chatbot::IsThanking(const QString &s) {
    return s.contains("ขอบคุณ") || s.contains("thank") ||
           s == "thanks" || s == "แต๊ง" || s == "โอเค";
}

bool Chatbot::IsFarewell(const QString &s) {
    return s.contains("บาย") || s == "bye" ||
           s.contains("goodbye") || s.contains("ลาก่อน");
}

bool Chatbot::IsAffirmative(const QString &s) {
    return s == "ใช่" || s == "yes" || s == "ok" ||
           s == "ได้" || s == "ครับ" || s == "ค่ะ";
}

bool Chatbot::IsNegative(const QString &s) {
    return s == "ไม่" || s == "no" || s == "ไม่ใช่";
}

// ================================================================
//  Fallback — ตอบฉลาดเมื่อไม่เข้าใจ
// ================================================================
QString Chatbot::BuildFallback(const QString &input) {
    // ถ้าเป็นคำถาม (มี ?)
    if (input.contains("?") || input.contains("ไหม") ||
        input.contains("ยังไง") || input.contains("อะไร")) {

        return PickRandom({
            "ขออภัยครับ ยังไม่เข้าใจคำถามนี้ดี\n"
            "ลองอธิบายเพิ่มเติมได้ไหมครับ?\n"
            "หรือพิมพ์ 'ช่วย' เพื่อดูหัวข้อที่ผมตอบได้",

            "คำถามน่าสนใจครับ แต่ผมยังไม่มีข้อมูลในส่วนนี้\n"
            "ลองถามในเรื่อง: สถานะ, วิธีใช้งาน, หรือหลักการ AI ดูไหมครับ",

            "ผมยังไม่แน่ใจในคำถามนี้ครับ\n"
            "ช่วยบอกให้ชัดขึ้นได้ไหม? หรือจะให้แนะนำหัวข้อก็ได้ครับ"
        });
    }

    // ถ้าเป็นคำสั่ง
    if (input.length() < 15) {
        return PickRandom({
            "ได้ยินครับ แต่ขอรายละเอียดเพิ่มเติมหน่อยครับ",

            "ผมยังไม่แน่ใจครับ ลองพิมพ์ให้ยาวขึ้นอีกนิดได้ไหม?",

            "ช่วยอธิบายเพิ่มอีกนิดนะครับ ผมจะได้ตอบให้ตรงจุด"
        });
    }

    // ทั่วไป
    return PickRandom({
        "ขอบคุณสำหรับข้อความครับ\n"
        "แต่ผมยังไม่เข้าใจ ลองพิมพ์ 'ช่วย' เพื่อดูหัวข้อที่ผมตอบได้นะครับ",

        "ผมยังไม่แน่ใจว่าจะช่วยเรื่องนี้ยังไงครับ\n"
        "ลองถามในหัวข้อ: สถานะ, วิธีใช้งาน, หรือหลักการ AI ดูไหมครับ",

        "ยังไม่เข้าใจครับ\n"
        "ถ้าต้องการดูว่าผมทำอะไรได้ พิมพ์ 'ช่วย' ได้เลยครับ"
    });
}

// ================================================================
//  Help Response
// ================================================================
QString Chatbot::BuildHelpResponse() {
    return "Raphael ช่วยได้หลายเรื่องครับ\n\n"
           "ข้อมูลระบบ:\n"
           "  - 'สถานะเป็นไง' ดูภาพรวม\n"
           "  - 'บล็อกไปกี่อัน' ดูจำนวน Blocked\n"
           "  - 'มี alert ไหม' ดูการแจ้งเตือน\n\n"
           "การใช้งาน:\n"
           "  - 'วิธีบล็อก IP'\n"
           "  - 'วิธีปลดบล็อก'\n"
           "  - 'วิธี export log'\n"
           "  - 'วิธีดู packet'\n\n"
           "หลักการ:\n"
           "  - 'AI ทำงานยังไง'\n"
           "  - 'Z-Score คืออะไร'\n"
           "  - 'Pre-filter ทำงานยังไง'\n\n"
           "ลองพิมพ์หัวข้อที่สนใจได้เลยครับ";
}

// ================================================================
//  Status Response
// ================================================================
QString Chatbot::BuildStatusResponse() {
    int total    = GetPacketCount() + GetBlockedPacketCount();
    int blocked  = GetBlockedPacketCount();
    int allowed  = GetPacketCount();
    int blCount  = PreFilterGetBlacklistCount();
    int threshold = PreFilterGetRateThreshold();

    return QString(
        "สถานะระบบปัจจุบันครับ\n\n"
        "การรับส่งข้อมูล:\n"
        "  - Total Packets: %1\n"
        "  - Allowed: %2\n"
        "  - Blocked: %3\n\n"
        "การตั้งค่า:\n"
        "  - Blacklist IPs: %4\n"
        "  - Rate Threshold: %5 pps\n\n"
        "ระบบทำงานปกติ ไม่พบปัญหาครับ"
    )
    .arg(total).arg(allowed).arg(blocked)
    .arg(blCount).arg(threshold);
}

// ================================================================
//  Pick Random
// ================================================================
QString Chatbot::PickRandom(const QStringList &options) {
    if (options.isEmpty()) return QString();
    int idx = QRandomGenerator::global()->bounded(options.size());
    return options[idx];
}

// ================================================================
//  Welcome
// ================================================================
QString Chatbot::GetWelcome() {
    return "สวัสดีครับ ผมชื่อ Raphael\n"
           "ผู้ช่วย AI ของระบบ Sentinel\n\n"
           "ผมช่วยเรื่องเหล่านี้ได้ครับ\n"
           "  - ดูสถานะระบบปัจจุบัน\n"
           "  - แนะนำวิธีการใช้งาน\n"
           "  - อธิบายหลักการทำงานของ AI\n\n"
           "ลองพิมพ์ 'ช่วย' เพื่อดูหัวข้อทั้งหมดได้เลยครับ";
}

// ================================================================
//  Response Delay — ตามความยาวข้อความ
// ================================================================
int Chatbot::GetResponseDelay(const QString &response) const {
    int len = response.length();

    // 20ms ต่อ 1 ตัวอักษร — ขั้นต่ำ 400ms, สูงสุด 1800ms
    int delay = 400 + len * 20;
    if (delay > 1800) delay = 1800;
    return delay;
}

void Chatbot::Reset() {
    LastCategory = "";
    LastUserMsg = "";
    ConversationDepth = 0;
}

// ================================================================
//  AIPage
// ================================================================
AIPage::AIPage(QWidget *parent) : QWidget(parent) {

    this->setStyleSheet(
        QString("AIPage { background-color: %1; }").arg(COLOR_BG)
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

    QLabel *TitleLabel = new QLabel("🤖  Sentinel AI Assistant", TitleBar);
    TitleLabel->setStyleSheet(
        QString("color: %1; font-size: 14px; font-weight: 600;"
                "background: transparent; border: none;").arg(COLOR_TEXT)
    );
    TitleLayout->addWidget(TitleLabel);
    TitleLayout->addStretch();

    StatusLabel = new QLabel("● Ready", TitleBar);
    StatusLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; font-weight: 600;"
                "background: transparent; border: none;").arg(COLOR_ACCENT)
    );
    TitleLayout->addWidget(StatusLabel);

    RootLayout->addWidget(TitleBar);

    // ============================================================
    //  Sphere (บน)
    // ============================================================
    Sphere = new AISphere(this);
    Sphere->setFixedHeight(300);
    RootLayout->addWidget(Sphere);

    // ============================================================
    //  Chat (ล่าง)
    // ============================================================
    QWidget *ChatContainer = new QWidget(this);
    ChatContainer->setStyleSheet(
        QString("background-color: %1;"
                "border-top: 1px solid %2;").arg(COLOR_BG, COLOR_BORDER)
    );
    QVBoxLayout *ChatLayout = new QVBoxLayout(ChatContainer);
    ChatLayout->setContentsMargins(16, 12, 16, 12);
    ChatLayout->setSpacing(8);

    // ---- ChatView ----
    ChatView = new QTextEdit(ChatContainer);
    ChatView->setReadOnly(true);
    ChatView->setStyleSheet(
        QString("QTextEdit {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-family: 'Segoe UI';"
                "  font-size: 12px;"
                "  border: 1px solid %3;"
                "  border-radius: 6px;"
                "  padding: 10px;"
                "}"
                "QScrollBar:vertical {"
                "  background: %1; width: 8px; border: none;"
                "}"
                "QScrollBar::handle:vertical {"
                "  background: #3e3e42; border-radius: 4px; min-height: 20px;"
                "}"
                "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
                "  height: 0px;"
                "}").arg(COLOR_PANEL, COLOR_TEXT, COLOR_BORDER)
    );
    ChatLayout->addWidget(ChatView, 1);

    // ---- Input Row ----
    QHBoxLayout *InputRow = new QHBoxLayout();
    InputRow->setSpacing(8);

    Input = new QLineEdit(ChatContainer);
    Input->setPlaceholderText("พิมพ์คำถาม...");
    Input->setStyleSheet(
        QString("QLineEdit {"
                "  background-color: %1;"
                "  color: %2;"
                "  font-size: 12px;"
                "  border: 1px solid %3;"
                "  border-radius: 6px;"
                "  padding: 8px 12px;"
                "}"
                "QLineEdit:focus { border: 1px solid %4; }")
            .arg(COLOR_PANEL, COLOR_TEXT, COLOR_BORDER, COLOR_ACCENT)
    );
    InputRow->addWidget(Input, 1);

    SendBtn = new QPushButton("Send ➤", ChatContainer);
    SendBtn->setCursor(Qt::PointingHandCursor);
    SendBtn->setFixedSize(90, 36);
    SendBtn->setStyleSheet(
        QString("QPushButton {"
                "  background-color: #2a3a35;"
                "  color: %1;"
                "  font-size: 12px;"
                "  font-weight: 600;"
                "  border: 1px solid %1;"
                "  border-radius: 6px;"
                "}"
                "QPushButton:hover { background-color: #3a4a45; }")
            .arg(COLOR_ACCENT)
    );
    InputRow->addWidget(SendBtn);

    ChatLayout->addLayout(InputRow);
    RootLayout->addWidget(ChatContainer, 1);

    // ============================================================
    //  Welcome
    // ============================================================
    AppendMessage("AI", Bot.GetWelcome(), true);

    // ============================================================
    //  Connect
    // ============================================================
    connect(SendBtn, &QPushButton::clicked, this, &AIPage::OnSend);
    connect(Input,   &QLineEdit::returnPressed, this, &AIPage::OnSend);

    SpeakingTimer = new QTimer(this);
    SpeakingTimer->setSingleShot(true);
    connect(SpeakingTimer, &QTimer::timeout, this, &AIPage::OnSpeakingTimeout);
}

AIPage::~AIPage() {}

void AIPage::AppendMessage(const QString &from, const QString &msg, bool isAI) {
    QString time = QDateTime::currentDateTime().toString("hh:mm");

    QString html;
    if (isAI) {
        html = QString(
            "<div style='margin-bottom:12px;'>"
            "  <span style='color:%1; font-weight:bold;'>Raphael</span>"
            "  <span style='color:%2; font-size:10px;'>  %3</span><br>"
            "  <span style='color:%4;'>%5</span>"
            "</div>")
            .arg(COLOR_ACCENT, COLOR_DIM, time, COLOR_TEXT,
                 msg.toHtmlEscaped().replace("\n", "<br>"));
    } else {
        html = QString(
            "<div style='margin-bottom:12px; text-align:right;'>"
            "  <span style='color:%1; font-size:10px;'>%2  </span>"
            "  <span style='color:%3; font-weight:bold;'>You</span><br>"
            "  <span style='color:%4;'>%5</span>"
            "</div>")
            .arg(COLOR_DIM, time, COLOR_INFO, COLOR_TEXT,
                 msg.toHtmlEscaped().replace("\n", "<br>"));
    }

    ChatView->append(html);

    QScrollBar *bar = ChatView->verticalScrollBar();
    bar->setValue(bar->maximum());
}

void AIPage::OnSend() {
    QString text = Input->text().trimmed();
    if (text.isEmpty()) return;

    AppendMessage("You", text, false);
    Input->clear();

    // ---- Speaking Effect ----
    Sphere->SetSpeaking(true);
    StatusLabel->setText("● กำลังพิมพ์...");
    StatusLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; font-weight: 600;"
                "background: transparent; border: none;").arg(COLOR_INFO)
    );

    // ---- Response + Delay ตามความยาว ----
    QString response = Bot.Respond(text);
    int delay = Bot.GetResponseDelay(response);

    SpeakingTimer->start(delay);

    QTimer::singleShot(delay, this, [this, response]() {
        AppendMessage("AI", response, true);
    });
}

void AIPage::OnSpeakingTimeout() {
    Sphere->SetSpeaking(false);
    StatusLabel->setText("● Ready");
    StatusLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; font-weight: 600;"
                "background: transparent; border: none;").arg(COLOR_ACCENT)
    );
}