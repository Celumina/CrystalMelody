#include "CrystalImageViewer.h"
#include <QResizeEvent>
#include <QByteArray>

CrystalImageViewer::CrystalImageViewer(QWidget* parentWidget) : 
	QLabel(parentWidget) {
	setScaledContents(true);
	setMouseTracking(true);
	setWindowIcon(QIcon(":/CrystalMelody/Resource/Graphics/icon.ico"));
	setWindowTitle("CrystalMelody Image Viewer");
	// setWindowFlags(Qt::FramelessWindowHint | Qt::MSWindowsFixedSizeDialogHint);
	setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);
}

void CrystalImageViewer::setDisplaySource(const QString& path) {
	presetDisplay();
	if (path.endsWith(".gif", Qt::CaseInsensitive)) {
		movie.reset(new QMovie(path));
		setMovie(movie.get());
	}
	else {
		QPixmap pixmap(path);
		setPixmap(pixmap);
		resize(pixmap.size());
	}
	
}

void CrystalImageViewer::setDisplayData(const CrystalPackage::Index& index) {
	presetDisplay();
	QByteArray bytes(CrystalPackage::Stream(index).data(), index.size());

	if (QString(index.name().data()).endsWith(".gif", Qt::CaseInsensitive)) {
		movieBuffer.reset(new QBuffer);
		movieBuffer->setData(bytes);
		movie.reset(new QMovie(movieBuffer.get()));
		setMovie(movie.get());
		movie->start();
	}
	else {
		QPixmap pixmap;
		pixmap.loadFromData(bytes);
		setPixmap(pixmap);
		resize(pixmap.size());
	}

}

void CrystalImageViewer::display() {
	if (movie) {
		movie->start();
		resize(movie->currentPixmap().size());
	}
	show();
}

void CrystalImageViewer::wheelEvent(QWheelEvent* wheelEvent) {
	auto windowSize = size();
	qreal ratio = static_cast<qreal>(size().width()) / size().height();
	if (width() > minimumWidth || wheelEvent->angleDelta().y() > 0) {
		windowSize.setWidth(width() + wheelEvent->angleDelta().y() / 4);
		windowSize.setHeight(windowSize.width() / ratio);
	}
	auto cursorRelativePos = QCursor().pos();
	cursorRelativePos -= pos();
	qreal standardizedCursorValue[2] = {static_cast<qreal>(cursorRelativePos.x()) / size().width(), static_cast<qreal>(cursorRelativePos.y()) / size().height()};
	auto offsetPos = cursorRelativePos - QPoint(windowSize.width() * standardizedCursorValue[0], windowSize.height() * standardizedCursorValue[1]);
	QPoint windowPos = pos() + offsetPos;
	setGeometry(windowPos.x(), windowPos.y(), windowSize.width(), windowSize.height());
}

void CrystalImageViewer::closeEvent(QCloseEvent* closeEvent) {
	movieBuffer.reset();
	movie.reset();
	QLabel::closeEvent(closeEvent);
}

void CrystalImageViewer::mousePressEvent(QMouseEvent* mouseEvent) {
	if (mouseEvent->button() == Qt::LeftButton) {
		mousePressedPos = mouseEvent->pos();
		isMousePressed = true;
	}
	if (mouseEvent->button() == Qt::MouseButton::RightButton) {
		toggleWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	}
	QLabel::mousePressEvent(mouseEvent);
}

void CrystalImageViewer::mouseMoveEvent(QMouseEvent* mouseEvent) {
	if (!isMousePressed) {
		return;
	}
	move(pos() + mouseEvent->pos() - mousePressedPos);
	QLabel::mouseMoveEvent(mouseEvent);
}

void CrystalImageViewer::mouseReleaseEvent(QMouseEvent* mouseEvent) {
	isMousePressed = false;
	QLabel::mouseReleaseEvent(mouseEvent);
}

void CrystalImageViewer::mouseDoubleClickEvent(QMouseEvent* mouseEvent) {
	// : DEV add a bottom mode
	toggleWindowFlags(Qt::WindowStaysOnTopHint);
}

void CrystalImageViewer::enterEvent(QEnterEvent* enterEvent) {
	// : set static_cast<Qt::WindowType>(0) to recover window flags
	// setWindowFlags(static_cast<Qt::WindowType>(0));
	// show();
}

void CrystalImageViewer::leaveEvent(QEvent* leaveEvent) {

}

void CrystalImageViewer::presetDisplay() {
	movieBuffer.reset();
	movie.reset();
	if (pos().x() < 0 || pos().y() < 0) {
		setGeometry(0, 0, size().width(), size().height());
	}
}

void CrystalImageViewer::toggleWindowFlags(Qt::WindowFlags flags) {
	if ((static_cast<int>(windowFlags()) & static_cast<int>(flags)) == 0) {
		setWindowFlags(windowFlags() | flags);
		show();
	}
	else {
		setWindowFlags(windowFlags() ^ flags);
		show();
	}
}
