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
#include "settings.h"
#include "worker.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupWorker();
    void setupMaskGroup();
    void setupDeleteCheckBox();
    void setupSavePathGroup();
    void setupInputPathGroup();
    void setupDuplicationGroup();
    void setupModeGroup();
    void setupHexValueGroup();

    Settings collectSettings();

    QThread* m_thread;
    Worker *m_worker;

    QVBoxLayout *main_vlayout;

    QLabel *input_file_mask_label;
    QComboBox *input_file_mask;
    QCheckBox *delete_input_file;

    QHBoxLayout *save_path_hlayout;
    QLineEdit *save_path_lineedit;
    QLineEdit *save_name_lineedit;
    QPushButton *discover_save_directory;

    QHBoxLayout *input_path_hlayout;
    QLineEdit *input_path_lineedit;
    QPushButton *discover_input_directory;

    QComboBox *action_on_name_duplication;
    QComboBox *mode;
    QSpinBox *timer_duration;

    QLineEdit *xor_key;

    QPushButton* start;
    QPushButton* shutdown;

    QProgressBar* file_progress_bar;
    QProgressBar* processing_progress_bar;
    QLabel* status_label;

    bool m_isActive;
    bool m_isPaused;

private slots:
    void currentModeChanged(const QString &text);
    void on_selectSaveDirectoryButton_clicked();
    void on_selectInputDirectoryButton_clicked();
    void on_startButton_clicked();
    void on_shutdownButton_clicked();

    void setFilesCount(int cnt);
    void setFileSize(int size);
    void fileProcessed();
    void oneBlockProcessed(int step);
    void completeProcessing();

signals:
    void startWork(Settings s);
};
#endif // MAINWINDOW_H
