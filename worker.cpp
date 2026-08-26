#include "worker.h"

Worker::Worker(QObject *parent)
    : QObject{parent}
{
    pauseRequested = false;
}


// Установка флага паузы
void Worker::pause()
{
    QMutexLocker locker(&pauseMutex);

    pauseRequested = true;
}


// Снятие паузы и пробуждение Worker
void Worker::resume()
{
    {
        QMutexLocker locker(&pauseMutex);

        pauseRequested = false;
    } // mutex освобождается при выходе из блока

    pauseCondition.wakeAll();
}


// Запрос на завершение обработки
void Worker::cancel()
{
    cancelRequested.store(true);

    // Пробуждаем Worker, если он находится на паузе
    pauseCondition.wakeAll();
}


// Ожидание снятия паузы
void Worker::waitIfPaused()
{
    QMutexLocker locker(&pauseMutex);

    while (pauseRequested && !cancelRequested.load())
    {
        pauseCondition.wait(&pauseMutex);
    }
}


// Получение уникального имени выходного файла
QString Worker::getUniqueFileName(
    const QString &directory,
    const QString &fileName)
{
    QFileInfo info(directory, fileName);

    // Если файл не существует, используем исходное имя
    if (!info.exists())
        return info.filePath();


    QString baseName = info.completeBaseName();
    QString suffix = info.suffix();

    int counter = 1;

    // Поиск свободного имени с добавлением счётчика
    while (true)
    {
        QString newName;

        if (suffix.isEmpty())
            newName = QString("%1%2")
                          .arg(baseName)
                          .arg(counter);
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


// Запуск обработки всех подходящих файлов
void Worker::startProcessing(const Settings &s)
{
    cancelRequested.store(false);

    // Подсчёт количества входных файлов
    QDirIterator fileCountIterator(
        s.inputPath,
        QStringList() << s.mask,
        QDir::Files,
        QDirIterator::Subdirectories
        );

    int cnt = 0;

    while (fileCountIterator.hasNext())
    {
        fileCountIterator.next();
        cnt++;
    }

    emit filesCount(cnt);

    QString outputFilePath =
        QDir(s.savePath).filePath(s.saveName + ".bin");

    QByteArray key = QByteArray::fromHex(
        s.XOR_key.toLatin1()
        );

    if (key.size() != 8)
    {
        qDebug() << "Некорректный XOR-ключ";
        return;
    }


    // Поиск входных файлов
    QDirIterator fileIterator(
        s.inputPath,
        QStringList() << s.mask,
        QDir::Files,
        QDirIterator::Subdirectories
        );


    while (fileIterator.hasNext())
    {
        QString inputPath = fileIterator.next();

        QFileInfo fileInfo(inputPath);
        qint64 size = fileInfo.size();


        // Создание уникального имени при необходимости
        if (s.duplicateAction == DuplicateAction::Counter)
        {
            outputFilePath =
                getUniqueFileName(
                    s.savePath,
                    s.saveName + ".bin"
                    );
        }


        // Обработка текущего файла
        bool processed_status =
            processFile(
                inputPath,
                outputFilePath,
                key,
                s.deleteInputFile,
                size
                );


        // Завершение обработки при ошибке или отмене
        if (!processed_status)
            return;


        emit fileProcessed();
    }

    qDebug() << "Все файлы обработаны";
    emit completeProcessing(s);
}


// Обработка одного файла блоками
bool Worker::processFile(
    const QString &inputPath,
    const QString &outputPath,
    const QByteArray &key,
    bool deleteInputFile,
    const quint64 &file_size)
{
    qDebug() << "Обработка файла" << inputPath;


    QFile input(inputPath);
    QFile output(outputPath);


    // Открытие входного файла
    if (!input.open(QIODevice::ReadOnly))
    {
        qDebug() << input.errorString();
        return false;
    }


    // Открытие выходного файла
    if (!output.open(QIODevice::WriteOnly))
    {
        qDebug() << output.errorString();
        return false;
    }

    // Размер одного блока обработки
    const qsizetype blockSize = 4 * 1024 * 1024;

    // Обработка файла блоками
    while (true)
    {
        QByteArray block = input.read(blockSize);


        // Проверка конца файла и ошибки чтения
        if (block.isEmpty())
        {
            if (input.atEnd())
                break;

            qDebug() << "Ошибка чтения:"
                     << input.errorString();

            return false;
        }

        waitIfPaused();

        // Проверка запроса на завершение
        if (cancelRequested.load())
            return false;


        // XOR каждого байта с циклическим ключом
        for (qsizetype i = 0; i < block.size(); ++i)
        {
            block[i] ^= key[i % key.size()];
        }


        // Запись обработанного блока
        quint64 written = output.write(block);

        if (written != block.size())
        {
            qDebug() << "Ошибка записи:"
                     << output.errorString();

            return false;
        }


        // Передача текущего прогресса
        emit progress(input.pos(), file_size);
    }


    // Закрытие файлов
    input.close();
    output.close();

    // Удаление исходного файла после успешной обработки
    if (deleteInputFile)
    {
        if (!QFile::remove(inputPath))
        {
            qDebug() << "Не удалось удалить исходный файл:"
                     << inputPath;

            return false;
        }
    }

    qDebug() << "Файл " << inputPath << " обработан";

    return true;
}
