#pragma once
#include <QWidget>

class CrystalFramelessWidget : public QWidget {
public:
	CrystalFramelessWidget(QWidget* parent = nullptr);
	~CrystalFramelessWidget();

protected:
	void mousePressEvent(QMouseEvent* _event) override;
	void mouseMoveEvent(QMouseEvent* _event) override;
	void mouseReleaseEvent(QMouseEvent* _event) override;
	void mouseDoubleClickEvent(QMouseEvent* _event) override;

	void resizeEvent(QResizeEvent* _event);


	bool flexible();
	uint8_t borderWidth();
	uint8_t cornerScale();
	bool halfScreenState();


	void setFlexible(bool flexible);
	void setBorderWidth(uint8_t borderWidth);
	void setCornerScale(uint8_t cornerScale);

private:
	enum BorderFlag : uint8_t {
		BorderNone =						0b0000'0000, 
		BorderTop =						0b0000'0001, 
		BorderBottom =					0b0000'0010,
		BorderLeft =						0b0000'0100,
		BorderRight =						0b0000'1000,
		BorderTopLeft =					0b0000'0101,
		BorderTopRight =				0b0000'1001,
		BorderBottomLeft =			0b0000'0110,
		BorderBottomRight =		0b0000'1010
	};

	void updateBorderFlag();

	// parameters
	bool _flexible = true;
	uint8_t _borderWidth = 4;
	uint8_t _cornerScale = 4; // Corner consider size with _borderWidth * _cornerScale

	// for move window
	bool _moveEnabled = false;
	QPoint _relativePressedPos;

	// for drag window borders size
	// DEV: encode these variables
	BorderFlag _borderFlag;	 // Bit bool: U, D, L, R, UL, UR, DL, DR

	// for half-screen recognize
	bool _halfScreenState = false;
	QSize _standardSize;
};

