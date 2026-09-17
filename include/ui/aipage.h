#ifndef AIPAGE_H
#define AIPAGE_H

#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QColor>
#include <QString>
#include <QStringList>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

// ================================================================
//  AISphere — Particle Sphere Widget
// ================================================================
class AISphere : public QWidget {
    Q_OBJECT

public:
    explicit AISphere(QWidget *parent = nullptr);
    ~AISphere();

    void SetSpeaking(bool speaking);
    bool IsSpeaking() const { return Speaking; }

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void OnTick();

private:
    struct Point3D {
        float  x, y, z;
        QColor color;
        int    size;
    };

    QVector<Point3D> Points;
    QTimer          *Timer;
    float            Angle;
    bool             Speaking;
    float            PulsePhase;

    void    InitPoints(int count);
    QPointF Project(const Point3D &p, float &outDepth, float &outScale);
    float   Distance3D(const Point3D &a, const Point3D &b);
};

class Chatbot {
public:
    Chatbot();

    QString Respond(const QString &input);
    QString GetWelcome();
    void    Reset();

    int     GetResponseDelay(const QString &response) const;

private:
    struct Rule {
        QStringList keywords;
        QString     response;
        QString     category;
        int         priority;
    };

    QVector<Rule> Rules;

    // ---- Context ----
    QString LastCategory;
    QString LastUserMsg;
    QString LastTopic;
    int     ConversationDepth;
    int     MissStreak;         // นับครั้งที่ตอบไม่ได้ติดกัน

    // ---- Helpers ----
    void    InitRules();
    QString BuildStatusResponse();
    QString BuildBlockedResponse();
    QString BuildAlertsResponse();
    QString BuildHelpResponse();
    QString BuildAboutResponse();
    QString BuildFallback(const QString &input);
    QString BuildMissResponse(const QString &input);
    QString PickRandom(const QStringList &options);

    // ---- Normalize / Match ----
    QString Normalize(const QString &input);
    int     KeywordScore(const QString &input, const QString &keyword);
    bool    IsGreeting(const QString &s);
    bool    IsThanking(const QString &s);
    bool    IsFarewell(const QString &s);
    bool    IsAboutSelf(const QString &s);
    bool    IsCapabilityQuestion(const QString &s);
    bool    IsMetaQuestion(const QString &s);
    bool    IsYesNoQuestion(const QString &s);
    QString GetYesNoTarget(const QString &s);
};

// ================================================================
//  AIPage — Main Page (Sphere + Chat)
// ================================================================
class AIPage : public QWidget {
    Q_OBJECT

public:
    explicit AIPage(QWidget *parent = nullptr);
    ~AIPage();

private slots:
    void OnSend();
    void OnSpeakingTimeout();

private:
    AISphere    *Sphere;
    QTextEdit   *ChatView;
    QLineEdit   *Input;
    QPushButton *SendBtn;
    QLabel      *StatusLabel;

    Chatbot      Bot;
    QTimer      *SpeakingTimer;

    void AppendMessage(const QString &from, const QString &msg, bool isAI);
};

#endif