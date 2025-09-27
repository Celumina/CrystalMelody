#include "CrystalVideoWidget.h"
#include <QCloseEvent>
#include <QApplication>

CrystalVideoWidget::CrystalVideoWidget(QWidget* parentWidget) :
QVideoWidget(parentWidget) {
	setWindowIcon(QIcon(":/CrystalMelody/Resource/Graphics/icon.ico"));
	setWindowTitle("CrystalMelody Video Widget");
}

void CrystalVideoWidget::display(const QSize& displaySize) {
	videoSize = displaySize;

	if (size() != videoSize && !isMaximized() && !isFullScreen()) {
		resize(videoSize);
	}
	show();
}

void CrystalVideoWidget::adaptiveFullScreen(bool fullScreen) {
	if (fullScreen) {
		resize(QGuiApplication::primaryScreen()->geometry().size());	// <- both of these code can't be reverse
		setFullScreen(true);
	}
	else {
		setFullScreen(false); // <- both of these code can't be reverse
		resize(videoSize);
	}
}

void CrystalVideoWidget::mouseDoubleClickEvent(QMouseEvent* mouseEvent) {
	adaptiveFullScreen(!isFullScreen());
	QVideoWidget::mouseDoubleClickEvent(mouseEvent);
}

void CrystalVideoWidget::closeEvent(QCloseEvent* closeEvent) {
	emit closed();
	hide(); 

	// only hide, but not close, for slove white video window problem
	closeEvent->ignore();
	// QVideoWidget::closeEvent(closeEvent);
}

