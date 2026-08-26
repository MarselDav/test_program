#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_settings = Settings();
    m_processingTimer = new QTimer(this);
    m_processingTimer->setSingleShot(true);

    connect(
        m_processingTimer,
        &QTimer::timeout,
        this,
        &MainWindow::startProcessing
    );

    QWidget *central = new QWidget;
    main_vlayout = new QVBoxLayout;

    m_isActive = false;
    m_isPaused = false;

    setupWorker();
    setupMaskGroup();
    setupDeleteCheckBox();
    setupSavePathGroup();
    setupInputPathGroup();
    setupDuplicationGroup();
    setupModeGroup();
    setupHexValueGroup();

    startButton = new QPushButton("Начать выполнение");
    connect(startButton, &QPushButton::clicked, this, &MainWindow::on_startButton_clicked);
    main_vlayout->addWidget(startButton);

    cancelButton = new QPushButton("Завершить");
    connect(cancelButton, &QPushButton::clicked, this, &MainWindow::on_cancelButton_clicked);
    cancelButton->setStyleSheet("background-color: rgb(245, 85, 73)");
    cancelButton->setVisible(false);
    main_vlayout->addWidget(cancelButton);

    file_progress_bar = new QProgressBar;
    file_progress_bar->setFormat("%v/%m");
    file_progress_bar->setVisible(false);
    main_vlayout->addWidget(file_progress_bar);

    processing_progress_bar = new QProgressBar;
    processing_progress_bar->setRange(0, 100);
    processing_progress_bar->setFormat("%p%");
    processing_progress_bar->setVisible(false);
    main_vlayout->addWidget(processing_progress_bar);

    status_label = new QLabel;
    status_label->setAlignment(Qt::AlignCenter);
    status_label->setVisible(false);
    main_vlayout->addWidget(status_label);

    central->setLayout(main_vlayout);
    setCentralWidget(central);
}

void MainWindow::setupWorker()
{
    m_thread = new QThread(this);
    m_worker = new Worker();

    m_worker->moveToThread(m_thread);

    connect(this, &MainWindow::startWork, m_worker, &Worker::startProcessing);

    connect(m_worker, &Worker::filesCount, this, &MainWindow::setFilesCount);
    connect(m_worker, &Worker::fileProcessed, this, &MainWindow::fileProcessed);
    connect(m_worker, &Worker::progress, this, &MainWindow::progress);
    connect(m_worker, &Worker::completeProcessing, this, &MainWindow::completeProcessing);

    m_thread->start();
}

void MainWindow::setupMaskGroup()
{
    QGroupBox *mask_group = new QGroupBox("Маска файлов");
    QHBoxLayout *mask_layout = new QHBoxLayout(mask_group);

    mask_layout->addWidget(new QLabel("Маска:"));
    input_file_mask = new QComboBox;
    // connect(input_file_mask, &QComboBox::currentTextChanged, )
    input_file_mask->setEditable(true);
    input_file_mask->addItems({"*.txt", "*.bin", "*.*"});
    mask_layout->addWidget(input_file_mask);

    main_vlayout->addWidget(mask_group);
}

void MainWindow::setupDeleteCheckBox()
{
    delete_input_file = new QCheckBox("Удалять исходный файл после обработки");
    main_vlayout->addWidget(delete_input_file);
}

void MainWindow::setupSavePathGroup()
{
    QGroupBox *save_group = new QGroupBox("Путь для сохранения результатов");
    save_path_hlayout = new QHBoxLayout(save_group);
    save_path_lineedit = new QLineEdit("D://test_files");
    save_name_lineedit = new QLineEdit("test_file");
    QLabel *separator = new QLabel("/");
    save_name_lineedit->setPlaceholderText("Имя выходного файла");
    save_path_lineedit->setReadOnly(true);
    discover_save_directory = new QPushButton("Обзор...");
    connect(discover_save_directory,
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

void MainWindow::setupInputPathGroup()
{
    QGroupBox *input_group = new QGroupBox("Путь для поиска файлов");
    input_path_hlayout = new QHBoxLayout(input_group);
    input_path_lineedit = new QLineEdit("D://test_files");
    input_path_lineedit->setReadOnly(true);
    discover_input_directory = new QPushButton("Обзор...");
    connect(discover_input_directory,
            &QPushButton::clicked,
            this,
            &MainWindow::on_selectInputDirectoryButton_clicked
            );
    input_path_hlayout->addWidget(input_path_lineedit);
    input_path_hlayout->addWidget(discover_input_directory);
    main_vlayout->addWidget(input_group);
}

void MainWindow::setupDuplicationGroup()
{
    QGroupBox *name_duplication_group = new QGroupBox("Действие при повторении имени выходного файла");
    QHBoxLayout *name_duplication_layout = new QHBoxLayout(name_duplication_group);
    action_on_name_duplication = new QComboBox;
    action_on_name_duplication->addItems({"Счётчик", "Перезаписать"});
    name_duplication_layout->addWidget(action_on_name_duplication);
    main_vlayout->addWidget(name_duplication_group);
}

void MainWindow::setupModeGroup()
{
    QGroupBox *processing_mode_group = new QGroupBox("Режим работы");
    QHBoxLayout *processing_mode_layout = new QHBoxLayout(processing_mode_group);
    mode = new QComboBox;
    mode->addItem("Разовый запуск");
    mode->addItem("Таймер");
    processing_mode_layout->addWidget(mode);
    main_vlayout->addWidget(processing_mode_group);

    connect(mode,
            &QComboBox::currentTextChanged,
            this,
            &MainWindow::currentModeChanged
            );

    timer_duration = new QSpinBox();
    timer_duration->setValue(5);
    timer_duration->setRange(1, 3600);
    timer_duration->setEnabled(false);
    timer_duration->setSuffix(" сек.");
    processing_mode_layout->addWidget(timer_duration);
}

void MainWindow::setupHexValueGroup()
{
    QGroupBox *hex_value_group = new QGroupBox("8-байт для XOR операции");
    QHBoxLayout *hex_value_layout = new QHBoxLayout(hex_value_group);

    xor_key = new QLineEdit;
    xor_key->setPlaceholderText("1234567890ABCDEF");
    xor_key->setValidator(
        new QRegularExpressionValidator(QRegularExpression("[0-9A-Fa-f]{16}"), this));
    hex_value_layout->addWidget(xor_key);
    main_vlayout->addWidget(hex_value_group);
}

Settings MainWindow::collectSettings()
{
    DuplicateAction duplicateNameAction;
    if (action_on_name_duplication->currentText() == "Счётчик")
    {
        duplicateNameAction = DuplicateAction::Counter;
    }
    else
    {
        duplicateNameAction = DuplicateAction::Rewrite;
    }

    Mode work_mode;
    if (mode->currentText() == "Разовый запуск")
    {
        work_mode = Mode::Single;
    }
    else
    {
        work_mode = Mode::Timer;
    }

    Settings s(input_file_mask->currentText(),
               delete_input_file->checkState(),
               save_path_lineedit->text(),
               save_name_lineedit->text(),
               input_path_lineedit->text(),
               duplicateNameAction,
               work_mode,
               timer_duration->value(),
               xor_key->text());
    return s;
}

void MainWindow::startProcessing()
{
    startButton->setEnabled(true);
    if (!m_isActive)
    {
        m_settings = collectSettings();

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

        m_isActive = true;

        status_label->setVisible(true);
        cancelButton->setVisible(true);
        status_label->setVisible(true);
        processing_progress_bar->setVisible(true);

        emit startWork(m_settings);
    }

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

void MainWindow::on_startButton_clicked()
{
    startProcessing();
}

void MainWindow::on_cancelButton_clicked()
{
    cancelButton->setVisible(false);
    status_label->setVisible(false);
    processing_progress_bar->setVisible(false);
    file_progress_bar->setVisible(false);
    m_isActive = false;
    m_isPaused = false;
    startButton->setText("Начать выполнение");

    m_worker->cancel();
    m_processingTimer->stop();
}

void MainWindow::currentModeChanged(const QString &text)
{
    if (text == "Таймер")
    {
        timer_duration->setEnabled(true);
    }
    else {
        timer_duration->setEnabled(false);
    }
}

void MainWindow::on_selectSaveDirectoryButton_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(
        this, "Выберите директорию"
    );

    if (!directory.isEmpty())
    {
        save_path_lineedit->setText(directory);
    }
}

void MainWindow::on_selectInputDirectoryButton_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(
        this, "Выберите директорию"
    );

    if (!directory.isEmpty())
    {
        input_path_lineedit->setText(directory);
    }
}

void MainWindow::setFilesCount(const int& cnt)
{
    file_progress_bar->setRange(0, cnt);
    file_progress_bar->setValue(0);
    file_progress_bar->setVisible(true);
}

void MainWindow::fileProcessed()
{
    file_progress_bar->setValue(file_progress_bar->value() + 1);
    // processing_progress_bar->setValue(processing_progress_bar->maximum());
}

void MainWindow::progress(const quint64& pos, const quint64& size)
{
    int percent =
        static_cast<int>(
            (pos * 100) / size
    );
    processing_progress_bar->setValue(percent);
}

void MainWindow::completeProcessing()
{
    m_isActive = false;
    m_isPaused = false;

    if (m_settings.workMode == Mode::Timer)
    {
        int seconds = m_settings.seconds;
        qDebug() << "Следующий запуск через" << seconds << "сек.";
        status_label->setText("Следующий запуск через " + QString::number(seconds) + " сек.");

        startButton->setEnabled(false);
        m_processingTimer->start(seconds * 1000);
    }
    else
    {
        status_label->setText("Выполнение завершено");
        startButton->setText("Начать выполнение");
        cancelButton->setVisible(false);
    }
}

MainWindow::~MainWindow() {
    m_processingTimer->stop();
    m_worker->cancel();
    m_thread->quit();
    m_thread->wait();

    delete m_worker;
}
