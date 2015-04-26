#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../PRA_model/my_global.h"
#include "../PRA_model/mprocess.h"
#include "../PRA_model/mmu.h"
#include "../PRA_model/ram.h"

#include <QStyleOptionGraphicsItem>
#include <QGraphicsRectItem>
#include <QPainter>
#include <QWidget>

// I did it in less than 1.5 hours

std::shared_ptr<ProcessQueue> process_container;

class GraphicRAM : public QGraphicsRectItem, public RandomAccesMemory
{
public:
    GraphicRAM(size_t size, QGraphicsItem *parent = 0) :
        QGraphicsRectItem(parent),
        RandomAccesMemory(size)
    {
        const QSize itemSize = QSize(70, 30);
        QRect itemRect(QPoint(100, 0), itemSize);
        for(realPageAddress_tp i: pagesAddress()) {
            items[i].border = new QGraphicsRectItem(itemRect, this);
            items[i].text   = new QGraphicsTextItem(QString::number(itemRect.y()), this);
            items[i].text->setPos(itemRect.topLeft());
            itemRect.translate(0, itemSize.height() + 6);
        }
        dataUpdate();
    }

    virtual ~GraphicRAM() = default;

    void dataUpdate() {
        for(auto i = begin(items); i != end(items); ++i) {
            realPage &rPage = page(i->first);
            tick_tp ticks = 0;
            if(process_container) {
                ticks = process_container->tickCount() - rPage.lastAccesTime;
                i->second.border->setBrush(QBrush( ticks < constants::TICKS_WORKING_SET ? Qt::blue : Qt::black));
            }
            i->second.text->setPlainText(QString::number(rPage.data) + "\t" + QString::number(ticks) + "\t" + QString::number(rPage.R) + "\t" + QString::number(rPage.M));
        }
    }

private:
    struct ramPageItem {
        QGraphicsTextItem *text = nullptr;
        QGraphicsRectItem *border = nullptr;
    };

    std::map<realPageAddress_tp, ramPageItem> items;
};

class GraphicMMU : public QGraphicsRectItem, public PagedMemoryManagementUnit
{
public:
    GraphicMMU(ram_ptr ram, QGraphicsItem *parent = 0) :
        QGraphicsRectItem(parent),
        PagedMemoryManagementUnit(ram)
    {
    }

private:

};

class GraphicProcess: public QGraphicsRectItem
{
public:
    GraphicProcess(process_ptr process, QGraphicsItem *parent = 0) :
        QGraphicsRectItem(parent),
        _process(process)
    {
    }

    virtual ~GraphicProcess() = default;

    virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) {
        QGraphicsRectItem::paint(painter, option, widget);
        if(_process) {
            painter->drawText(0, 0, QString::number(_process->pid()));
            painter->drawText(0, 20, processStateToStr(_process->_processState));

            const QSize itemSize = QSize(70, 30);
            QRect itemRect(QPoint(0, 30), itemSize);
            QRect rBitRect(QPoint(5, 40), QSize(10, 10));
            for(auto p : _process->virtualPages) {
                if(_process->_workingSet.count(p.first)) {
                    painter->setPen(QPen(Qt::green));
                }
                painter->drawRect(itemRect);
                painter->setPen(QPen(Qt::black));

                QString address;
                if(p.second.P) {
                    address = QString::number(p.second.memAddress);
                    painter->setBrush(QBrush(Qt::blue));
                } else {
                    painter->setBrush(QBrush(Qt::red));
                }

                painter->drawRect(rBitRect);
                painter->setBrush(QBrush());


                painter->drawText(itemRect.topLeft() + QPoint(20, 20), address);
                painter->drawText(itemRect.topLeft() + QPoint(50, 20), QString::number(_process->_pagesFaults[p.first]));
                itemRect.translate(0, itemSize.height() + 6);
                rBitRect.translate(0, itemSize.height() + 6);

                painter->setPen(QPen(Qt::black));
            }
        }
    }

    virtual QRectF boundingRect() const {
        return QRectF(0, 0, 100, 1000);
    }

private:
    process_ptr _process;

    QString processStateToStr(Program::State state) {
        switch (state) {
        case Program::State::ALLOCATE:
            return "Allocate";
            break;
        case Program::State::CHANGE_WORKING_SET:
            return "Change working set";
            break;
        case Program::State::WORK:
            return "Work";
            break;
        case Program::State::DEALLOCATE:
            return "Deallocate";
            break;
        case Program::State::EXIT:
            return "Exit";
            break;
        default: return "Error";
            break;
        }
    }
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->graphicsView->setScene(&scene);

    _RAM = std::shared_ptr<GraphicRAM>(new GraphicRAM(constants::RAM_PAGE_COUNT));
    _MMU = std::shared_ptr<GraphicMMU>(new GraphicMMU(_RAM));

    queue = std::shared_ptr<ProcessQueue>(new ProcessQueue(_MMU));
    process_container = queue;
    _MMU->setProcessContainer(queue);


    for(int i = 0; i < 5; i++) {
        addNewProcess();
    }

    scene.addItem(_RAM.get());

    queue->run();

    auto sleepTime = queue->getSleepTime().count();
    _timerId = startTimer(sleepTime);
    ui->SleepTimeSlider->setValue(sleepTime);
    on_SleepTimeSlider_sliderMoved(sleepTime);

    ui->graphicsView->scroll(10, 1000);
    showFullScreen();
}

MainWindow::~MainWindow()
{
    queue->stop();
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Space) {
        if(_started) {
            queue->stop();
        } else {
            queue->run();
        }
        _started = !_started;
    }
}

void MainWindow::timerEvent(QTimerEvent *)
{
    _RAM->dataUpdate();
    ui->graphicsView->scene()->update();
}

void MainWindow::on_SleepTimeSlider_sliderMoved(int position)
{
    ui->SleepTime->setText(QString::number(position));
    queue->setSleepTime(std::chrono::milliseconds(position));
    killTimer(_timerId);
    _timerId = startTimer(position);
}

void MainWindow::on_addProcess_button_clicked()
{
    addNewProcess();
}

void MainWindow::addNewProcess()
{
    static QPoint processPos(-100, 0);
    std::shared_ptr<Program>  process = std::shared_ptr<Program>(new Program());

    queue->addProcess(process);
    GraphicProcess *gProcess = new GraphicProcess(process);
    gProcess->setPos(processPos);
    processPos.setX(processPos.x() - 100);
    scene.addItem(gProcess);
}
