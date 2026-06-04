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
#include <QTimer>
#include <QImageReader>

#ifdef _WIN32
#include <windows.h>
#endif

ImageCanvas::ImageCanvas(QWidget *parent)
    : QWidget(parent),
      currentState(CanvasState::Idle),
      drawTimer(new QTimer(this)),
      currentPointIndex(0),
      drawSpeed(8),
      imageLoaded(false),
      skipPoints(3)  // 进一步提高效率
{
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    connect(drawTimer, &QTimer::timeout, this, &ImageCanvas::drawNextPoint);
    
    // 确保窗口能接收全局按键事件
    grabKeyboard();
}

ImageCanvas::~ImageCanvas()
{
    releaseKeyboard();
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
    
    // 使用 QImageReader 尝试多种格式加载
    originalImage.load(path);
    if (originalImage.isNull()) {
        // 尝试其他加载方式
        QImageReader reader(path);
        reader.setAutoDetectImageFormat(true);
        originalImage = reader.read();
    }
    
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
        
        // 确保窗口能接收事件
        setFocus();
        activateWindow();
        raise();
        
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
    
    // 确保释放鼠标按钮
#ifdef _WIN32
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
#endif
    
    currentState = CanvasState::Idle;
    emit statusUpdated("绘画已停止");
    qDebug() << "绘画已停止";
}

void ImageCanvas::setDrawSpeed(int speed)
{
    drawSpeed = qBound(1, speed, 30);  // 进一步降低最高速度，更稳定
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
    qDebug() << "Key pressed:" << event->key();
    
    // 检查 ESC 键，无论状态如何都尝试停止
    if (event->key() == Qt::Key_Escape) {
        qDebug() << "ESC pressed, stopping...";
        if (currentState == CanvasState::Drawing) {
            stopDrawing();
        }
        return;
    } else if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
        setDrawSpeed(drawSpeed + 3);
        return;
    } else if (event->key() == Qt::Key_Minus) {
        setDrawSpeed(drawSpeed - 3);
        return;
    }
    
    QWidget::keyPressEvent(event);
}

void ImageCanvas::drawNextPoint()
{
    if (currentPointIndex < drawingPoints.size()) {
        QPoint point = drawingPoints[currentPointIndex];
        
        // 移动鼠标并拖拽
        simulateMouseDrag(point);
        
        currentPointIndex++;
        
        if (currentPointIndex % 50 == 0) {
            emit statusUpdated(QString("绘画进度: %1/%2").arg(currentPointIndex).arg(drawingPoints.size()));
        }
        
        if (currentPointIndex >= drawingPoints.size()) {
            stopDrawing();
            emit drawingFinished();
            emit statusUpdated("绘画完成！");
        }
    }
}

void ImageCanvas::simulateMouseDrag(const QPoint &point)
{
#ifdef _WIN32
    // Windows 平台：使用 SendInput 模拟拖拽绘画
    static bool isMouseDown = false;
    static QPoint lastPoint;
    
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    
    // 移动鼠标到目标位置
    input.mi.dx = (LONG)((point.x() * 65535.0f) / (GetSystemMetrics(SM_CXSCREEN) - 1));
    input.mi.dy = (LONG)((point.y() * 65535.0f) / (GetSystemMetrics(SM_CYSCREEN) - 1));
    input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    SendInput(1, &input, sizeof(INPUT));
    
    // 如果还没按下鼠标，先按下
    if (!isMouseDown) {
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(INPUT));
        isMouseDown = true;
        QThread::msleep(5);
    }
    
    // 短暂延迟
    QThread::msleep(3);
    
    // 检查是否是行的结尾或需要抬起
    // 如果 Y 坐标变化较大，说明换行了，需要短暂抬起
    if (!lastPoint.isNull() && abs(point.y() - lastPoint.y()) > 2) {
        // 短暂释放鼠标再按下，模拟换行
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(INPUT));
        QThread::msleep(3);
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(INPUT));
    }
    
    lastPoint = point;
    
    // 在绘画结束时确保释放鼠标
    if (currentPointIndex >= drawingPoints.size() - 1) {
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(INPUT));
        isMouseDown = false;
    }
#else
    // 非 Windows 平台：只移动鼠标
    QCursor::setPos(point);
    QCoreApplication::processEvents();
    QThread::msleep(1);
#endif
}

void ImageCanvas::processImage()
{
    if (originalImage.isNull()) return;
    
    // 确保是 RGB 格式
    QImage rgbImage;
    if (originalImage.format() != QImage::Format_RGB32 && 
        originalImage.format() != QImage::Format_ARGB32) {
        rgbImage = originalImage.convertToFormat(QImage::Format_RGB32);
    } else {
        rgbImage = originalImage;
    }
    
    // 转换为灰度图
    QImage grayImage = rgbImage.convertToFormat(QImage::Format_Grayscale8);
    
    // 二值化处理
    processedImage = QImage(grayImage.size(), QImage::Format_Mono);
    processedImage.setColorCount(2);
    processedImage.setColor(0, qRgb(255, 255, 255));
    processedImage.setColor(1, qRgb(0, 0, 0));
    
    int threshold = 150;  // 提高阈值，获得更清晰的线条
    
    for (int y = 0; y < grayImage.height(); y++) {
        for (int x = 0; x < grayImage.width(); x++) {
            int gray = qGray(grayImage.pixel(x, y));
            processedImage.setPixel(x, y, gray < threshold ? 1 : 0);
        }
    }
    
    qDebug() << "图片处理完成";
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
    
    qDebug() << "开始收集绘画点，画布大小:" << selectedCanvas << "图片缩放后大小:" << scaledImage.size();
    
    // 扫描线方式收集点，按行和列的顺序
    for (int y = 0; y < scaledImage.height(); y++) {
        QList<QPoint> linePoints;
        
        for (int x = 0; x < scaledImage.width(); x++) {
            if (qGray(scaledImage.pixel(x, y)) < 128) {
                linePoints.append(QPoint(x, y));
            }
        }
        
        // 对每一行的点进行下采样，减少数量
        for (int i = 0; i < linePoints.size(); i += (skipPoints + 1)) {
            QPoint p = linePoints[i];
            QPoint globalPoint(selectedCanvas.x() + p.x() + offsetX, 
                              selectedCanvas.y() + p.y() + offsetY);
            drawingPoints.append(globalPoint);
        }
    }
    
    qDebug() << "生成了" << drawingPoints.size() << "个绘画点";
}
