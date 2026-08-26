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

    bool processFile(const QString &inputPath,
        const QString &outputPath,
        const QByteArray &key,
        const bool &deleteInputFile);

    void pause();
    void resume();
    void waitIfPaused();

    void shutdown();

private:
    QString getUniqueFileName(const QString &directory,
                              const QString &fileName);

    QMutex pauseMutex;
    QWaitCondition pauseCondition;
    bool pauseRequested;

    std::atomic_bool shutdownRequested{false};

public slots:
    void startProcessing(const Settings &s);

signals:
    void filesCount(int cnt);
    void fileSize(int size);
    void fileProcessed();
    void oneBlockProcessed(int step);
    void completeProcessing();
};

#endif // WORKER_H
