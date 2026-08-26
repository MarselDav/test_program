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
    explicit Worker(QObject *parent = nullptr);


    // Обработка одного файла блоками
    bool processFile(
        const QString &inputPath,
        const QString &outputPath,
        const QByteArray &key,
        bool deleteInputFile,
        const quint64 &file_size
        );


    // Управление паузой обработки
    void pause();
    void resume();
    void waitIfPaused();

    // Запрос на завершение обработки
    void cancel();


private:
    // Генерация уникального имени выходного файла
    QString getUniqueFileName(
        const QString &directory,
        const QString &fileName
        );


    // Синхронизация доступа к состоянию паузы
    QMutex pauseMutex;
    QWaitCondition pauseCondition;

    // Флаг запроса на паузу
    bool pauseRequested;

    // Атомарный флаг запроса на завершение обработки
    std::atomic_bool cancelRequested{false};


public slots:
    // Запуск обработки файлов с заданными настройками
    void startProcessing(const Settings &s);


signals:
    // Передача количества найденных файлов
    void filesCount(int cnt);

    // Сигнал после завершения обработки одного файла
    void fileProcessed();

    // Передача текущего прогресса обработки файла
    void progress(
        const quint64& pos,
        const quint64& size
        );

    // Сигнал полного завершения текущего запуска
    void completeProcessing(const Settings& s);
};


#endif // WORKER_H
