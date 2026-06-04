#include "mainwindow.h"
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("你画我猜绘画辅助工具 v0.0.1");
    
    canvas = new ImageCanvas(this);
    setCentralWidget(canvas);
    
    createActions();
    createMenus();
    createToolBar();
    
    statusBar()->showMessage("欢迎使用你画我猜绘画辅助工具 v0.0.1");
    
    connect(canvas, &ImageCanvas::statusUpdated, this, &MainWindow::updateStatus);
}

MainWindow::~MainWindow()
{
}

void MainWindow::selectImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择图片", "", "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (!fileName.isEmpty()) {
        canvas->loadImage(fileName);
    }
}

void MainWindow::selectCanvas()
{
    canvas->startCanvasSelection();
}

void MainWindow::startDrawing()
{
    canvas->startDrawing();
}

void MainWindow::stopDrawing()
{
    canvas->stopDrawing();
}

void MainWindow::increaseSpeed()
{
    canvas->setDrawSpeed(canvas->getDrawSpeed() + 5);
}

void MainWindow::decreaseSpeed()
{
    canvas->setDrawSpeed(canvas->getDrawSpeed() - 5);
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "关于", 
        "<h3>你画我猜绘画辅助工具</h3>"
        "<p>版本：v0.0.1</p>"
        "<p>版权：无忧。所有</p>"
        "<p>功能说明：</p>"
        "<ul>"
        "<li>选择图片：加载要绘制的图片</li>"
        "<li>解析图片：自动将图片转换为线条画</li>"
        "<li>框选画布：鼠标框选游戏画布，回车确认</li>"
        "<li>开始绘画：自动在画布上绘制</li>"
        "<li>+号：增加绘画速度</li>"
        "<li>-号：减少绘画速度</li>"
        "<li>ESC：终止绘画</li>"
        "</ul>");
}

void MainWindow::updateStatus(const QString &message)
{
    statusBar()->showMessage(message);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
        increaseSpeed();
    } else if (event->key() == Qt::Key_Minus) {
        decreaseSpeed();
    } else if (event->key() == Qt::Key_Escape) {
        stopDrawing();
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::createActions()
{
    selectImageAction = new QAction("选择图片", this);
    selectImageAction->setShortcut(QKeySequence::Open);
    connect(selectImageAction, &QAction::triggered, this, &MainWindow::selectImage);
    
    selectCanvasAction = new QAction("框选画布", this);
    selectCanvasAction->setShortcut(QKeySequence("C"));
    connect(selectCanvasAction, &QAction::triggered, this, &MainWindow::selectCanvas);
    
    startDrawAction = new QAction("开始绘画", this);
    startDrawAction->setShortcut(QKeySequence("S"));
    connect(startDrawAction, &QAction::triggered, this, &MainWindow::startDrawing);
    
    stopDrawAction = new QAction("停止绘画", this);
    stopDrawAction->setShortcut(QKeySequence("X"));
    connect(stopDrawAction, &QAction::triggered, this, &MainWindow::stopDrawing);
    
    aboutAction = new QAction("关于", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
    fileMenu->addAction(selectImageAction);
    
    QMenu *editMenu = menuBar()->addMenu("编辑(&E)");
    editMenu->addAction(selectCanvasAction);
    editMenu->addSeparator();
    editMenu->addAction(startDrawAction);
    editMenu->addAction(stopDrawAction);
    
    QMenu *helpMenu = menuBar()->addMenu("帮助(&H)");
    helpMenu->addAction(aboutAction);
}

void MainWindow::createToolBar()
{
    QToolBar *toolBar = addToolBar("工具栏");
    toolBar->addAction(selectImageAction);
    toolBar->addAction(selectCanvasAction);
    toolBar->addSeparator();
    toolBar->addAction(startDrawAction);
    toolBar->addAction(stopDrawAction);
}
