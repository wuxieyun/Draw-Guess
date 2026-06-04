#include "imagecanvas.h"
#include "screenselector.h"
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QMouseEvent>
#include <QCursor>
#include <QFileInfo>
#include <QThread>

ImageCanvas::ImageCanvas(QWidget *parent)
    : QWidget(parent),
      currentState(CanvasState::Idle),
      drawTimer(new QTimer(this)),
      currentPointIndex(0),
      drawSpeed(20),
      imageLoaded(false)
{
    setFocusPolicy(Qt::StrongFocus);
    connect(drawTimer, &QTimer::timeout, this, &ImageCanvas::drawNextPoint);
}

void ImageCanvas::loadImage(const QString &path)
{
    qDebug() << "尝试加载图片:" << path;
    
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        qDebug() << "文件不存在！";
        emit statusUpdated("错误：图片文件不存在！");
        return;
    }
    
    originalImage.load(path);
    if (!originalImage.isNull()) {
        imageLoaded = true;
        processImage();
        update();
        emit statusUpdated(QString("图片已加载并解析 (%1x%2)").arg(originalImage.width()).arg(originalImage.height()));
        qDebug() << "图片加载成功，尺寸:" << originalImage.size();
    } else {
        qDebug() << "图片加载失败！";
        emit statusUpdated("错误：图片加载失败，请检查文件格式！");
    }
}

void ImageCanvas::startScreenSelection()
{
    ScreenSelector *selector = new ScreenSelector(this);
    connect(selector, &ScreenSelector::selectionConfirmed, this, &ImageCanvas::onScreenSelectionConfirmed);
    connect(selector, &ScreenSelector::selectionCancelled, this, &ImageCanvas::onScreenSelectionCancelled);
    selector->show();
    emit statusUpdated("请在屏幕上选择绘画区域...");
}

void ImageCanvas::onScreenSelectionConfirmed(const QRect &rect)
{
    selectedCanvas = rect;
    emit canvasConfirmed(selectedCanvas);
    emit statusUpdated(QString("已选择画布区域: (%1,%2) - %3x%4")
                       .arg(selectedCanvas.x())
                       .arg(selectedCanvas.y())
                       .arg(selectedCanvas.width())
                       .arg(selectedCanvas.height()));
    update();
}

void ImageCanvas::onScreenSelectionCancelled()
{
    emit statusUpdated("已取消画布选择");
}

void ImageCanvas::startDrawing()
{
    if (selectedCanvas.isValid() && !processedImage.isNull()) {
        generateDrawingPoints();
        if (drawingPoints.isEmpty()) {
            emit statusUpdated("没有可绘制的内容！请检查图片解析结果。");
            return;
        }
        
        currentPointIndex = 0;
        currentState = CanvasState::Drawing;
        drawTimer->start(1000 / drawSpeed);
        emit statusUpdated(QString("开始绘画，共 %1 个点，按ESC停止，+加速，-减速").arg(drawingPoints.size()));
    } else if (processedImage.isNull()) {
        emit statusUpdated("请先加载图片！");
    } else {
        emit statusUpdated("请先选择画布区域！");
    }
}

void ImageCanvas::stopDrawing()
{
    drawTimer->stop();
    currentState = CanvasState::Idle;
    emit statusUpdated("绘画已停止");
}

void ImageCanvas::setDrawSpeed(int speed)
{
    drawSpeed = qBound(1, speed, 100);
    if (drawTimer->isActive()) {
        drawTimer->setInterval(1000 / drawSpeed);
    }
    emit statusUpdated(QString("速度: %1").arg(drawSpeed));
}

int ImageCanvas::getDrawSpeed() const
{
    return drawSpeed;
}

QRect ImageCanvas::getSelectedCanvas() const
{
    return selectedCanvas;
}

QImage ImageCanvas::getProcessedImage() const
{
    return processedImage;
}

void ImageCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    
    painter.fillRect(rect(), Qt::lightGray);
    
    if (!originalImage.isNull()) {
        // 左侧显示原始图片
        QRect leftRect = QRect(10, 10, width() / 2 - 15, height() - 20);
        QImage scaledOriginal = originalImage.scaled(leftRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QRect drawOriginalRect = scaledOriginal.rect();
        drawOriginalRect.moveCenter(leftRect.center());
        painter.drawImage(drawOriginalRect, scaledOriginal);
        
        // 右侧显示处理后的图片
        QRect rightRect = QRect(width() / 2 + 5, 10, width() / 2 - 15, height() - 20);
        if (!processedImage.isNull()) {
            QImage scaledProcessed = processedImage.scaled(rightRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QRect drawProcessedRect = scaledProcessed.rect();
            drawProcessedRect.moveCenter(rightRect.center());
            painter.drawImage(drawProcessedRect, scaledProcessed);
        }
        
        // 绘制边框
        painter.setPen(QPen(Qt::black, 1));
        painter.drawRect(leftRect);
        painter.drawRect(rightRect);
        
        // 绘制标签
        painter.setPen(Qt::black);
        QFont font;
        font.setBold(true);
        font.setPointSize(10);
        painter.setFont(font);
        painter.drawText(leftRect.adjusted(5, 5, 0, 0), Qt::AlignLeft | Qt::AlignTop, "原图");
        painter.drawText(rightRect.adjusted(5, 5, 0, 0), Qt::AlignLeft | Qt::AlignTop, "线条图");
    } else {
        // 显示提示信息
        painter.setPen(Qt::gray);
        QFont font;
        font.setPointSize(14);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, "请先选择一张图片\n\n使用工具栏的\"选择图片\"按钮");
    }
    
    // 显示已选择的画布信息
    if (!selectedCanvas.isNull()) {
        QString canvasInfo = QString("已选择画布: (%1,%2) - %3x%4")
                            .arg(selectedCanvas.x())
                            .arg(selectedCanvas.y())
                            .arg(selectedCanvas.width())
                            .arg(selectedCanvas.height());
        
        painter.setPen(Qt::darkGreen);
        QFont infoFont;
        infoFont.setPointSize(10);
        painter.setFont(infoFont);
        painter.drawText(10, height() - 10, canvasInfo);
    }
}

void ImageCanvas::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (currentState == CanvasState::Drawing) {
            stopDrawing();
        }
    } else if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
        setDrawSpeed(drawSpeed + 5);
    } else if (event->key() == Qt::Key_Minus) {
        setDrawSpeed(drawSpeed - 5);
    }
    QWidget::keyPressEvent(event);
}

void ImageCanvas::drawNextPoint()
{
    if (currentPointIndex < drawingPoints.size()) {
        QPoint point = drawingPoints[currentPointIndex];
        
        // 移动鼠标并点击
        simulateMouseClick(point);
        
        currentPointIndex++;
        
        if (currentPointIndex % 100 == 0) {
            emit statusUpdated(QString("绘画进度: %1/%2").arg(currentPointIndex).arg(drawingPoints.size()));
        }
        
        if (currentPointIndex >= drawingPoints.size()) {
            stopDrawing();
            emit drawingFinished();
            emit statusUpdated("绘画完成！");
        }
    }
}

void ImageCanvas::simulateMouseClick(const QPoint &point)
{
    // 移动鼠标
    QCursor::setPos(point);
    
    // 使用事件循环确保窗口系统处理了移动
    QCoreApplication::processEvents();
    QThread::msleep(1);
}

void ImageCanvas::processImage()
{
    if (originalImage.isNull()) return;
    
    // 转换为灰度图
    QImage grayImage = originalImage.convertToFormat(QImage::Format_Grayscale8);
    
    // 二值化处理
    processedImage = QImage(grayImage.size(), QImage::Format_Mono);
    processedImage.setColorCount(2);
    processedImage.setColor(0, qRgb(255, 255, 255));
    processedImage.setColor(1, qRgb(0, 0, 0));
    
    int threshold = 128;
    
    for (int y = 0; y < grayImage.height(); y++) {
        for (int x = 0; x < grayImage.width(); x++) {
            int gray = qGray(grayImage.pixel(x, y));
            processedImage.setPixel(x, y, gray < threshold ? 1 : 0);
        }
    }
}

void ImageCanvas::generateDrawingPoints()
{
    drawingPoints.clear();
    
    if (processedImage.isNull() || !selectedCanvas.isValid()) return;
    
    // 将图片缩放到画布大小
    QImage scaledImage = processedImage.scaled(selectedCanvas.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    // 计算居中偏移
    int offsetX = (selectedCanvas.width() - scaledImage.width()) / 2;
    int offsetY = (selectedCanvas.height() - scaledImage.height()) / 2;
    
    // 收集所有黑色像素点
    for (int y = 0; y < scaledImage.height(); y++) {
        for (int x = 0; x < scaledImage.width(); x++) {
            if (qGray(scaledImage.pixel(x, y)) < 128) {
                drawingPoints.append(QPoint(selectedCanvas.x() + x + offsetX, selectedCanvas.y() + y + offsetY));
            }
        }
    }
    
    qDebug() << "生成了" << drawingPoints.size() << "个绘画点";
}
