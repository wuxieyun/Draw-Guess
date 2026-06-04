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
    setWindowTitle("你画我猜绘画辅助工具 v0.1");
    resize(1000, 700);
    
    canvas = new ImageCanvas(this);
    setCentralWidget(canvas);
    
    createActions();
    createMenus();
    createToolBar();
    
    statusBar()->showMessage("欢迎使用你画我猜绘画辅助工具！请先选择图片和画板区域。");
    
    connect(canvas, &ImageCanvas::statusUpdated, this, &MainWindow::updateStatus);
}

MainWindow::~MainWindow()
{
}

void MainWindow::selectImage()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, 
        "选择图片", 
        "", 
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif *.tiff);;所有文件 (*.*)"
    );
    if (!fileName.isEmpty()) {
        canvas->loadImage(fileName);
    }
}

void MainWindow::selectCanvas()
{
    canvas->startScreenSelection();
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
        "<p>版本：0.1</p>"
        "<p>功能说明：</p>"
        "<ul>"
        "<li><b>选择图片</b>：加载要绘制的图片</li>"
        "<li><b>选择画板</b>：在屏幕上框选绘画区域</li>"
        "<li><b>开始绘画</b>：在选定区域自动绘制图片</li>"
        "<li><b>快捷键</b>：ESC停止，+加速，-减速</li>"
        "</ul>"
        "<p>提示：绘画前请确保目标窗口可见且已准备好。</p>");
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
    selectImageAction->setStatusTip("选择要绘制的图片");
    connect(selectImageAction, &QAction::triggered, this, &MainWindow::selectImage);
    
    selectCanvasAction = new QAction("选择画板", this);
    selectCanvasAction->setShortcut(QKeySequence("C"));
    selectCanvasAction->setStatusTip("在屏幕上选择绘画区域");
    connect(selectCanvasAction, &QAction::triggered, this, &MainWindow::selectCanvas);
    
    startDrawAction = new QAction("开始绘画", this);
    startDrawAction->setShortcut(QKeySequence("S"));
    startDrawAction->setStatusTip("开始自动绘画");
    connect(startDrawAction, &QAction::triggered, this, &MainWindow::startDrawing);
    
    stopDrawAction = new QAction("停止绘画", this);
    stopDrawAction->setShortcut(QKeySequence("X"));
    stopDrawAction->setStatusTip("停止绘画");
    connect(stopDrawAction, &QAction::triggered, this, &MainWindow::stopDrawing);
    
    aboutAction = new QAction("关于", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
    fileMenu->addAction(selectImageAction);
    
    QMenu *editMenu = menuBar()->addMenu("操作(&O)");
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
