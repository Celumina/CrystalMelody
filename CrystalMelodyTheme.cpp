
#include "CrystalMelodyTheme.h"

#include <QListView>
#include <QMediaMetaData>

#include "FunctionalLibrary.h"
#include "CrystalStyleSheetEditor.h"

#include "DefaultStyleSheets.h"
#include "DefaultWidgetConfigurations.h"

CrystalMelodyTheme::CrystalMelodyTheme() {

}

void CrystalMelodyTheme::setup(Ui::CrystalMelodyClass& ref) {
	handleUi = &ref;
	// comboBoxList style, no shadow and translucent background
	handleUi->comboBoxList->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);
	handleUi->comboBoxList->view()->parentWidget()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
	handleUi->comboBoxList->setMaxVisibleItems(melodycfg::maxVisibleItems);
	// ui.comboBoxList->lineEdit()->setAlignment(Qt::AlignCenter);
}

void CrystalMelodyTheme::setThumbnail(QPixmap& pixmap) {
if (pixmap.isNull()) {
	pixmap = QPixmap(":/CrystalMelody/Resource/Graphics/defaultThumbnailImage.png");
}

auto blurImage = fnlib::FastGaussianBlur(pixmap.toImage(), melodycfg::gaussianBlurPrecision);
CrystalStyleSheetEditor qssEditSlider(qss::crystalSlider);
CrystalStyleSheetEditor qssEditFont;

QColor avgColor = fnlib::AverageColor(blurImage, melodycfg::gaussianBlurPrecision).rgb();

//  dynamic font color
if (avgColor.toHsv().value() < 100) {
	qssEditFont.setColor(QColor(180, 180, 180));
}
QString fontString = qssEditFont.toString();
handleUi->labelTitle->setStyleSheet(fontString);
handleUi->labelAuthor->setStyleSheet(fontString);
handleUi->labelLyrics->setStyleSheet(fontString);

// TEMP:: ADD A COLOR EDITOR
avgColor.setRed(avgColor.red() + 32);
avgColor.setGreen(avgColor.green() + 32);
avgColor.setBlue(avgColor.blue() + 32);

qssEditSlider.setBackground(avgColor);

handleUi->sliderMain->setStyleSheet(qssEditSlider.toString());
fnlib::SetPixmap(*handleUi->labelFrame, QPixmap::fromImage(blurImage), CrystalRadius::right);
fnlib::SetPixmap(*handleUi->labelPicture, pixmap, CrystalRadius::left);
}
