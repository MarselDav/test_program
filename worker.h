#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QDebug>
#include <QDirIterator>
#include <atomic>
#include "settings.h"

#include <QMutex>
#include <QWaitCondition>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr); // вспомнить что такое explicit

    void processFile(const QString &inputPath,
        const QString &outputPath,
        const QByteArray &key);

    void pause();
    void resume();
    void waitIfPaused();

private:
    QMutex pauseMutex;
    QWaitCondition pauseCondition;
    bool pauseRequested;

public slots:
    void startProcessing(const Settings &s);

signals:
    void filesCount(int cnt);
    void fileSize(int size);
    void fileProcessed();
    void oneBlockProcessed(int step);
};

#endif // WORKER_H
