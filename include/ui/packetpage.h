#ifndef PACKETPAGE_H
#define PACKETPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QTimer>

class PacketPage : public  QWidget{

    Q_OBJECT

    public:
        explicit PacketPage(QWidget *parent = nullptr);
        ~PacketPage();

    private slots:
        void UpdateTable();

    private:
        QTableWidget *PacketTable;
        QTimer *UpdateTimer;

        static const int MaximumRows = 500;

};

#endif