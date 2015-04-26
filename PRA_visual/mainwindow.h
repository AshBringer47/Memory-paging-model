#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsScene>
#include <QMainWindow>
#include <QKeyEvent>

#include <memory>

class ProcessQueue;
class GraphicRAM;
class GraphicMMU;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *e);

private slots:
    void on_SleepTimeSlider_sliderMoved(int position);

    void on_addProcess_button_clicked();

private:
    void addNewProcess();

    void timerEvent(QTimerEvent *);

    Ui::MainWindow *ui;
    QGraphicsScene scene;

    int _timerId = 0;

    bool _started = true;
    std::shared_ptr<ProcessQueue> queue;
    std::shared_ptr<GraphicRAM> _RAM;
    std::shared_ptr<GraphicMMU> _MMU;
};

#endif // MAINWINDOW_H
