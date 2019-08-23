#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include "dmfb.h"
#include <QTimer>

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

    void inputSound();
    void outputSound();
    void moveSound();
    void mergeSound1();
    void mergeSound2();
    void splitSound1();
    void splitSound2();

private:
    Ui::MainWindow *ui;
    DMFB * manager;
    auto getDrawPos(int x, int y, int row, int col);

    bool reverse;
    bool isPlaying;
    bool showPollution;
    QString filename;

    void updateDMFB();
    QTimer timer;
    QTimer washTimer;
    void updatePushbutton();
    void setShowPollution();
    void hidePollution();
    void reset();


    bool checkWash();
    bool washing;
};

#endif // MAINWINDOW_H
