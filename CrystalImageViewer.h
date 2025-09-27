#pragma once
#include <QLabel>
#include <QMovie>
#include  <QBuffer>
#include "CrystalPackage.h"

class CrystalImageViewer : public QLabel {
public:
    enum MagicNumber {
        minimumWidth = 200
    };

    CrystalImageViewer(QWidget* parentWidget = nullptr);

    void setDisplaySource(const QString& path);
    void setDisplayData(const CrystalPackage::Index& index);
    void display();

protected:
    void wheelEvent(QWheelEvent* wheelEvent) override;
    void closeEvent(QCloseEvent* closeEvent) override;

    void mousePressEvent(QMouseEvent* mouseEvent) override;
    void mouseMoveEvent(QMouseEvent* mouseEvent) override;
    void mouseReleaseEvent(QMouseEvent* mouseEvent) override;
    void mouseDoubleClickEvent(QMouseEvent* mouseEvent) override;

    void enterEvent(QEnterEvent* enterEvent) override;
    void leaveEvent(QEvent* leaveEvent) override;

private:
    void presetDisplay();
    void toggleWindowFlags(Qt::WindowFlags flag);
    std::unique_ptr<QMovie> movie = nullptr;
    std::unique_ptr <QBuffer> movieBuffer = nullptr;

    bool isMousePressed = false;
    QPoint mousePressedPos;
};

