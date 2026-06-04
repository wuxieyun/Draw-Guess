#ifndef SCREENSELECTOR_H
#define SCREENSELECTOR_H

#include <QDialog>
#include <QPoint>
#include <QRect>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ScreenSelector : public QDialog
{
    Q_OBJECT

public:
    explicit ScreenSelector(QWidget *parent = nullptr);
    QRect getSelectedRect() const { return selectedRect; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void onConfirmClicked();
    void onCancelClicked();

private:
    QPoint selectionStart;
    QPoint selectionEnd;
    QRect selectedRect;
    bool isSelecting;
    
    QPushButton *confirmButton;
    QPushButton *cancelButton;
    QLabel *hintLabel;
    QWidget *buttonWidget;
};

#endif // SCREENSELECTOR_H
