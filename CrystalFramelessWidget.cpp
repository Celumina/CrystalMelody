#include "CrystalFramelessWidget.h"
#include <QMouseEvent>
#include <QGuiApplication>

#include "DebugLibrary.h"

CrystalFramelessWidget::CrystalFramelessWidget(QWidget* parent) : 
QWidget(parent), 
_borderFlag(BorderNone) {

	setWindowFlags(Qt::FramelessWindowHint);

	// client do 
	// setAttribute(Qt::WA_TranslucentBackground);
	// setAttribute(Qt::WA_Hover);

	setMinimumSize(200, 100);
}

CrystalFramelessWidget::~CrystalFramelessWidget() {
}

void CrystalFramelessWidget::mousePressEvent(QMouseEvent* mouseEvent) {
	updateBorderFlag();
	if (mouseEvent->button() == Qt::LeftButton) {
		// move window reg
		_relativePressedPos = mouseEvent->globalPos() - pos();
		_moveEnabled = true;
	}
	QWidget::mousePressEvent(mouseEvent);
}

// don' t use mouseEvent->pos(), it return a position which is unstable.
void CrystalFramelessWidget::mouseMoveEvent(QMouseEvent* mouseEvent) {
	QWidget::mouseMoveEvent(mouseEvent);
	if (_borderFlag && _flexible) {
		showNormal();

		auto newSize = size();
		auto newPos = pos();
		if (_borderFlag & BorderRight) {
			setCursor(Qt::SizeHorCursor);
			newSize.setWidth(mouseEvent->globalPos().x() - x());
		}
		if (_borderFlag & BorderBottom) {
			setCursor(Qt::SizeVerCursor);
			newSize.setHeight(mouseEvent->globalPos().y() - y());
		}
		if (_borderFlag & BorderLeft) {
			setCursor(Qt::SizeHorCursor);
			newPos.setX(mouseEvent->globalPos().x() - _relativePressedPos.x());
			newSize.setWidth(x() - newPos.x() + width());
		}
		if (_borderFlag & BorderTop) {
			setCursor(Qt::SizeVerCursor);
			newPos.setY(mouseEvent->globalPos().y() - _relativePressedPos.y());
			newSize.setHeight(y() - newPos.y() + height());
		}


		if (_borderFlag == BorderTopRight || _borderFlag == BorderBottomLeft) {
			setCursor(Qt::SizeBDiagCursor);
		}
		else if (_borderFlag == BorderTopLeft || _borderFlag == BorderBottomRight) {
			setCursor(Qt::SizeFDiagCursor);
		}

		move(newPos);
		resize(newSize);

	}
	else if (_moveEnabled) {
		QSize screenSize = QGuiApplication::primaryScreen()->size();
		if (isMaximized()) {
			_relativePressedPos.setX(static_cast<float>(mouseEvent->globalPos().x()) / width() * normalGeometry().width());
			
		}
		else if (_halfScreenState) {
			_relativePressedPos.setX(static_cast<float>(mouseEvent->globalPos().x()) / width() * _standardSize.width() / 2);
			_relativePressedPos.setY(static_cast<float>(mouseEvent->globalPos().y()) / height() * _standardSize.height());
		}

		if (!isMaximized() && !_halfScreenState) {
			move(mouseEvent->globalPos() - _relativePressedPos);
		}
		

		if (!_flexible) {
			return;
		}

		if (isMaximized() && mouseEvent->globalPos().y() > _borderWidth) {
			showNormal();
		}

		if (_halfScreenState &&
			mouseEvent->globalPos().x() < screenSize.width() - 1 - _borderWidth &&
			mouseEvent->globalPos().x() > _borderWidth) {
			_halfScreenState = false;
			resize(_standardSize);
		}

		if (mouseEvent->globalPos().y() == 0) {
			showMaximized();
		}
		else if (mouseEvent->globalPos().x() == 0 && !_halfScreenState) {
			_standardSize = size();
			// Set _halfScreenState before setGeometry, because resizeEvent may want to be get halfScreenState.
			_halfScreenState = true;
			setGeometry(
				0,
				0,
				screenSize.width() / 2,
				screenSize.height()
			);
			
		}
		else if (mouseEvent->globalPos().x() == screenSize.width() - 1 && !_halfScreenState) {
			_standardSize = size();
			_halfScreenState = true;
			setGeometry(screenSize.width() / 2,
				0,
				screenSize.width() / 2,
				screenSize.height()
			);
		}
	}
}

void CrystalFramelessWidget::mouseReleaseEvent(QMouseEvent* mouseEvent) {
	if (_borderFlag) {
		setCursor(Qt::ArrowCursor);
	}
	_moveEnabled = false;
	QWidget::mouseReleaseEvent(mouseEvent);
}

void CrystalFramelessWidget::mouseDoubleClickEvent(QMouseEvent* mouseEvent) {
	if (_flexible) {
		if (isMaximized()) {
			showNormal();
		}
		else {
			showMaximized();
		}
	}
	QWidget::mouseDoubleClickEvent(mouseEvent);
}

void CrystalFramelessWidget::resizeEvent(QResizeEvent* _event) {
	QWidget::resizeEvent(_event);
}

bool CrystalFramelessWidget::flexible() {
	return _flexible;
}

uint8_t CrystalFramelessWidget::borderWidth() {
	return _borderWidth;
}

uint8_t CrystalFramelessWidget::cornerScale() {
	return _cornerScale;
}

bool CrystalFramelessWidget::halfScreenState() {
	return _halfScreenState;
}

void CrystalFramelessWidget::setFlexible(bool flexible) {
	_flexible = flexible;
}

void CrystalFramelessWidget::setBorderWidth(uint8_t borderWidth) {
	_borderWidth = borderWidth;
}

void CrystalFramelessWidget::setCornerScale(uint8_t cornerScale) {
	_cornerScale = cornerScale;
}

void CrystalFramelessWidget::updateBorderFlag() {
	_borderFlag = BorderNone;

	if (width() + x() - QCursor::pos().x() <= _borderWidth * _cornerScale &&
		QCursor::pos().y() - y() <= _borderWidth * _cornerScale) {
		_borderFlag = BorderTopRight;
	}
	else if (QCursor::pos().x() - x() <= _borderWidth * _cornerScale &&
		QCursor::pos().y() - y() <= _borderWidth * _cornerScale
		) {
		_borderFlag = BorderTopLeft;
	}
	else if (width() + x() - QCursor::pos().x() <= _borderWidth * _cornerScale &&
		height() + y() - QCursor::pos().y() <= _borderWidth * _cornerScale) {
		_borderFlag = BorderBottomRight;
	}
	else if (QCursor::pos().x() - x() <= _borderWidth * _cornerScale &&
		height() + y() - QCursor::pos().y() <= _borderWidth * _cornerScale) {
		_borderFlag = BorderBottomLeft;
	}
	else {
		// right side
		if (width() + x() - QCursor::pos().x() <= _borderWidth) {
			_borderFlag = BorderRight;
		}
		// bottom side
		if (height() + y() - QCursor::pos().y() <= _borderWidth) {
			_borderFlag = BorderBottom;
		}
		// left side
		if (QCursor::pos().x() - x() <= _borderWidth) {
			_borderFlag =  BorderLeft;
		}
		// top side
		if (QCursor::pos().y() - y() <= _borderWidth) {
			_borderFlag = BorderTop;
		}
	}
}
