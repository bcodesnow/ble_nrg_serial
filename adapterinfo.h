#ifndef ADAPTERINFO_H
#define ADAPTERINFO_H

// UNUSED RIGHT NOW, and we also do not need it anymore ase we use all hci interfaces available

#include <QObject>
#include <QBluetoothAddress>

class AdapterInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString adapterName READ getName NOTIFY adapterChanged)
    Q_PROPERTY(QString adapterAddress READ getAddressStr NOTIFY adapterChanged)
private:
    QString m_name;
    QBluetoothAddress m_addr;
public:
    AdapterInfo(const QString name, const QBluetoothAddress addr);
    QString getName();
    QBluetoothAddress getAddress();
    QString getAddressStr();

signals:
    void adapterChanged();

public slots:
};

#endif // ADAPTERINFO_H
