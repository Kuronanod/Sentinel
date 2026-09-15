#ifndef PACKETPAGE_H
#define PACKETPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QTimer>
#include <QLabel>
#include <QLineEdit>

class PacketPage : public QWidget {
    Q_OBJECT

public:
    explicit PacketPage(QWidget *parent = nullptr);
    ~PacketPage();

private slots:
    void UpdateTable();
    void OnSearchChanged(const QString &text);

private:
    QTableWidget *PacketTable;
    QTimer       *UpdateTimer;
    QLabel       *StatsLabel;
    QLineEdit    *SearchInput;

    static const int MaximumRows = 500;

    void AddPacketRow(const QString &src, const QString &dst,
                      int sport, int dport, const QString &proto, int len);
};

#endif