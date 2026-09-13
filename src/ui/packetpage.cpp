#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QDebug>

#include <winsock2.h>

#include "packetpage.h"
#include "receiver/request_queue.h"

static QString IPToString(unsigned int IP) {
    return QString("%1.%2.%3.%4")
        .arg(IP & 0xFF)
        .arg((IP >> 8) & 0xFF)
        .arg((IP >> 16) & 0xFF)
        .arg((IP >> 24) & 0xFF);
}

PacketPage::PacketPage(QWidget *parent):QWidget(parent){

    QVBoxLayout *Layout = new QVBoxLayout(this);
    
    Layout->setContentsMargins(10, 10, 10, 10);

    PacketTable = new QTableWidget(this);
    PacketTable->setColumnCount(6);
    PacketTable->setHorizontalHeaderLabels({"Source IP", "Destination IP", "Source Port", "Destination Port", "Protocol", "Length"});

    PacketTable->horizontalHeader()->setStretchLastSection(true);
    PacketTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    PacketTable->verticalHeader()->setVisible(false);
    PacketTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    PacketTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    PacketTable->setAlternatingRowColors(true);

    PacketTable->setStyleSheet(
        "QTableWidget {"
        "  background-color: #1a1a1a;"
        "  color: #e0e0e0;"
        "  gridline-color: #333;"
        "  font-family: Consolas;"
        "  font-size: 12px;"
        "}"
        "QHeaderView::section {"
        "  background-color: #2c3e50;"
        "  color: white;"
        "  padding: 6px;"
        "  border: none;"
        "}"
        "QTableWidget::item:alternate {"
        "  background-color: #222;"
        "}"
    );

    Layout->addWidget(PacketTable);

    UpdateTimer = new QTimer(this);
    connect(UpdateTimer, &QTimer::timeout, this, &PacketPage::UpdateTable);
    UpdateTimer->start(200);
}

PacketPage::~PacketPage() {}

void PacketPage::UpdateTable() {

    qDebug() << "UpdateTable called, rows:" << PacketTable->rowCount()
             << "queue:" << GetPacketQueueSize();

    if (PacketTable->rowCount() > MaximumRows) {
        PacketTable->setRowCount(MaximumRows);
    }

    PacketRecord Records;
    int RecordsAdded = 0;

    while (PopPacket(&Records)) {

        PacketTable->insertRow(0);

        QString SourceIP = IPToString(Records.SourceIP);
        QString DestinationIP = IPToString(Records.DestinationIP);

        QString Protocol;
        if (Records.Protocol == 6){
             Protocol = "TCP";
        }
        else if (Records.Protocol == 17){
            Protocol = "UDP";
        }else if (Records.Protocol == 1){
            Protocol = "ICMP";
        }
        else{
            Protocol = QString::number(Records.Protocol);
        }

        PacketTable->setItem(0, 0, new QTableWidgetItem(SourceIP));
        PacketTable->setItem(0, 1, new QTableWidgetItem(DestinationIP));
        PacketTable->setItem(0, 2, new QTableWidgetItem(QString::number(Records.SourcePort)));
        PacketTable->setItem(0, 3, new QTableWidgetItem(QString::number(Records.DestinationPort)));
        PacketTable->setItem(0, 4, new QTableWidgetItem(Protocol));
        PacketTable->setItem(0, 5, new QTableWidgetItem(QString::number(Records.Length)));

        RecordsAdded++;

        if (PacketTable->rowCount() > MaximumRows) {
            PacketTable->removeRow(PacketTable->rowCount() - 1);
        }
    }

    if (RecordsAdded > 0) {
        PacketTable->scrollToTop();
    }

};