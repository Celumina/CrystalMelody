#include "CrystalSlider.h"

#include <limits>

CrystalSlider::CrystalSlider(QWidget* parentWidget) :
	QSlider(parentWidget) {}

bool CrystalSlider::isTriggered() {
	return triggered;
}

bool CrystalSlider::isWheelChanged() {
	return wheelChanged;
}

void CrystalSlider::updateStatus() {
	if (wheelStatus == WheelStatus::loading) {
		wheelStatus = WheelStatus::ready;
	}
	else if (wheelStatus == WheelStatus::ready) {
		triggered = false;
		wheelStatus = WheelStatus::invalid;
	}
	wheelChanged = false;
}
// �ڳ�ʼ���б��е��� QSlider ���캯������˼��: ʹ�ò��� paraParent �������� �̳��� QSlider �Ĳ��ֳ�ʼ��

void CrystalSlider::mousePressEvent(QMouseEvent* mouseEvent) {
	triggered = true;
	int currentX = mouseEvent->x();
	double scalar = 1.0 / width();
	double percent = std::max(currentX * scalar - scalar, 0.0);
	int value = percent * (maximum() - minimum()) + minimum() + handleWidth * percent;
	setValue(value);

	QSlider::mousePressEvent(mouseEvent);	//  ���� QSlider ��ɷֵ� mousePressEvent, ʹ QSlider ����������¼���������Ӧ
}

void CrystalSlider::mouseReleaseEvent(QMouseEvent* mouseEvent) {
	triggered = false;

	QSlider::mouseReleaseEvent(mouseEvent);
}

void CrystalSlider::wheelEvent(QWheelEvent* wheelEvent) {
	triggered = true;
	wheelChanged = true;
	wheelStatus = WheelStatus::loading;
	QSlider::wheelEvent(wheelEvent);
}


