#pragma once
#include <QVideoWidget>
class CrystalVideoWidget :
    public QVideoWidget {
    Q_OBJECT

public:
    CrystalVideoWidget(QWidget* parentWidget = nullptr);
    void display(const QSize& displaySize = { 1280, 720 }); // default window size
    void adaptiveFullScreen(bool fullScreen);

signals:
    void closed();

protected:
    void mouseDoubleClickEvent(QMouseEvent* mouseEvent) override;
    void closeEvent(QCloseEvent* closeEvent) override;

private:
    QSize videoSize = { 1280, 720 };
};

