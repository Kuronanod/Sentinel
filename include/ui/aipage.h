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

    // ---- Delay simulation ----
    int     GetResponseDelay(const QString &response) const;

private:
    struct Rule {
        QStringList keywords;
        QString     response;
        QString     category;   // สำหรับ context
    };

    QVector<Rule> Rules;

    // ---- Context Memory ----
    QString LastCategory;
    QString LastUserMsg;
    int     ConversationDepth;

    // ---- Helpers ----
    void    InitRules();
    QString BuildStatusResponse();
    QString BuildHelpResponse();
    QString PickRandom(const QStringList &options);
    QString BuildFallback(const QString &input);

    // ---- Fuzzy Match ----
    int  KeywordScore(const QString &input, const QString &keyword);
    bool IsGreeting(const QString &input);
    bool IsThanking(const QString &input);
    bool IsFarewell(const QString &input);
    bool IsAffirmative(const QString &input);
    bool IsNegative(const QString &input);
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