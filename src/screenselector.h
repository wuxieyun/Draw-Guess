#ifndef SCREENSELECTOR_H
#define SCREENSELECTOR_H

#include <QWidget>
#include <QPoint>
#include <QRect>

class ScreenSelector : public QWidget
{
    Q_OBJECT

public:
    explicit ScreenSelector(QWidget *parent = nullptr);
    QRect getSelectedRect() const { return selectedRect; }

signals:
    void selectionConfirmed(const QRect &rect);
    void selectionCancelled();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QPoint selectionStart;
    QPoint selectionEnd;
    QRect selectedRect;
    bool isSelecting;
};

#endif // SCREENSELECTOR_H
