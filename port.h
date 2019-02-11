#ifndef PORT_H
#define PORT_H

#include <QObject>

class Port : public QObject
{
    Q_OBJECT
public:
    explicit Port(QObject *parent = nullptr);

signals:

public slots:
};

#endif // PORT_H