#pragma once
# include <QObject>
#include <QAudioOutput>
#include <QMediaPlayer>
#include <QFile>
#include <QBuffer>

#include "CrystalVideoWidget.h"
#include "CrystalPackage.h"

#include "CrystalDynamicEngine.h" // windows only

class CrystalPlayer: public QMediaPlayer {
	Q_OBJECT
public:
	enum MagicNumber : int {
		timerInterval = 100
	};

	enum class PlayMode : uint16_t {
		singleTrack,
		singleCycle, 
		loops, 
		shuffle
	};

	enum class IOExpection : uint16_t {
		none,
		thumbnailEmpty,
		accessError
	};

	CrystalPlayer(QObject* parentWidget);
	void setVolume(float volume);
	void setMediaData(QFile& source, const CrystalPackage::Index& index);
	void setMediaUrl(const QUrl& url);
	void setDynamicDesktop(bool isDynamic);

	IOExpection saveThumbnailAs(const QString& dir);

	void switchPlayMode();
	PlayMode playMode();
	const QPixmap& thumbnailImage();

	qint64 mediaDuration();

	void updateVideo();
	void close();
	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
protected:
	void timerEvent(QTimerEvent* timerEvent);

private slots:
	void onDurationChanged(qint64  _duration);
	void onMediaStatusChanged(QMediaPlayer::MediaStatus _status);
	void onVideoClosed();

private:
	void mediaPreprocess();

	bool isVideo = false;
	int timerId = 0; // zero means not start a timer
	qint64 retainedDuration = 0;
	QString thumbnailFormat = ".png";
	QString resourcePath;

	QAudioOutput audio;
	CrystalVideoWidget video;
	QPixmap thumbnail;

	std::unique_ptr<QBuffer> videoBuffer = nullptr;
	
	PlayMode currentMode = PlayMode::singleTrack;

#ifdef Q_OS_WIN
	CrystalDynamicEngine dynamicEngine; // windows only
#endif
};

