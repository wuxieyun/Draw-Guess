#include "imagecanvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QWindow>

ImageCanvas::ImageCanvas(QWidget *parent)
    : QWidget(parent),
      currentState(CanvasState::Idle),
      drawTimer(new QTimer(this)),
      currentPointIndex(0),
      drawSpeed(50)
{
    setFocusPolicy(Qt::StrongFocus);
    connect(drawTimer, &QTimer::timeout, this, &ImageCanvas::drawNextPoint);
}

void ImageCanvas::loadImage(const QString &path)
{
    originalImage.load(path);
    if (!originalImage.isNull()) {
        processImage();
        update();
        emit statusUpdated("图片已加载并解析");
    } else {
        emit statusUpdated("图片加载失败");
    }
}

void ImageCanvas::startCanvasSelection()
{
    currentState = CanvasState::SelectingCanvas;
    selectionStart = QPoint();
    selectionEnd = QPoint();
    selectedCanvas = QRect();
    setFocus();
    emit statusUpdated("请用鼠标框选画布，完成后按回车键确认");
    update();
}

void ImageCanvas::stopCanvasSelection()
{
    currentState = CanvasState::Idle;
    update();
}

void ImageCanvas::startDrawing()
{
    if (selectedCanvas.isValid() && !processedImage.isNull()) {
        generateDrawingPoints();
        currentPointIndex = 0;
        currentState = CanvasState::Drawing;
        drawTimer->start(1000 / drawSpeed);
        emit statusUpdated("开始绘画，+加速，-减速，ESC停止");
    } else {
        emit statusUpdated("请先选择图片和画布");
    }
}

void ImageCanvas::stopDrawing()
{
    drawTimer->stop();
    currentState = CanvasState::Idle;
    emit statusUpdated("绘画已停止");
    update();
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
        QRect imageRect = QRect(0, 0, width() / 2, height());
        QImage scaledImage = originalImage.scaled(imageRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QRect drawRect = scaledImage.rect();
        drawRect.moveCenter(imageRect.center());
        painter.drawImage(drawRect, scaledImage);
        
        if (!processedImage.isNull()) {
            QRect processedRect = QRect(width() / 2, 0, width() / 2, height());
            QImage scaledProcessed = processedImage.scaled(processedRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QRect drawProcessedRect = scaledProcessed.rect();
            drawProcessedRect.moveCenter(processedRect.center());
            painter.drawImage(drawProcessedRect, scaledProcessed);
        }
    }
    
    if (currentState == CanvasState::SelectingCanvas && !selectionStart.isNull() && !selectionEnd.isNull()) {
        QPen pen(Qt::red, 2, Qt::DashLine);
        painter.setPen(pen);
        painter.drawRect(QRect(selectionStart, selectionEnd).normalized());
    }
    
    if (!selectedCanvas.isNull()) {
        QPen pen(Qt::green, 2, Qt::SolidLine);
        painter.setPen(pen);
        painter.drawRect(selectedCanvas);
    }
}

void ImageCanvas::mousePressEvent(QMouseEvent *event)
{
    if (currentState == CanvasState::SelectingCanvas && event->button() == Qt::LeftButton) {
        selectionStart = event->pos();
        selectionEnd = event->pos();
        update();
    }
}

void ImageCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if (currentState == CanvasState::SelectingCanvas) {
        selectionEnd = event->pos();
        update();
    }
}

void ImageCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (currentState == CanvasState::SelectingCanvas && event->button() == Qt::LeftButton) {
        selectionEnd = event->pos();
        update();
    }
}

void ImageCanvas::keyPressEvent(QKeyEvent *event)
{
    if (currentState == CanvasState::SelectingCanvas && event->key() == Qt::Key_Return) {
        selectedCanvas = QRect(selectionStart, selectionEnd).normalized();
        if (selectedCanvas.isValid()) {
            emit canvasConfirmed(selectedCanvas);
            emit statusUpdated(QString("画布已确认: (%1, %2) - %3x%4")
                               .arg(selectedCanvas.x())
                               .arg(selectedCanvas.y())
                               .arg(selectedCanvas.width())
                               .arg(selectedCanvas.height()));
        }
        currentState = CanvasState::Idle;
        update();
    } else if (event->key() == Qt::Key_Escape) {
        if (currentState == CanvasState::Drawing) {
            stopDrawing();
        } else if (currentState == CanvasState::SelectingCanvas) {
            stopCanvasSelection();
            emit statusUpdated("已取消画布选择");
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
        
        QCursor::setPos(point);
        
        QWindow *window = QGuiApplication::topLevelAt(point);
        if (window) {
            QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            QCoreApplication::sendEvent(window, &pressEvent);
            
            QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            QCoreApplication::sendEvent(window, &releaseEvent);
        }
        
        currentPointIndex++;
        update();
        
        if (currentPointIndex >= drawingPoints.size()) {
            stopDrawing();
            emit drawingFinished();
            emit statusUpdated("绘画完成");
        }
    }
}

void ImageCanvas::processImage()
{
    if (originalImage.isNull()) return;
    
    processedImage = originalImage.convertToFormat(QImage::Format_Grayscale8);
    
    QImage edgeImage(processedImage.size(), QImage::Format_Grayscale8);
    int threshold = 128;
    
    for (int y = 0; y < processedImage.height(); y++) {
        for (int x = 0; x < processedImage.width(); x++) {
            int gray = qGray(processedImage.pixel(x, y));
            if (gray < threshold) {
                edgeImage.setPixel(x, y, qRgb(0, 0, 0));
            } else {
                edgeImage.setPixel(x, y, qRgb(255, 255, 255));
            }
        }
    }
    
    processedImage = edgeImage;
}

void ImageCanvas::generateDrawingPoints()
{
    drawingPoints.clear();
    
    if (processedImage.isNull() || !selectedCanvas.isValid()) return;
    
    QImage scaledImage = processedImage.scaled(selectedCanvas.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    for (int y = 0; y < scaledImage.height(); y++) {
        for (int x = 0; x < scaledImage.width(); x++) {
            if (qGray(scaledImage.pixel(x, y)) < 128) {
                drawingPoints.append(QPoint(selectedCanvas.x() + x, selectedCanvas.y() + y));
            }
        }
    }
}
