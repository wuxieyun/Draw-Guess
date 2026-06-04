#include "screenselector.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QStyle>
#include <QDebug>

ScreenSelector::ScreenSelector(QWidget *parent)
    : QDialog(parent),
      isSelecting(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setModal(true);
    
    // 全屏显示
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        setGeometry(screen->availableGeometry());
    }
    
    // 创建按钮容器
    buttonWidget = new QWidget(this);
    buttonWidget->setStyleSheet("background-color: rgba(255, 255, 255, 200); border-radius: 10px; padding: 10px;");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(buttonWidget);
    
    // 提示标签
    hintLabel = new QLabel("拖动鼠标选择区域，然后点击确认或按 Enter");
    hintLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333; padding: 5px;");
    mainLayout->addWidget(hintLabel);
    
    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    confirmButton = new QPushButton("确认 (Enter)");
    confirmButton->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; border: none; padding: 10px 20px; font-size: 14px; border-radius: 5px; }"
        "QPushButton:hover { background-color: #45a049; }"
    );
    
    cancelButton = new QPushButton("取消 (ESC)");
    cancelButton->setStyleSheet(
        "QPushButton { background-color: #f44336; color: white; border: none; padding: 10px 20px; font-size: 14px; border-radius: 5px; }"
        "QPushButton:hover { background-color: #d32f2f; }"
    );
    
    buttonLayout->addWidget(confirmButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);
    
    // 将按钮容器放在屏幕底部中央
    buttonWidget->move(width() / 2 - buttonWidget->sizeHint().width() / 2, height() - 100);
    
    // 连接信号
    connect(confirmButton, &QPushButton::clicked, this, &ScreenSelector::onConfirmClicked);
    connect(cancelButton, &QPushButton::clicked, this, &ScreenSelector::onCancelClicked);
    
    qDebug() << "ScreenSelector 构造完成";
}

void ScreenSelector::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    qDebug() << "ScreenSelector 显示";
    activateWindow();
    raise();
    setFocus();
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
        QPen pen(Qt::red, 3, Qt::SolidLine);
        painter.setPen(pen);
        painter.drawRect(selectRect);
        
        // 显示尺寸信息
        painter.setPen(Qt::white);
        QFont font;
        font.setPointSize(14);
        font.setBold(true);
        painter.setFont(font);
        
        QString infoText = QString("%1 x %2").arg(selectRect.width()).arg(selectRect.height());
        QRect textRect = selectRect.adjusted(5, 5, 0, 0);
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignTop, infoText);
    } else {
        // 显示提示信息
        painter.setPen(Qt::white);
        QFont font;
        font.setPointSize(18);
        font.setBold(true);
        painter.setFont(font);
        
        QString hintText = "请用鼠标拖动选择绘画区域\n选择完成后点击确认按钮或按 Enter 键";
        QRect textRect = rect().adjusted(0, 50, 0, 0);
        painter.drawText(textRect, Qt::AlignTop | Qt::AlignHCenter, hintText);
    }
}

void ScreenSelector::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "mousePressEvent at:" << event->pos();
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
    qDebug() << "mouseReleaseEvent at:" << event->pos();
    if (event->button() == Qt::LeftButton && isSelecting) {
        selectionEnd = event->pos();
        isSelecting = false;
        update();
        
        if (!selectionStart.isNull() && !selectionEnd.isNull()) {
            hintLabel->setText("选择完成！点击确认或按 Enter");
        }
    }
}

void ScreenSelector::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "keyPressEvent, key:" << event->key();
    
    if (event->key() == Qt::Key_Escape) {
        qDebug() << "ESC pressed, cancelling";
        onCancelClicked();
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        qDebug() << "Enter/Return pressed, confirming";
        onConfirmClicked();
    }
}

void ScreenSelector::onConfirmClicked()
{
    qDebug() << "Confirm clicked";
    if (!selectionStart.isNull() && !selectionEnd.isNull()) {
        selectedRect = QRect(selectionStart, selectionEnd).normalized();
        
        if (selectedRect.isValid() && selectedRect.width() > 20 && selectedRect.height() > 20) {
            qDebug() << "Selection confirmed:" << selectedRect;
            accept(); // 接受对话框
        } else {
            hintLabel->setText("选择区域太小，请重新选择更大的区域！");
        }
    } else {
        hintLabel->setText("请先选择一个区域！");
    }
}

void ScreenSelector::onCancelClicked()
{
    qDebug() << "Cancel clicked";
    selectedRect = QRect(); // 清空选择
    reject(); // 拒绝对话框
}
