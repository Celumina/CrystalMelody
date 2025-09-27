#pragma once

#include <QtWidgets/QMainWindow>
#include <QFileDialog>
#include <QTimer>
#include <QBuffer>

#include "CrystalMelodyTheme.h"

#include "CrystalPlayer.h"
#include "CrystalImageViewer.h"
#include "CrystalIndexInfos.h"
#include "CrystalProfile.h"
#include "CrystalPackage.h"
#include "CrystalFramelessWidget.h"


class CrystalMelody : public CrystalFramelessWidget{
	Q_OBJECT

public:
	CrystalMelody(int argc, char* argv[], QWidget *parent = nullptr);

	enum class FileMode : uint16_t {
		file, 
		folder, 
		package
	};

protected:
	void enterEvent(QEnterEvent* enterEvent) override;
	void leaveEvent(QEvent* leaveEvent) override;
	void closeEvent(QCloseEvent* closeEvent) override;
	void keyPressEvent(QKeyEvent* keyEvent) override;

private slots:
	// Player Events
	void onMetaDataChanged();
	void onPositionChanged(qint64 position);
	void onMediaStatusChanged(QMediaPlayer::MediaStatus mediaStatus);

	void clickedPushButtonClose();
	void clickedPushButtonHide();

	void clickedPushButtonFileOpen();
	void clickedPushButtonFileOpenFolder();
	void clickedPushButtonFilePack();
	void clickedPushButtonFileUnpack();
	void clickedPushButtonFileSaveThumbnailAs();

	void toggledPushButtonSettingsDynamic(bool isDynamic);
	void toggledPushButtonStayOnTop(bool isStay);

	void clickedPushButtonPause(bool isPause);
	void clickedPushButtonPrev();
	void clickedPushButtonNext();
	void clickedPushButtonMode();

	void releasedSliderMain();
	void changedSliderVolume(int value);

	void changedComboBoxList(int index);

	void changedLineEditerFilter(QString text);

private:
	void initProfile();
	void saveProfile();
	void loadParams(int argc, char* argv[]);

	void updateIndexInfos();

	void urlSwitch(const QUrl& url);
	void packageSwitch();

	void openFile(const QUrl& url);
	void openFolder(const QDir& dir);

	void openFileMedia(const QUrl& url);
	void openFilePackage(const QString& path);
	void openFileImage(const QString& path);

	void openPackageMedia();
	void openPackageImage();

	Ui::CrystalMelodyClass ui;
	CrystalMelodyTheme theme;

	FileMode fileMode = FileMode::file;

	// dlog: �����Զ�����Դ������
	CrystalPlayer player;
	CrystalImageViewer imageViewer;

	std::unique_ptr <QFile> packageSource = nullptr;
	std::unique_ptr <CrystalPackage> package = nullptr;
	CrystalPackage::Index currentIndex = CrystalPackage::Index(nullptr, 0);

	QFileInfoList fileInfos;
	CrystalIndexInfos indexInfos;
	CrystalProfile profile;

};
