#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QSpinBox>
#include <QFileDialog>
#include <QProgressBar>
#include <QRegularExpressionValidator>
#include <QThread>
#include <QTimer>
#include "settings.h"
#include "worker.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // Инициализация Worker и элементов интерфейса
    void setupWorker();
    void setupMaskGroup();
    void setupDeleteCheckBox();
    void setupSavePathGroup();
    void setupInputPathGroup();
    void setupDuplicationGroup();
    void setupModeGroup();
    void setupHexValueGroup();

    // Сбор настроек из элементов GUI
    Settings collectSettings();

    // Поток и Worker для фоновой обработки файлов
    QThread* m_thread;
    Worker *m_worker;

    QVBoxLayout *main_vlayout;

    // Настройка маски входных файлов
    QLabel *input_file_mask_label;
    QComboBox *input_file_mask;

    // Удаление входных файлов после обработки
    QCheckBox *delete_input_file;

    // Настройки выходного пути и имени файла
    QHBoxLayout *save_path_hlayout;
    QLineEdit *save_path_lineedit;
    QLineEdit *save_name_lineedit;
    QPushButton *discover_save_directory;

    // Настройки пути поиска входных файлов
    QHBoxLayout *input_path_hlayout;
    QLineEdit *input_path_lineedit;
    QPushButton *discover_input_directory;

    // Обработка дубликатов и выбор режима работы
    QComboBox *action_on_name_duplication;
    QComboBox *mode;
    QSpinBox *timer_duration;

    // 8-байтный XOR ключ в hex формате
    QLineEdit *xor_key;

    // Управление обработкой
    QPushButton* startButton;
    QPushButton* cancelButton;

    // Индикаторы прогресса и состояния
    QProgressBar* file_progress_bar;
    QProgressBar* processing_progress_bar;
    QLabel* status_label;

    // Текущее состояние обработки
    bool m_isActive;
    bool m_isPaused;

    // Текущие настройки и таймер повторного запуска
    Settings m_settings;
    QTimer *m_processingTimer;

private slots:
    // Обработка изменения режима работы
    void currentModeChanged(const QString &text);

    // Выбор директорий
    void on_selectSaveDirectoryButton_clicked();
    void on_selectInputDirectoryButton_clicked();

    // Управление обработкой
    void on_startButton_clicked();
    void startProcessing();
    void on_cancelButton_clicked();

    // Обновление информации о ходе обработки
    void setFilesCount(const int& cnt);
    void fileProcessed();
    void progress(const quint64& pos, const quint64& size);
    void completeProcessing();

signals:
    // Запуск обработки Worker-ом в отдельном потоке
    void startWork(Settings s);
};
#endif // MAINWINDOW_H
