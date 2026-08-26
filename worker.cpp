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


void Worker::cancel()
{
    cancelRequested.store(true);
    pauseCondition.wakeAll();
}


void Worker::waitIfPaused()
{
    QMutexLocker locker(&pauseMutex);

    while (pauseRequested && !cancelRequested.load())
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
    cancelRequested.store(false);

    // подсчёт кол-ва файлов для прогресс бара
    QDirIterator fileCountIterator (
        s.inputPath,
        QStringList() << s.mask,
        QDir::Files
    ); // QDirIterator::Subdirectories для рекурсии

    int cnt = 0;
    while (fileCountIterator.hasNext())
    {
        fileCountIterator.next();
        cnt++;
    }

    emit filesCount(cnt);

    QString outputFilePath = QDir(s.savePath).filePath(s.saveName + ".bin");

    QByteArray key = QByteArray::fromHex(
        s.XOR_key.toLatin1()
    );

    if (key.size() != 8)
    {
        qDebug() << "Некорректный XOR-ключ";
        return;
    }

    QDirIterator fileIterator (
        s.inputPath,
        QStringList() << s.mask,
        QDir::Files
    ); // QDirIterator::Subdirectories для рекурсии

    while (fileIterator.hasNext())
    {
        QString inputPath = fileIterator.next();

        QFileInfo fileInfo(inputPath);
        qint64 size = fileInfo.size();

        if (s.duplicateAction == DuplicateAction::Counter)
        {
            outputFilePath = getUniqueFileName(s.savePath, s.saveName + ".bin");
        }

        bool processed_status = processFile(inputPath, outputFilePath,
                                            key, s.deleteInputFile, size);

        if (!processed_status)
            return;

        emit fileProcessed();
    }

    qDebug() << "Все файлы обработаны";

    emit completeProcessing(s);
}


bool Worker::processFile(const QString &inputPath,
                const QString &outputPath,
                const QByteArray &key,
                bool deleteInputFile,
                const quint64 &file_size)
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

    constexpr qsizetype blockSize = 4 * 1024 * 1024; // про qsizetype подробнее и constexpr

    while (true)
    {
        QByteArray block = input.read(blockSize);

        if (block.isEmpty())
        {
            if (input.atEnd())
                break;

            qDebug() << "Ошибка чтения:"
                     << input.errorString();;
            return false;
        }

        waitIfPaused();

        if (cancelRequested.load())
            return false;

        for (qsizetype i = 0; i < block.size(); ++i)
        {
            block[i] ^= key[i % key.size()];
        }

        quint64 written = output.write(block);
        if (written != block.size())
        {
            qDebug() << "Ошибка записи:"
                     << output.errorString();

            return false;
        }

        emit progress(input.pos(), file_size);
    }

    input.close();
    output.close();

    if (deleteInputFile)
    {
        if (!QFile::remove(inputPath))
        {
            qDebug() << "Не удалось удалить исходный файл:"
                     << inputPath;

            return false;
        }
    }

    qDebug() << "Файл " << outputPath <<  " обработан";
    return true;
}
