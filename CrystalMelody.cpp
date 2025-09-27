#include "CrystalMelody.h"

#include <QMediaMetaData>
#include <QMimeDatabase>
#include <QListView>

#include <QMovie>

#include "DefaultStyleSheets.h"
#include "DefaultMessages.h"
#include "DefaultWidgetConfigurations.h"


CrystalMelody::CrystalMelody(int argc, char* argv[], QWidget* parent)
	: CrystalFramelessWidget(parent), player(this) {
	ui.setupUi(this);
	theme.setup(ui);

	setFlexible(false);

	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_Hover);

	// Player Events
	connect(&player, SIGNAL(metaDataChanged()), this, SLOT(onMetaDataChanged()));
	connect(&player, SIGNAL(positionChanged(qint64)), this, SLOT(onPositionChanged(qint64)));
	connect(&player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
		this, SLOT(onMediaStatusChanged(QMediaPlayer::MediaStatus)));

	initProfile();
	loadParams(argc, argv);

	ui.labelLyrics->setText("Lyrics Label, Work in Progress.");
}

void CrystalMelody::enterEvent(QEnterEvent* enterEvent) {
	ui.tabWidgetMenu->show();
	if (ui.pushButtonSettingsLowFocus->isChecked()) {
		setWindowOpacity(1);
	}
	CrystalFramelessWidget::enterEvent(enterEvent);
}

void CrystalMelody::leaveEvent(QEvent* leaveEvent) {
	ui.tabWidgetMenu->hide();
	if (ui.pushButtonSettingsLowFocus->isChecked()) {
		setWindowOpacity(melodycfg::lowFocusOpacity);
	}
	CrystalFramelessWidget::leaveEvent(leaveEvent);
}

void CrystalMelody::closeEvent(QCloseEvent* closeEvent) {
	player.close();
	imageViewer.close();
	saveProfile();
	CrystalFramelessWidget::closeEvent(closeEvent);
}

void CrystalMelody::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key_Space) {
		ui.pushButtonPause->click();
	}
}

void CrystalMelody::onMetaDataChanged() {
	player.updateVideo();

	if (!player.metaData().stringValue(QMediaMetaData::Title).isEmpty()) {
		ui.labelTitle->setText(player.metaData().stringValue(QMediaMetaData::Title));
	}

	ui.labelAuthor->setText(player.metaData().stringValue(QMediaMetaData::Author));

	auto pixmap = player.metaData().value(QMediaMetaData::ThumbnailImage).value<QPixmap>();
	// this functions could changes input pixmap contain
	theme.setThumbnail(pixmap);
}

void CrystalMelody::onPositionChanged(qint64 position) {
	if (ui.sliderMain->isWheelChanged()) {
		releasedSliderMain();
	}
	if (!ui.sliderMain->isTriggered() && player.duration()) { 
		ui.sliderMain->setValue(position * ui.sliderMain->maximum() / player.duration());
	}
	ui.sliderMain->updateStatus();
}

void CrystalMelody::onMediaStatusChanged(QMediaPlayer::MediaStatus mediaStatus) {
	if (mediaStatus == QMediaPlayer::MediaStatus::EndOfMedia) {
		// int mediaIndex = -1;
		CrystalFileType type = CrystalFileType::unknown;

		// if wanna declare object on switch, using {} after case
		switch (player.playMode()) {
		case CrystalPlayer::PlayMode::loops:
			// <- no break, do shuffle things
		case CrystalPlayer::PlayMode::shuffle:
			indexInfos.nextMedia();
			if (indexInfos.currentIndex() == ui.comboBoxList->currentIndex()) {
				player.play();
			}
			else {
				ui.comboBoxList->setCurrentIndex(indexInfos.currentIndex());
			}
			break;
		}
	}
}

void CrystalMelody::clickedPushButtonClose() {
	close();
}

void CrystalMelody::clickedPushButtonHide() {
	showMinimized();
}

void CrystalMelody::clickedPushButtonFileOpen() {
	QFileDialog fileDialog(this);
	fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	QUrl url = fileDialog.getOpenFileUrl();
	if (url.isEmpty()) {
		return;
	}
	fileMode = FileMode::file;
	openFile(url);
}

void CrystalMelody::clickedPushButtonFileOpenFolder() {
	QDir dir = QFileDialog::getExistingDirectory();
	if (dir.dirName() == ".") {
		return;
	}
	openFolder(dir);
}

void CrystalMelody::clickedPushButtonFilePack() {
	if (fileMode != FileMode::folder) {
		ui.labelTitle->setText(msg::noFolderOpened);
		return;
	}

	QString dirString  = QFileDialog::getExistingDirectory(this, msg::outputDirectory);
	if (dirString.isEmpty()) {
		return;
	}
	QDir dir(dirString);
	CrystalPackage::pack(fileInfos[0].dir().absolutePath(), dir.absolutePath());
	ui.labelTitle->setText(msg::packagedSuccessfully);
}

void CrystalMelody::clickedPushButtonFileUnpack() {
	if (fileMode != FileMode::package) {
		ui.labelTitle->setText(msg::noPackageOpened);
		return;
	}
	QString dirString = QFileDialog::getExistingDirectory(this, msg::outputDirectory);
	if (dirString.isEmpty()) {
		return;
	}
	QDir dir(dirString);
	package->unpack(dir.absolutePath());
	ui.labelTitle->setText(msg::unpackagedSuccessfully);
}

void CrystalMelody::clickedPushButtonFileSaveThumbnailAs() {
	QDir dir = QFileDialog::getExistingDirectory(nullptr, msg::saveThumbnailAs);
	CrystalPlayer::IOExpection expection = player.saveThumbnailAs(dir.absolutePath());
	if (expection == CrystalPlayer::IOExpection::thumbnailEmpty) {
		ui.labelTitle->setText(msg::thumbnailEmpty);
	}
	else if (expection == CrystalPlayer::IOExpection::accessError) {
		ui.labelTitle->setText(msg::accessError);
	}
	else {
		ui.labelTitle->setText(msg::saveSuccessfully);
	}
}


void CrystalMelody::toggledPushButtonSettingsDynamic(bool isDynamic) {
#ifdef Q_OS_WIN
	player.setDynamicDesktop(isDynamic);
#endif
}

void CrystalMelody::toggledPushButtonStayOnTop(bool isStay) {
	if (isStay) {
		setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
		show();
	}
	else {
		setWindowFlags(Qt::FramelessWindowHint);
		show();
	}
}

void CrystalMelody::clickedPushButtonPause(bool isPause) {
	if (isPause) {
		player.pause();
	}
	else {
		player.play();
	}

}

void CrystalMelody::clickedPushButtonPrev() {
	if (player.playMode() == CrystalPlayer::PlayMode::shuffle) {
		ui.comboBoxList->setCurrentIndex(indexInfos.currentIndex());
	}
	else if (ui.comboBoxList->currentIndex() > 0) {
		ui.comboBoxList->setCurrentIndex(ui.comboBoxList->currentIndex() - 1);
	}
	
}

void CrystalMelody::clickedPushButtonNext() {
	if (player.playMode() == CrystalPlayer::PlayMode::shuffle) {
		ui.comboBoxList->setCurrentIndex(indexInfos.currentIndex());
	}
	else if (ui.comboBoxList->currentIndex() < ui.comboBoxList->count() - 1) {
		ui.comboBoxList->setCurrentIndex(ui.comboBoxList->currentIndex() + 1);
	}
}

void CrystalMelody::clickedPushButtonMode() {
	player.switchPlayMode();

	// DLOG: Constructor a new ProfileClass for save these Text, doesn't use StaticMessageText
	const char* playModeText[] = { "Single" , "Cycle" , "Loops" , "Shuffle" };

	ui.pushButtonMode->setText(playModeText[static_cast<uint16_t>(player.playMode())]);
	if (player.playMode() == CrystalPlayer::PlayMode::singleCycle) {
		player.setLoops(QMediaPlayer::Infinite); // param: loop times
	}
	else {
		player.setLoops(1);
	}
	updateIndexInfos();
}

void CrystalMelody::releasedSliderMain() {
	player.setPosition(ui.sliderMain->value() * player.duration() / ui.sliderMain->maximum());
}

void CrystalMelody::changedSliderVolume(int value) {
	float volume = static_cast<float>(value) / ui.sliderVolume->maximum();
	player.setVolume(volume);
}

void CrystalMelody::changedComboBoxList(int index) {
	switch (fileMode) {
	case FileMode::file: // <- no break
	case FileMode::folder:
		if (index >= 0) {
			urlSwitch(QUrl::fromLocalFile(fileInfos[index].absoluteFilePath()));
		}
		break;
	case FileMode::package:
		currentIndex = CrystalPackage::Index(package.get(), index);
		if (!currentIndex.empty()) {
			packageSwitch();
		}
		break;
	}
	indexInfos.setDataIndex(index);
}

void CrystalMelody::changedLineEditerFilter(QString text) {
	QListView* view = qobject_cast<QListView*>(ui.comboBoxList->view());
	int hiddenCount = 0;
	for (int index = 0; index < ui.comboBoxList->count() + 1; ++index) {
		if (ui.comboBoxList->itemText(index).contains(text, Qt::CaseInsensitive)) {
			view->setRowHidden(index, false);
		}
		else {
			view->setRowHidden(index, true);
			++hiddenCount;
		}
	}

	ui.comboBoxList->setMaxVisibleItems(melodycfg::maxVisibleItems + hiddenCount); // Fix this, error item number visible

	if (!text.isEmpty()) {
		// ui.lineEditFilter->style()->unpolish(ui.lineEditFilter);
		ui.lineEditFilter->setStyleSheet(qss::lineEditFilterOn);
	}
	else {
		// ui.lineEditFilter->style()->unpolish(ui.lineEditFilter);
		ui.lineEditFilter->setStyleSheet(qss::lineEditFilterOff);
	}

}


// : user config save and load edit entrance :
void CrystalMelody::initProfile() {
	profile.asInt("volume", &CrystalSlider::setValue, ui.sliderVolume);
	profile.asInt("dynamicDesktop", &QPushButton::setChecked, ui.pushButtonSettingsDynamic);
	profile.asInt("stayOnTop", &QPushButton::setChecked, ui.pushButtonSettingsStayOnTop);
	profile.asInt("lowFocus", &QPushButton::setChecked, ui.pushButtonSettingsLowFocus);

	profile.asString("folder", &CrystalMelody::openFolder, this);

	if (profile.exists("playMode")) {
		for (int i = 0; i < profile["playMode"].toInt(); ++i) {
			clickedPushButtonMode();
		}
	}
}

void CrystalMelody::saveProfile() {
	profile.set("volume", ui.sliderVolume->value());
	profile.set("dynamicDesktop", ui.pushButtonSettingsDynamic->isChecked());
	profile.set("stayOnTop", ui.pushButtonSettingsStayOnTop->isChecked());
	profile.set("lowFocus", ui.pushButtonSettingsLowFocus->isChecked());
	profile.set("playMode", player.playMode());
}



void CrystalMelody::loadParams(int argc, char* argv[]) {
	// if param exist, argv[0] will be self. So argv[1] is the first file path;
	if (argc == 2) {
		QFileInfo file(argv[1]);
		if (file.isDir()) {
			openFolder(file.dir());
		}
		else {
			openFile(QUrl::fromLocalFile(QString::fromLocal8Bit(argv[1]))); // params could be difference coding...
		}
		
	}
	else if (argc > 2) {
		for (int count = 1; count < argc; ++count) {
			fileInfos.push_back(QFileInfo(argv[count]));
			ui.comboBoxList->addItem(fileInfos[count - 1].fileName());
		}
	}
}


void CrystalMelody::updateIndexInfos() {
	CrystalFileType type = CrystalFileType::unknown;
	if (player.playMode() == CrystalPlayer::PlayMode::loops ||
		player.playMode() == CrystalPlayer::PlayMode::shuffle) {
		indexInfos.clear();
		for (int index = 0; index < ui.comboBoxList->count(); ++index) {
			type = fnlib::FileType(ui.comboBoxList->itemText(index));
			indexInfos.append({ type, index });
			if (index == ui.comboBoxList->currentIndex()) {
				indexInfos.setCurrentAsEnd();
			}
		}
	}
	if (player.playMode() == CrystalPlayer::PlayMode::shuffle) {
		indexInfos.shuffle();
	}
}


void CrystalMelody::urlSwitch(const QUrl& url) {
	QMimeDatabase mimeDatabase;
	QMimeType mime = mimeDatabase.mimeTypeForUrl(url);
	QFileInfo fileInfo(url.toLocalFile());
	ui.labelTitle->setText(url.fileName());
	if (mime.name().startsWith("audio/") || mime.name().startsWith("video/")) { 
		openFileMedia(url);
	}
	else if (url.toString().endsWith(".crystal", Qt::CaseInsensitive)) {
		openFilePackage(fileInfo.absoluteFilePath());
	}
	else if (mime.name().startsWith("image/")) {
		openFileImage(fileInfo.absoluteFilePath());
	}
	else {
		ui.labelTitle->setText(msg::unknownFileType);
	}
}

void CrystalMelody::packageSwitch() {
	QMimeDatabase mimeDatabase;
	QMimeType mime = mimeDatabase.mimeTypeForFile(currentIndex.name());
	if (mime.name().startsWith("audio/") || mime.name().startsWith("video/")) {
		openPackageMedia();
	}
	else if (mime.name().startsWith("image/"))  {
		openPackageImage();
	}
	else {
		ui.labelTitle->setText(msg::unknownFileType);
	}
}

void CrystalMelody::openFile(const QUrl& url) {
	fileMode = FileMode::file;
	QFileInfo fileInfo(url.toLocalFile());
	fileInfos.clear();
	fileInfos.push_back(fileInfo);
	ui.comboBoxList->clear();
	ui.comboBoxList->addItem(fileInfo.fileName());
	updateIndexInfos();
}

void CrystalMelody::openFolder(const QDir& dir) {
	fileInfos = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDot);
	// Changing fileMode must be earlier than changing comboxBox, otherwise program will crash
	fileMode = FileMode::folder;
	ui.comboBoxList->clear();

	for (const auto& fileInfo : fileInfos) {
		if (fileInfo.isDir()) {
			ui.comboBoxList->addItem("[" + fileInfo.fileName() + "]");
		}
		else {
			ui.comboBoxList->addItem(fileInfo.fileName());
		}
	}

	// update playlist
	updateIndexInfos();
	
	// update profile for open folder when program initializing.
	profile.set("folder", dir.absolutePath());
}

void CrystalMelody::openFileMedia(const QUrl& url) {
	ui.pushButtonPause->setChecked(false);
	player.setMediaUrl(url);
	player.play();
}

void CrystalMelody::openFilePackage(const QString& path) {
	std::unique_ptr<QFile> tempFile(new QFile(path));
	tempFile->open(QIODevice::ReadOnly);
	if (!tempFile->isOpen()) {
		ui.labelLyrics->setText(msg::failedOpenPackage);
		return;
	}
	packageSource.reset(tempFile.release());

	ui.labelLyrics->setText(path);

	package.reset(new CrystalPackage(path));

	fileMode = FileMode::package;
	ui.comboBoxList->clear();
	for (auto fileIndex = package->firstIndex(); !fileIndex.empty(); ++fileIndex) {
		ui.comboBoxList->addItem(fileIndex.name());
	}
	
	updateIndexInfos();
}

void CrystalMelody::openFileImage(const QString& path) {
	imageViewer.setDisplaySource(path);
	imageViewer.display();
}

void CrystalMelody::openPackageMedia() {
	ui.labelTitle->setText(currentIndex.name());
	ui.pushButtonPause->setChecked(false);

	player.setMediaData(*packageSource, currentIndex);
	player.play();
}

void CrystalMelody::openPackageImage() {
	imageViewer.setDisplayData(currentIndex);
	imageViewer.display();
}
