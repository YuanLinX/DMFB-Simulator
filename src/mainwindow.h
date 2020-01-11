#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include "dmfb.h"
#include <QTimer>
#include <QMediaPlayer>
#include <QSound>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void keyPressEvent(QKeyEvent *);

public slots:
    void setOption();
    void loadFile();
    void on_pushButton_play_clicked();
    void on_pushButton_last_clicked();
    void on_pushButton_next_clicked();
    void on_pushButton_reset_clicked();
    void showAbout();
    void showInstruction();
    void setInterval(int);
    void updatePlay();
    void wash();
    void sound();

    void inputSound(bool);
    void outputSound(bool);
    void moveSound(bool);
    void mergeSound1(bool);
    void mergeSound2(bool);
    void splitSound1(bool);
    void splitSound2(bool);

private:
    Ui::MainWindow *ui;
    DMFB * manager;
    auto getDrawPos(int x, int y, int row, int col);

    bool reverse;
    bool isPlaying;
    bool showPollution;
    QString filename;
    bool playSound;
    QSound washSound;

    void updateDMFB();
    QTimer timer;
    QTimer washTimer;
    void updatePushbutton();
    void setShowPollution();
    void hidePollution();
    void reset();
    void stopPlay();


    bool checkWash();
    bool washing;
    //QMediaPlayer * washSound;
};

#endif // MAINWINDOW_H
