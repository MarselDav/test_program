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
        bool deleteInputFile,
        const quint64 &file_size);

    void pause();
    void resume();
    void waitIfPaused();

    void cancel();

private:
    QString getUniqueFileName(const QString &directory,
                              const QString &fileName);

    QMutex pauseMutex;
    QWaitCondition pauseCondition;
    bool pauseRequested;

    std::atomic_bool cancelRequested{false};

public slots:
    void startProcessing(const Settings &s);

signals:
    void filesCount(int cnt);
    void fileProcessed();
    void progress(const quint64& pos, const quint64& size);
    void completeProcessing(const Settings& s);
};

#endif // WORKER_H
