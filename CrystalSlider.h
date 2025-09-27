#pragma once
#include <QObject>
#include <QSlider>
#include <QMouseEvent>

class CrystalSlider: public QSlider {
public:
	enum MagicNumber {
		handleWidth = 6 // for get handle width would cause too much matter, so define this magic number
	};

	enum class WheelStatus : uint16_t {
		invalid, 
		loading, 
		ready
	};

	CrystalSlider(QWidget* parentWidget = nullptr);
	bool isTriggered();
	bool isWheelChanged();

	// @brief: update isTriggered and wheel changed.
	void updateStatus();

	// void wheelSlid();

protected:
	void mousePressEvent(QMouseEvent* mouseEvent) override;
	void mouseReleaseEvent(QMouseEvent* mouseEvent) override;
	void wheelEvent(QWheelEvent* wheelEvent) override;

private:
	bool triggered = false;
	bool wheelChanged = false;
	WheelStatus wheelStatus = WheelStatus::invalid;
};

