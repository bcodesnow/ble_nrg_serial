#ifndef CATCHCONTROLLER_H
#define CATCHCONTROLLER_H

#include <QObject>

class CatchController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(quint8 appState MEMBER m_appState NOTIFY appStateChanged)
    Q_PROPERTY(bool sdEnabled READ sdEnabled NOTIFY sdEnabledChanged)

    //Q_PROPERTY(QString lastPath MEMBER m_last_path NOTIFY lastPathChanged)

private:
    quint8 m_appState;
    bool m_sdEnabled;

public:
    explicit CatchController(QObject *parent = nullptr);
    bool sdEnabled() const;


signals:
    void appStateChanged (quint8 state);
    void sdEnabledChanged();

public slots:
};

#endif // CATCHCONTROLLER_H
