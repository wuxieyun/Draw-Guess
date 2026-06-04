#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "imagecanvas.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void selectImage();
    void selectCanvas();
    void startDrawing();
    void stopDrawing();
    void increaseSpeed();
    void decreaseSpeed();
    void showAbout();
    void updateStatus(const QString &message);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void createActions();
    void createMenus();
    void createToolBar();

    ImageCanvas *canvas;
    QAction *selectImageAction;
    QAction *selectCanvasAction;
    QAction *startDrawAction;
    QAction *stopDrawAction;
    QAction *aboutAction;
};

#endif
