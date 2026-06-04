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
    Drawing
};

class ImageCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit ImageCanvas(QWidget *parent = nullptr);
    void loadImage(const QString &path);
    void startScreenSelection();
    void startDrawing();
    void stopDrawing();
    void setDrawSpeed(int speed);
    int getDrawSpeed() const;
    QRect getSelectedCanvas() const;
    QImage getProcessedImage() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void canvasConfirmed(const QRect &rect);
    void drawingFinished();
    void statusUpdated(const QString &message);

private slots:
    void drawNextPoint();
    void onScreenSelectionConfirmed(const QRect &rect);
    void onScreenSelectionCancelled();

private:
    void processImage();
    void generateDrawingPoints();
    void simulateMouseClick(const QPoint &point);

    QImage originalImage;
    QImage processedImage;
    QRect selectedCanvas;
    CanvasState currentState;
    QTimer *drawTimer;
    QList<QPoint> drawingPoints;
    int currentPointIndex;
    int drawSpeed;
    bool imageLoaded;
    int skipPoints;  // 跳跃采样点数
};

#endif // IMAGECANVAS_H
