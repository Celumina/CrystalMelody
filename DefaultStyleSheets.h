#pragma once

namespace qss {
	using Text = const char*;

	static Text lineEditFilterOn =
		"QLineEdit{\
			color: rgb(255, 255, 255);\
			border-bottom-right-radius: 20px;\
			background: rgb(5, 110, 180);\
		}";

	static Text lineEditFilterOff =
		"QLineEdit{\
			color: rgb(120, 120, 120);\
			border-bottom-right-radius: 20px;\
			background: rgba(240, 240, 240, 128);\
		}\
		QLineEdit::hover{\
			background: rgba(255, 255, 255, 180);\
		}";

	static Text crystalSlider =
		"QSlider::groove:horizontal{\
				height: 9px;\
				border: 0px;\
				border-radius: 4.495px;\
				background: rgba(200, 200, 200, 255);\
			}\
			QSlider::handle:horizontal{\
				background: rgb(90, 80, 160);\
				width: 14px;\
				height: 14px;\
				margin: -4.5px 0px -4.495px 0px;\
				border-radius: 9px;\
				border: 2px solid white;\
			}\
			QSlider::sub-page:horizontal{\
				border-radius: 4.495px;\
				margin: 0px -4.5px 0px 0px;\
				background: rgb(90, 80, 160);\
			}\
			QSlider::groove::hover{\
				background: rgba(190, 190, 190, 255);\
		}";
}