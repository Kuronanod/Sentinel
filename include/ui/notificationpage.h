#ifndef NOTIFICATIONPAGE_H
#define NOTIFICATIONPAGE_H

#include <QWidget>
#include <QVector>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QTimer>

struct NotifItem {
    int     type;       // 0=alert, 1=ai, 2=system
    QString title;
    QString message;
    QString time;
    bool    isNew;
};

class NotificationPage : public QWidget {
    Q_OBJECT

public:
    explicit NotificationPage(QWidget *parent = nullptr);
    ~NotificationPage();

    void AddNotification(int type,
                         const QString &title,
                         const QString &message,
                         const QString &time);

private slots:
    void OnMarkAllRead();
    void OnClearAll();
    void PollNotifications();

private:
    QVBoxLayout *ListLayout;
    QLabel      *HeaderLabel;
    QLabel      *EmptyLabel;
    QLabel      *CountLabel;

    QVector<NotifItem> Items;
    static const int   MAX_ITEMS = 50;

    QWidget* BuildItemWidget(const NotifItem &item);
    void RebuildList();
};

#endif