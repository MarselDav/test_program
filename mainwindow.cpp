#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Инициализация настроек и таймера повторного запуска
    m_settings = Settings();

    m_processingTimer = new QTimer(this);
    m_processingTimer->setSingleShot(true);

    // Запуск обработки после срабатывания таймера
    connect(
        m_processingTimer,
        &QTimer::timeout,
        this,
        &MainWindow::startProcessing
        );


    // Создание центрального виджета и основного layout
    QWidget *central = new QWidget;
    main_vlayout = new QVBoxLayout;

    // Начальное состояние программы
    m_isActive = false;
    m_isPaused = false;

    // Настройка Worker и элементов интерфейса
    setupWorker();
    setupMaskGroup();
    setupDeleteCheckBox();
    setupSavePathGroup();
    setupInputPathGroup();
    setupDuplicationGroup();
    setupModeGroup();
    setupHexValueGroup();

    // Кнопка запуска/паузы обработки
    startButton = new QPushButton("Начать выполнение");

    connect(
        startButton,
        &QPushButton::clicked,
        this,
        &MainWindow::on_startButton_clicked
        );

    main_vlayout->addWidget(startButton);


    // Кнопка завершения обработки
    cancelButton = new QPushButton("Завершить");

    connect(
        cancelButton,
        &QPushButton::clicked,
        this,
        &MainWindow::on_cancelButton_clicked
        );

    cancelButton->setStyleSheet("background-color: rgb(245, 85, 73)");
    cancelButton->setVisible(false);

    main_vlayout->addWidget(cancelButton);


    // Прогресс обработки количества файлов
    file_progress_bar = new QProgressBar;
    file_progress_bar->setFormat("%v/%m");
    file_progress_bar->setVisible(false);

    main_vlayout->addWidget(file_progress_bar);


    // Прогресс обработки текущего файла
    processing_progress_bar = new QProgressBar;
    processing_progress_bar->setRange(0, 100);
    processing_progress_bar->setFormat("%p%");
    processing_progress_bar->setVisible(false);

    main_vlayout->addWidget(processing_progress_bar);


    // Вывод текущего состояния программы
    status_label = new QLabel;
    status_label->setAlignment(Qt::AlignCenter);
    status_label->setVisible(false);

    main_vlayout->addWidget(status_label);


    // Установка layout центрального виджета
    central->setLayout(main_vlayout);
    setCentralWidget(central);
}


// Настройка Worker и отдельного потока
void MainWindow::setupWorker()
{
    m_thread = new QThread(this);
    m_worker = new Worker();

    // Перенос Worker в отдельный поток
    m_worker->moveToThread(m_thread);


    // Запуск обработки по сигналу из MainWindow
    connect(
        this,
        &MainWindow::startWork,
        m_worker,
        &Worker::startProcessing
        );


    // Получение результатов обработки от Worker
    connect(
        m_worker,
        &Worker::filesCount,
        this,
        &MainWindow::setFilesCount
        );

    connect(
        m_worker,
        &Worker::fileProcessed,
        this,
        &MainWindow::fileProcessed
        );

    connect(
        m_worker,
        &Worker::progress,
        this,
        &MainWindow::progress
        );

    connect(
        m_worker,
        &Worker::completeProcessing,
        this,
        &MainWindow::completeProcessing
        );


    // Запуск потока
    m_thread->start();
}


// Настройка маски входных файлов
void MainWindow::setupMaskGroup()
{
    QGroupBox *mask_group = new QGroupBox("Маска файлов");
    QHBoxLayout *mask_layout = new QHBoxLayout(mask_group);

    mask_layout->addWidget(new QLabel("Маска:"));

    input_file_mask = new QComboBox;
    input_file_mask->setEditable(true);

    // Доступные маски по умолчанию
    input_file_mask->addItems({"*.txt", "*.bin", "*.*"});

    mask_layout->addWidget(input_file_mask);

    main_vlayout->addWidget(mask_group);
}


// Настройка удаления входных файлов
void MainWindow::setupDeleteCheckBox()
{
    delete_input_file = new QCheckBox(
        "Удалять исходный файл после обработки"
        );

    main_vlayout->addWidget(delete_input_file);
}


// Настройка пути и имени выходного файла
void MainWindow::setupSavePathGroup()
{
    QGroupBox *save_group =
        new QGroupBox("Путь для сохранения результатов");

    save_path_hlayout = new QHBoxLayout(save_group);

    save_path_lineedit = new QLineEdit("D://test_files");
    save_name_lineedit = new QLineEdit("test_file");

    QLabel *separator = new QLabel("/");

    save_name_lineedit->setPlaceholderText(
        "Имя выходного файла"
        );

    // Путь задаётся через диалог выбора директории
    save_path_lineedit->setReadOnly(true);

    discover_save_directory = new QPushButton("Обзор...");

    connect(
        discover_save_directory,
        &QPushButton::clicked,
        this,
        &MainWindow::on_selectSaveDirectoryButton_clicked
        );


    save_path_hlayout->addWidget(save_path_lineedit);
    save_path_hlayout->addWidget(separator);
    save_path_hlayout->addWidget(save_name_lineedit);
    save_path_hlayout->addWidget(discover_save_directory);

    main_vlayout->addWidget(save_group);
}


// Настройка пути поиска входных файлов
void MainWindow::setupInputPathGroup()
{
    QGroupBox *input_group =
        new QGroupBox("Путь для поиска файлов");

    input_path_hlayout = new QHBoxLayout(input_group);

    input_path_lineedit = new QLineEdit("D://test_files");

    // Путь выбирается через QFileDialog
    input_path_lineedit->setReadOnly(true);

    discover_input_directory = new QPushButton("Обзор...");

    connect(
        discover_input_directory,
        &QPushButton::clicked,
        this,
        &MainWindow::on_selectInputDirectoryButton_clicked
        );


    input_path_hlayout->addWidget(input_path_lineedit);
    input_path_hlayout->addWidget(discover_input_directory);

    main_vlayout->addWidget(input_group);
}


// Настройка поведения при совпадении имени выходного файла
void MainWindow::setupDuplicationGroup()
{
    QGroupBox *name_duplication_group =
        new QGroupBox(
            "Действие при повторении имени выходного файла"
            );

    QHBoxLayout *name_duplication_layout =
        new QHBoxLayout(name_duplication_group);

    action_on_name_duplication = new QComboBox;

    action_on_name_duplication->addItems({
        "Счётчик",
        "Перезаписать"
    });

    name_duplication_layout->addWidget(
        action_on_name_duplication
        );

    main_vlayout->addWidget(name_duplication_group);
}


// Настройка режима работы программы
void MainWindow::setupModeGroup()
{
    QGroupBox *processing_mode_group =
        new QGroupBox("Режим работы");

    QHBoxLayout *processing_mode_layout =
        new QHBoxLayout(processing_mode_group);

    mode = new QComboBox;

    mode->addItem("Разовый запуск");
    mode->addItem("Таймер");

    processing_mode_layout->addWidget(mode);

    main_vlayout->addWidget(processing_mode_group);


    // Изменение доступности настройки таймера
    connect(
        mode,
        &QComboBox::currentTextChanged,
        this,
        &MainWindow::currentModeChanged
        );


    // Настройка периода повторного запуска
    timer_duration = new QSpinBox();

    timer_duration->setValue(5);
    timer_duration->setRange(1, 3600);
    timer_duration->setEnabled(false);
    timer_duration->setSuffix(" сек.");

    processing_mode_layout->addWidget(timer_duration);
}


// Настройка поля XOR-ключа
void MainWindow::setupHexValueGroup()
{
    QGroupBox *hex_value_group =
        new QGroupBox("8-байт для XOR операции");

    QHBoxLayout *hex_value_layout =
        new QHBoxLayout(hex_value_group);

    xor_key = new QLineEdit;

    xor_key->setPlaceholderText(
        "1234567890ABCDEF"
        );

    // Разрешаем ввод только 16 hex-символов
    xor_key->setValidator(
        new QRegularExpressionValidator(
            QRegularExpression("[0-9A-Fa-f]{16}"),
            this
            )
        );

    hex_value_layout->addWidget(xor_key);

    main_vlayout->addWidget(hex_value_group);
}


// Сбор настроек из элементов GUI
Settings MainWindow::collectSettings()
{
    DuplicateAction duplicateNameAction;

    // Получение режима обработки дубликатов
    if (action_on_name_duplication->currentText() == "Счётчик")
    {
        duplicateNameAction = DuplicateAction::Counter;
    }
    else
    {
        duplicateNameAction = DuplicateAction::Rewrite;
    }


    Mode work_mode;

    // Получение режима запуска программы
    if (mode->currentText() == "Разовый запуск")
    {
        work_mode = Mode::Single;
    }
    else
    {
        work_mode = Mode::Timer;
    }


    // Формирование объекта настроек
    Settings s(
        input_file_mask->currentText(),
        delete_input_file->checkState(),
        save_path_lineedit->text(),
        save_name_lineedit->text(),
        input_path_lineedit->text(),
        duplicateNameAction,
        work_mode,
        timer_duration->value(),
        xor_key->text()
        );

    return s;
}


// Запуск или приостановка обработки
void MainWindow::startProcessing()
{
    startButton->setEnabled(true);

    // Если обработка ещё не запущена
    if (!m_isActive)
    {
        // Получение текущих настроек
        m_settings = collectSettings();


        // Проверка введённых параметров
        if (m_settings.mask.isEmpty())
        {
            qDebug() << "Маска входных файлов пустая";
            return;
        }

        if (m_settings.savePath.isEmpty())
        {
            qDebug() << "Путь для сохранения файлов пустой";
            return;
        }

        if (m_settings.saveName.isEmpty())
        {
            qDebug() << "Имя выходного файла пустое";
            return;
        }

        if (m_settings.inputPath.isEmpty())
        {
            qDebug() << "Путь с входными файлами пустой";
            return;
        }

        if (m_settings.XOR_key.length() != 16)
        {
            qDebug() << "Длина ключа не равна 16 символов";
            return;
        }


        // Перевод программы в активное состояние
        m_isActive = true;

        status_label->setVisible(true);
        cancelButton->setVisible(true);
        processing_progress_bar->setVisible(true);


        // Передача настроек Worker
        emit startWork(m_settings);
    }


    // Переключение между паузой и продолжением
    if (m_isPaused)
    {
        startButton->setText("Возобновить");
        status_label->setText("Выполнение на паузе...");

        m_worker->pause();
    }
    else
    {
        startButton->setText("Приостановить");
        status_label->setText("Идёт выполнение...");

        m_worker->resume();
    }

    m_isPaused = !m_isPaused;
}


// Обработчик кнопки запуска
void MainWindow::on_startButton_clicked()
{
    startProcessing();
}


// Обработчик кнопки завершения
void MainWindow::on_cancelButton_clicked()
{
    // Сброс состояния интерфейса
    cancelButton->setVisible(false);
    status_label->setVisible(false);
    processing_progress_bar->setVisible(false);
    file_progress_bar->setVisible(false);

    m_isActive = false;
    m_isPaused = false;

    startButton->setText("Начать выполнение");


    // Остановка Worker и таймера
    m_worker->cancel();
    m_processingTimer->stop();
}


// Обработка изменения режима работы
void MainWindow::currentModeChanged(const QString &text)
{
    if (text == "Таймер")
    {
        timer_duration->setEnabled(true);
    }
    else
    {
        timer_duration->setEnabled(false);
    }
}


// Выбор директории для сохранения результатов
void MainWindow::on_selectSaveDirectoryButton_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(
        this,
        "Выберите директорию"
        );

    if (!directory.isEmpty())
    {
        save_path_lineedit->setText(directory);
    }
}


// Выбор директории с входными файлами
void MainWindow::on_selectInputDirectoryButton_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(
        this,
        "Выберите директорию"
        );

    if (!directory.isEmpty())
    {
        input_path_lineedit->setText(directory);
    }
}


// Установка количества найденных файлов
void MainWindow::setFilesCount(const int& cnt)
{
    file_progress_bar->setRange(0, cnt);
    file_progress_bar->setValue(0);
    file_progress_bar->setVisible(true);
}


// Обновление количества обработанных файлов
void MainWindow::fileProcessed()
{
    file_progress_bar->setValue(
        file_progress_bar->value() + 1
        );
}


// Обновление прогресса текущего файла
void MainWindow::progress(
    const quint64& pos,
    const quint64& size
    )
{
    int percent =
        static_cast<int>(
            (pos * 100) / size
            );

    processing_progress_bar->setValue(percent);
}


// Обработка завершения текущего запуска
void MainWindow::completeProcessing()
{
    m_isActive = false;
    m_isPaused = false;


    // Если выбран режим таймера — запланировать следующий запуск
    if (m_settings.workMode == Mode::Timer)
    {
        int seconds = m_settings.seconds;

        qDebug()
            << "Следующий запуск через"
            << seconds
            << "сек.";

        status_label->setText(
            "Следующий запуск через "
            + QString::number(seconds)
            + " сек."
            );


        startButton->setEnabled(false);

        // Запуск таймера
        m_processingTimer->start(seconds * 1000);
    }
    else
    {
        // Завершение разового запуска
        status_label->setText("Выполнение завершено");
        startButton->setText("Начать выполнение");
        cancelButton->setVisible(false);
    }
}

MainWindow::~MainWindow()
{
    m_processingTimer->stop();

    // Запрос завершения Worker
    m_worker->cancel();

    // Завершение потока
    m_thread->quit();
    m_thread->wait();

    delete m_worker;
}
