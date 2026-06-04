#include "screenselector.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>

ScreenSelector::ScreenSelector(QWidget *parent)
    : QWidget(parent),
      isSelecting(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    
    // 全屏显示
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        setGeometry(screen->availableGeometry());
    }
    
    qDebug() << "ScreenSelector 构造函数完成，准备显示";
}

void ScreenSelector::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    qDebug() << "ScreenSelector 显示，激活并获取焦点";
    activateWindow();
    raise();
    setFocus();
    qDebug() << "当前焦点:" << hasFocus();
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
        
        // 提示双击确认提示
        painter.setPen(Qt::yellow);
        QString confirmText = "按 Enter 确认选择，按 ESC 取消，或双击";
        QRect hintRect = selectRect.adjusted(0, -30, 0, 0);
        painter.drawText(selectRect.center(), confirmText);
    } else {
        // 显示提示信息
        painter.setPen(Qt::white);
        QFont font;
        font.setPointSize(16);
        font.setBold(true);
        painter.setFont(font);
        
        QString hintText = "拖动鼠标选择屏幕区域作为绘画画板\n按ESC取消，按Enter确认，或直接双击确认";
        QRect textRect = rect().adjusted(0, 50, 0, 0);
        painter.drawText(textRect, Qt::AlignTop | Qt::AlignHCenter, hintText);
    }
}

void ScreenSelector::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "mousePressEvent";
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
    qDebug() << "mouseReleaseEvent";
    if (event->button() == Qt::LeftButton && isSelecting) {
        selectionEnd = event->pos();
        isSelecting = false;
        update();
    }
}

void ScreenSelector::mouseDoubleClickEvent(QMouseEvent *event)
{
    qDebug() << "mouseDoubleClickEvent";
    Q_UNUSED(event);
    confirmSelection();
}

void ScreenSelector::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "keyPressEvent, key:" << event->key() << "Qt::Key_Return:" << (event->key() == Qt::Key_Return) << "Qt::Key_Enter:" << (event->key() == Qt::Key_Enter);
    
    if (event->key() == Qt::Key_Escape) {
        qDebug() << "ESC pressed, cancelling";
        emit selectionCancelled();
        close();
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        qDebug() << "Enter/Return pressed";
        confirmSelection();
    }
}

void ScreenSelector::confirmSelection()
{
    qDebug() << "confirmSelection called, selectionStart:" << selectionStart << "selectionEnd:" << selectionEnd;
    
    if (!selectionStart.isNull() && !selectionEnd.isNull()) {
        selectedRect = QRect(selectionStart, selectionEnd).normalized();
        qDebug() << "selectedRect:" << selectedRect << "isValid:" << selectedRect.isValid();
        
        if (selectedRect.isValid() && selectedRect.width() > 10 && selectedRect.height() > 10) {
            qDebug() << "Emitting selectionConfirmed";
            emit selectionConfirmed(selectedRect);
            close();
        } else {
            qDebug() << "Selection too small or invalid";
        }
    } else {
        qDebug() << "No selection made";
    }
}
