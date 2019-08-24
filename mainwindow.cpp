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
#include <QMediaPlaylist>
#include <QTextBrowser>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    manager(new DMFB),
    reverse(false),
    isPlaying(false),
    showPollution(false),
    playSound(true),
    washSound(":/sound/wash.wav"),
    washing(false)
    //washSound(new QMediaPlayer(this, QMediaPlayer::LowLatency))
{
    ui->setupUi(this);
    // set data for drawer
    ui->widget->manager = manager;
    // update layout for drawer
    ui->widget->updateSize();

    setWindowIcon(QIcon(":/png/icon.png"));

    ui->pushButton_last->setEnabled(false);
    ui->pushButton_next->setEnabled(false);
    ui->pushButton_play->setEnabled(false);
    ui->pushButton_reset->setEnabled(false);
    ui->pushButton_last->setVisible(false);
    ui->pushButton_next->setVisible(false);
    ui->pushButton_play->setVisible(false);
    ui->pushButton_reset->setVisible(false);
    ui->actionLast->setEnabled(false);
    ui->actionPlay->setEnabled(false);
    ui->actionNext->setEnabled(false);
    ui->actionReset->setEnabled(false);


    connect(&timer, SIGNAL(timeout()), this, SLOT(updatePlay()));
    connect(&washTimer, SIGNAL(timeout()), this, SLOT(wash()));
    setInterval(ui->horizontalSlider->value());
    ui->mainToolBar->addWidget(ui->lcdNumber);
    ui->mainToolBar->addWidget(ui->horizontalSlider);

    /*auto list = new QMediaPlaylist(washSound);
    list->addMedia(QUrl(":/sound/wash.wav"));
    list->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    washSound->setPlaylist(list);*/

    washSound.setLoops(-1);

    showInstruction();

}

MainWindow::~MainWindow()
{
    delete ui;
    if(manager)
        delete manager;
}

void MainWindow::sound()
{
    playSound = !playSound;
    if(playSound)
    {
        ui->actionSound->setIcon(QIcon(":/png/sound.png"));
    }
    else
    {
        ui->actionSound->setIcon(QIcon(":/png/nosound.png"));
    }
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
    if(!ui->pushButton_play->isEnabled())
        return;
    if(isPlaying)
    {
        stopPlay();
    }
    else
    {
        isPlaying = true;
        reverse = false;
        ui->pushButton_play->setText("stop");
        ui->pushButton_last->setEnabled(false);
        ui->pushButton_next->setEnabled(false);
        ui->pushButton_reset->setEnabled(false);
        ui->actionPlay->setIcon(QIcon(":/png/pause.png"));
        ui->actionNext->setEnabled(false);
        ui->actionNext->setEnabled(false);
        ui->actionReset->setEnabled(false);
        timer.start();
    }
}

void MainWindow::on_pushButton_last_clicked()
{
    if(!ui->pushButton_last->isEnabled())
        return;
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
    if(!ui->pushButton_next->isEnabled())
        return;
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


void MainWindow::on_pushButton_reset_clicked()
{

    if(!ui->pushButton_reset->isEnabled())
        return;
    reset();
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "about", "all rights reserved\nRao Congyuan 2019");
}

void MainWindow::showInstruction()
{
    const QString path(":/txt/instruction.txt");
    QFile f(path);
    if(!f.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "warning", QString("can't find file ") + path);
        return;
    }
    QTextStream text(&f);
    QString s = text.readAll();
    QMessageBox::information(this, "instruction", s);
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

void MainWindow::inputSound(bool flag)
{
    static QSound sound(":/sound/input.wav");
    if(flag)
        sound.play();
}

void MainWindow::outputSound(bool flag)
{
    static QSound sound(":/sound/output.wav");
    if(flag)
        sound.play();
}

void MainWindow::moveSound(bool flag)
{
    static QSound sound(":/sound/move.wav");
    if(flag)
        sound.play();
}

void MainWindow::mergeSound1(bool flag)
{
    static QSound sound(":/sound/merge1.wav");
    if(flag)
        sound.play();
}

void MainWindow::mergeSound2(bool flag)
{
    static QSound sound(":/sound/merge2.wav");
    if(flag)
        sound.play();
}

void MainWindow::splitSound1(bool flag)
{
    static QSound sound(":/sound/split1.wav");
    if(flag)
        sound.play();
}

void MainWindow::splitSound2(bool flag)
{
    static QSound sound(":/sound/split2.wav");
    if(flag)
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
        ui->actionLast->setEnabled(false);
        ui->actionReset->setEnabled(false);
        ui->actionNext->setEnabled(false);
        ui->actionPlay->setEnabled(false);
        return;
    }
    auto over = manager->over();
    ui->pushButton_play->setEnabled(!over);
    ui->pushButton_last->setEnabled(!manager->cleaner && manager->getT());
    ui->pushButton_reset->setEnabled(manager->getT());

    if(manager->normalOver())
    {
        ui->pushButton_next->setText("report");
        ui->actionNext->setIcon(QIcon(":/png/report.png"));
        ui->pushButton_next->setEnabled(!showPollution);
    }
    else
    {
        ui->pushButton_next->setText("next");
        ui->actionNext->setIcon(QIcon(":/png/next.png"));
        ui->pushButton_next->setEnabled(!over);
    }
    ui->actionLast->setEnabled(ui->pushButton_last->isEnabled());
    ui->actionNext->setEnabled(ui->pushButton_next->isEnabled());
    ui->actionPlay->setEnabled(ui->pushButton_play->isEnabled());
    ui->actionReset->setEnabled(ui->pushButton_reset->isEnabled());
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
        stopPlay();
    }
}

void MainWindow::stopPlay()
{
    timer.stop();
    ui->pushButton_play->setText("play");
    ui->actionPlay->setIcon(QIcon(":/png/play.png"));
    updatePushbutton();
    isPlaying = false;
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
    if(playSound)
    {
        inputSound(flag.input);
        outputSound(flag.output);
        moveSound(flag.move);
        mergeSound1(flag.merge1);
        mergeSound2(flag.merge2);
        splitSound1(flag.split1);
        splitSound2(flag.split2);
    }
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
    ui->actionLast->setEnabled(ui->pushButton_last->isEnabled());
    ui->actionNext->setEnabled(ui->pushButton_next->isEnabled());
    ui->lcdNumber->display("---");
    ui->widget->updateImage();
}

void MainWindow::hidePollution()
{
    showPollution = false;
    ui->widget->drawPollution = false;
    ui->pushButton_next->setEnabled(true);
    ui->pushButton_last->setEnabled(!manager->cleaner);
    ui->actionLast->setEnabled(ui->pushButton_last->isEnabled());
    ui->actionNext->setEnabled(ui->pushButton_next->isEnabled());
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
    washSound.stop();
}

bool MainWindow::checkWash()
{
    if(manager->cleaner && manager->wash())
    {
        washing = true;
        timer.stop();
        washTimer.start();
        if(playSound)
            washSound.play();
        updatePushbutton();
        ui->widget->updateImage();
        return true;
    }
    return false;
}

void MainWindow::keyPressEvent(QKeyEvent * e)
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
        //ui->pushButton_last->animateClick();
        on_pushButton_last_clicked();
        break;
    case Qt::Key::Key_Space:
    case Qt::Key::Key_P:
        //ui->pushButton_play->animateClick();
        on_pushButton_play_clicked();
        break;
    case Qt::Key::Key_Right:
    case Qt::Key::Key_D:
        //ui->pushButton_next->animateClick();
        on_pushButton_next_clicked();
        break;
    case Qt::Key::Key_R:
        //ui->pushButton_reset->animateClick();
        on_pushButton_reset_clicked();
        break;
    case Qt::Key::Key_Up:
    case Qt::Key::Key_W:
        // when press key for a time, silder will move faster
        ui->horizontalSlider->setValue(ui->horizontalSlider->value() + 1 + e->isAutoRepeat());
        break;
    case Qt::Key::Key_Down:
    case Qt::Key::Key_S:
        ui->horizontalSlider->setValue(ui->horizontalSlider->value() - 1 - e->isAutoRepeat());
        break;
    case Qt::Key::Key_V:
        sound();
        break;
    case Qt::Key::Key_I:
        showInstruction();
        break;
    default:
        break;
    }
    QMainWindow::keyPressEvent(e);
}
