#pragma once
#include "ui_CrystalMelody.h"
#include <QPixmap>

class CrystalMelodyTheme {
public:
	CrystalMelodyTheme();
	void setup(Ui::CrystalMelodyClass& ref);
	void setThumbnail(QPixmap& pixmap);

private:
	Ui::CrystalMelodyClass* handleUi = nullptr;
};

