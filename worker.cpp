#include "worker.h"

Worker::Worker(QObject *parent)
    : QObject{parent}
{
    pauseRequested = false;
}

void Worker::pause()
{
    QMutexLocker locker(&pauseMutex);
    pauseRequested = true;
}

void Worker::resume()
{
    {
        QMutexLocker locker(&pauseMutex);
        pauseRequested = false;
    } // чтобы locker.unlock()

    pauseCondition.wakeAll();
}


void Worker::shutdown()
{
    shutdownRequested.store(true);
    pauseCondition.wakeAll();
}


void Worker::waitIfPaused()
{
    QMutexLocker locker(&pauseMutex);

    while (pauseRequested && !shutdownRequested.load())
    {
        pauseCondition.wait(&pauseMutex);
    }
}

QString Worker::getUniqueFileName(const QString &directory,
                          const QString &fileName)
{
    QFileInfo info(directory, fileName);

    if (!info.exists())
        return info.filePath();

    QString baseName = info.completeBaseName();
    QString suffix = info.suffix();

    int counter = 1;

    while (true)
    {
        QString newName;

        if (suffix.isEmpty())
            newName = QString("%1%2").arg(baseName).arg(counter);
        else
            newName = QString("%1%2.%3")
                          .arg(baseName)
                          .arg(counter)
                          .arg(suffix);

        QString newPath = QDir(directory).filePath(newName);

        if (!QFile::exists(newPath))
            return newPath;

        counter++;
    }
}

void Worker::startProcessing(const Settings &s)
{
    qDebug() << "--------------";
    qDebug() << s.mask;
    qDebug() << s.deleteInputFile;
    qDebug() << s.savePath;
    qDebug() << s.saveName;
    qDebug() << s.inputPath;
    qDebug() << s.duplicateAction;
    qDebug() << s.workMode;
    qDebug() << s.seconds;
    qDebug() << s.XOR_key;
    qDebug() << "--------------";

    shutdownRequested.store(false);

    // подсчёт кол-ва файлов для прогресс бара
    QDirIterator it1 (
        s.inputPath,
        QStringList() << s.mask,
        QDir::Files
    ); // QDirIterator::Subdirectories для рекурсии

    int cnt = 0;
    while (it1.hasNext())
    {
        it1.next();
        cnt++;
    }

    qDebug() << "Кол-во файлов: " << cnt;

    emit filesCount(cnt);

    QString outputFilePath = QDir(s.savePath).filePath(s.saveName + ".bin");

    QByteArray key = QByteArray::fromHex(
        s.XOR_key.toLatin1()
    );

    qDebug() << key;

    QDirIterator it (
        s.inputPath,
        QStringList() << s.mask,
        QDir::Files
    ); // QDirIterator::Subdirectories для рекурсии

    while (it.hasNext())
    {
        QString inputPath = it.next();

        QFileInfo fileInfo(inputPath);
        qint64 size = fileInfo.size();

        emit fileSize(size);

        qDebug() << inputPath;
        qDebug() << outputFilePath;

        if (s.duplicateAction == onDuplicateInputFileAction::Counter)
        {
            outputFilePath = getUniqueFileName(s.savePath, s.saveName + ".bin");
        }

        bool processed_status = processFile(inputPath, outputFilePath, key);

        if (!processed_status)
            return;

        emit fileProcessed();
    }

    qDebug() << "Файлы обработаны";

    emit completeProcessing();
}


bool Worker::processFile(const QString &inputPath,
                 const QString &outputPath,
                 const QByteArray &key)
{
    qDebug() << "Обработка файла" << inputPath;

    QFile input(inputPath);
    QFile output(outputPath);

    if (!input.open(QIODevice::ReadOnly))
    {
        qDebug() << input.errorString();
        return false;
    }

    if (!output.open(QIODevice::WriteOnly))
    {
        qDebug() << output.errorString();
        return false;
    }

    constexpr qsizetype blockSize = 4096; // про qsizetype подробнее и constexpr

    while (!input.atEnd())
    {
        waitIfPaused();

        if (shutdownRequested.load())
            return false;

        QByteArray block = input.read(blockSize);

        for (qsizetype i = 0; i < block.size(); ++i)
        {
            block[i] ^= key[i % key.size()];
        }

        output.write(block);

        emit oneBlockProcessed(blockSize);
    }

    qDebug() << "Файл обработан";
    return true;
}
