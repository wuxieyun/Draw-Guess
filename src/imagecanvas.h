#ifndef IMAGECANVAS_H
#define IMAGECANVAS_H

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <QTimer>
#include <QCursor>

enum class CanvasState {
    Idle,
    SelectingCanvas,
    Drawing
};

class ImageCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit ImageCanvas(QWidget *parent = nullptr);
    void loadImage(const QString &path);
    void startCanvasSelection();
    void stopCanvasSelection();
    void startDrawing();
    void stopDrawing();
    void setDrawSpeed(int speed);
    int getDrawSpeed() const;
    QRect getSelectedCanvas() const;
    QImage getProcessedImage() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void canvasConfirmed(const QRect &rect);
    void drawingFinished();
    void statusUpdated(const QString &message);

private slots:
    void drawNextPoint();

private:
    void processImage();
    void generateDrawingPoints();

    QImage originalImage;
    QImage processedImage;
    QPixmap canvasPixmap;
    QPoint selectionStart;
    QPoint selectionEnd;
    QRect selectedCanvas;
    CanvasState currentState;
    QTimer *drawTimer;
    QList<QPoint> drawingPoints;
    int currentPointIndex;
    int drawSpeed;
};

#endif
