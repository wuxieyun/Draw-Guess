#include "screenselector.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QScreen>
#include <QGuiApplication>

ScreenSelector::ScreenSelector(QWidget *parent)
    : QWidget(parent),
      isSelecting(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    
    // 全屏显示
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        setGeometry(screen->availableGeometry());
    }
}

void ScreenSelector::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    
    // 画半透明黑色背景
    painter.fillRect(rect(), QColor(0, 0, 0, 128));
    
    // 如果有选择区域，把该区域挖空
    if (!selectionStart.isNull() && !selectionEnd.isNull()) {
        QRect selectRect = QRect(selectionStart, selectionEnd).normalized();
        
        // 挖空选择区域
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.fillRect(selectRect, Qt::transparent);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        
        // 画红色边框
        QPen pen(Qt::red, 2, Qt::SolidLine);
        painter.setPen(pen);
        painter.drawRect(selectRect);
        
        // 显示尺寸信息
        painter.setPen(Qt::white);
        QFont font;
        font.setPointSize(12);
        font.setBold(true);
        painter.setFont(font);
        
        QString infoText = QString("%1 x %2").arg(selectRect.width()).arg(selectRect.height());
        QRect textRect = selectRect.adjusted(5, 5, 0, 0);
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignTop, infoText);
    } else {
        // 显示提示信息
        painter.setPen(Qt::white);
        QFont font;
        font.setPointSize(16);
        font.setBold(true);
        painter.setFont(font);
        
        QString hintText = "拖动鼠标选择屏幕区域作为绘画画板，按ESC取消，按Enter确认";
        QRect textRect = rect().adjusted(0, 50, 0, 0);
        painter.drawText(textRect, Qt::AlignTop | Qt::AlignHCenter, hintText);
    }
}

void ScreenSelector::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        selectionStart = event->pos();
        selectionEnd = event->pos();
        isSelecting = true;
        update();
    }
}

void ScreenSelector::mouseMoveEvent(QMouseEvent *event)
{
    if (isSelecting) {
        selectionEnd = event->pos();
        update();
    }
}

void ScreenSelector::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isSelecting) {
        selectionEnd = event->pos();
        isSelecting = false;
        update();
    }
}

void ScreenSelector::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit selectionCancelled();
        close();
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (!selectionStart.isNull() && !selectionEnd.isNull()) {
            selectedRect = QRect(selectionStart, selectionEnd).normalized();
            if (selectedRect.isValid()) {
                emit selectionConfirmed(selectedRect);
                close();
            }
        }
    }
}
