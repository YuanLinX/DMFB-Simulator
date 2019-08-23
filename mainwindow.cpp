#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "option.h"
#include <QFile>
#include <QDebug>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>
#include <QScreen>
#include <QSound>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    manager(new DMFB),
    reverse(false),
    isPlaying(false),
    showPollution(false),
    washing(false)
{
    ui->setupUi(this);
    // set data for drawer
    ui->widget->manager = manager;
    // update layout for drawer
    ui->widget->updateSize();

    ui->pushButton_last->setEnabled(false);
    ui->pushButton_next->setEnabled(false);
    ui->pushButton_play->setEnabled(false);
    ui->pushButton_reset->setEnabled(false);

    connect(&timer, SIGNAL(timeout()), this, SLOT(updatePlay()));
    connect(&washTimer, SIGNAL(timeout()), this, SLOT(wash()));
    setInterval(ui->horizontalSlider->value());
}

MainWindow::~MainWindow()
{
    delete ui;
    if(manager)
        delete manager;
}

void MainWindow::setOption()
{
    Option option(this);
    if(option.exec() == QDialog::Accepted)
    {
        manager->row = option.row;
        manager->col = option.col;
        manager->output = option.output;
        manager->inputs = option.inputs;
        manager->cleaner = option.cleaner;
        manager->initalize();
        auto widget_size = ui->widget->getPreferredSize();
        widget_size.setWidth(int(widget_size.width() * 1.6));
        widget_size.setHeight(int(widget_size.height() * 1.6));

        resize(widget_size);
        ui->widget->updateSize();
        auto screen = qApp->screens()[0]->size();
        move((screen.width() - width())/2, (screen.height() - height()) / 2);
    }
    filename = "";
    reset();
}

auto MainWindow::getDrawPos(int x, int y, int row, int col)
{
    if(!x)
    {
        x--;
    }
    else if(x + 1 == col)
    {
        x++;
    }
    else if(!y)
    {
        y--;
    }
    else if(y + 1 == row)
    {
        y++;
    }
    else
    {
        qDebug() << "error input position";
    }
    return QPair<int, int>(x, y);
}

void MainWindow::on_pushButton_play_clicked()
{
    if(isPlaying)
    {
        timer.stop();
        ui->pushButton_play->setText("play");
        updatePushbutton();
        isPlaying = false;
    }
    else
    {
        isPlaying = true;
        reverse = false;
        ui->pushButton_play->setText("stop");
        ui->pushButton_last->setEnabled(false);
        ui->pushButton_next->setEnabled(false);
        ui->pushButton_reset->setEnabled(false);
        timer.start();
    }
}

void MainWindow::on_pushButton_last_clicked()
{
    if(showPollution)
    {
        hidePollution();
    }
    else
    {
        reverse = true;
        updateDMFB();
    }
}

void MainWindow::on_pushButton_next_clicked()
{
    if(manager->normalOver())
    {
        setShowPollution();
    }
    else
    {
        reverse = false;
        updateDMFB();
    }
}

void MainWindow::setInterval(int value)
{
    // hope f(0) = 1000 ms, f(99) = fastest ms
    // let f(x) = 1000 * exp(a * value) ms
    // then a = log(fastest / 1000) / 99
    static double fastest = 50;
    static double a = log(fastest / 1000) / 99;
    auto interval = 1000 * exp(a * value);
    timer.setInterval(int(interval));
    // wash will speed up to 4 times
    washTimer.setInterval(int(interval / 4));
}

void MainWindow::inputSound()
{
    static QSound sound("sound/input.wav");
    sound.play();
}

void MainWindow::outputSound()
{
    static QSound sound("sound/output.wav");
    sound.play();
}

void MainWindow::moveSound()
{
    static QSound sound("sound/move.wav");
    sound.play();
}

void MainWindow::mergeSound1()
{
    static QSound sound("sound/merge1.wav");
    sound.play();
}

void MainWindow::mergeSound2()
{
    static QSound sound("sound/merge2.wav");
    sound.play();
}

void MainWindow::splitSound1()
{
    static QSound sound("sound/split1.wav");
    sound.play();
}

void MainWindow::splitSound2()
{
    static QSound sound("sound/split2.wav");
    sound.play();
}

void MainWindow::updatePushbutton()
{
    if(washing)
    {
        ui->pushButton_next->setEnabled(false);
        ui->pushButton_play->setEnabled(false);
        ui->pushButton_reset->setEnabled(false);
        ui->pushButton_last->setEnabled(false);
        return;
    }
    auto over = manager->over();
    ui->pushButton_play->setEnabled(!over);
    ui->pushButton_last->setEnabled(!manager->cleaner && manager->getT());
    ui->pushButton_reset->setEnabled(manager->getT());

    if(manager->normalOver())
    {
        ui->pushButton_next->setText("report");
        ui->pushButton_next->setEnabled(!showPollution);
    }
    else
    {
        ui->pushButton_next->setText("next");
        ui->pushButton_next->setEnabled(!over);
    }
}

void MainWindow::loadFile()
{
    auto name = QFileDialog::getOpenFileName(this);
    if(!name.length())
        return;
    manager->initalize();
    if(name.length())
    {
        QFile f(name);
        if(!f.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this, "open file failed", QString("can't open the file ") + filename);
            return;
        }
        QTextStream s(&f);
        QString str;
        do
        {
            str = s.readLine().remove(";");
            manager->addInstruction(str);
        }while(str.length());
    }
    filename = name;
    reset();
}

void MainWindow::reset()
{
    if(manager->cleaner)
    {
        manager->initalize(false);
    }
    manager->reset();

    if(manager->getLastT() > -1)
    {
        ui->pushButton_next->setEnabled(true);
        ui->pushButton_play->setEnabled(true);
    }
    hidePollution();
    ui->lcdNumber->display(0);
    ui->widget->updateBackground();
    updatePushbutton();
    update();
}

void MainWindow::updatePlay()
{
    updateDMFB();
    if(manager->over())
    {
        timer.stop();
        on_pushButton_play_clicked();
    }
}

void MainWindow::updateDMFB()
{
    if(reverse)
    {
        manager->last();
    }
    else
    {
        if(!manager->next())
        {
            auto errors = manager->errorInfo;
            qDebug()<<errors;
            for(int i = 0; i < errors.length(); i++)
            {
                QMessageBox::critical(this, QString("error %1").arg(i + 1), errors[i]);
            }
        }
    }
    auto flag = manager->getFlag();
    if(flag.input)
        inputSound();
    if(flag.output)
        outputSound();
    if(flag.move)
        moveSound();
    if(flag.merge1)
        mergeSound1();
    if(flag.merge2)
        mergeSound2();
    if(flag.split1)
        splitSound1();
    if(flag.split2)
        splitSound2();
    if(!isPlaying)
    {
        updatePushbutton();
    }

    ui->lcdNumber->display(manager->getT());
    ui->widget->updateImage();

    checkWash();

}

void MainWindow::setShowPollution()
{
    showPollution = true;
    ui->widget->drawPollution = true;
    ui->pushButton_next->setEnabled(false);
    ui->pushButton_last->setEnabled(true);
    ui->lcdNumber->display("---");
    ui->widget->updateImage();
}

void MainWindow::hidePollution()
{
    showPollution = false;
    ui->widget->drawPollution = false;
    ui->pushButton_next->setEnabled(true);
    ui->pushButton_last->setEnabled(!manager->cleaner);
    ui->lcdNumber->display(manager->getT());
    ui->widget->updateImage();
}

void MainWindow::wash()
{
    if(manager->wash())
    {
        ui->widget->updateImage();
        return;
    }
    washing = false;
    washTimer.stop();
    if(isPlaying)
        timer.start();
    updatePushbutton();
    ui->widget->updateImage();
}

bool MainWindow::checkWash()
{
    if(manager->cleaner && manager->wash())
    {
        washing = true;
        timer.stop();
        washTimer.start();
        updatePushbutton();
        ui->widget->updateImage();
        return true;
    }
    return false;
}

void MainWindow::keyPressEvent(QKeyEvent * e)
{
    if(!e->isAutoRepeat())
    {
        switch(e->key())
        {
        case Qt::Key::Key_O:
            setOption();
            break;
        case Qt::Key::Key_L:
            loadFile();
            break;
        case Qt::Key::Key_Escape:
            close();
            break;
        case Qt::Key::Key_Left:
        case Qt::Key::Key_A:
            ui->pushButton_last->animateClick();
            break;
        case Qt::Key::Key_Space:
        case Qt::Key::Key_P:
            ui->pushButton_play->animateClick();
            break;
        case Qt::Key::Key_Right:
        case Qt::Key::Key_D:
            ui->pushButton_next->animateClick();
            break;
        case Qt::Key::Key_R:
            ui->pushButton_reset->animateClick();
            break;
        case Qt::Key::Key_Up:
            ui->horizontalSlider->setValue(ui->horizontalSlider->value() + 1);
            break;
        case Qt::Key::Key_Down:
            ui->horizontalSlider->setValue(ui->horizontalSlider->value() - 1);
            break;
        default:
            break;
        }
    }
    else
    {
        switch(e->key())
        {
        case Qt::Key::Key_Up:
            ui->horizontalSlider->setValue(ui->horizontalSlider->value() + 2);
            break;
        case Qt::Key::Key_Down:
            ui->horizontalSlider->setValue(ui->horizontalSlider->value() - 2);
            break;
        default:
            break;
        }
    }
    QMainWindow::keyPressEvent(e);
}
