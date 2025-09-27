#include "CrystalPlayer.h"
#include <QMediaMetaData>
#include <QMimeDatabase>
#include <QTimerEvent>

CrystalPlayer::CrystalPlayer(QObject* parentWidget) :
	QMediaPlayer(parentWidget) {
	setAudioOutput(&audio);
	setVideoOutput(&video);

	connect(this, SIGNAL(durationChanged(qint64)), this, SLOT(onDurationChanged(qint64)));
	connect(this, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, 
		SLOT(onMediaStatusChanged(QMediaPlayer::MediaStatus)));

	connect(&video, SIGNAL(closed()), this, SLOT(onVideoClosed()));
	
#ifdef Q_OS_WIN
	dynamicEngine.setWidget(reinterpret_cast<HWND>(video.winId()));
	timerId = startTimer(timerInterval);
#endif
}

void CrystalPlayer::setVolume(float volume) {
	audio.setVolume(volume);
}

void CrystalPlayer::setMediaData(QFile& source, const CrystalPackage::Index& index) {
	mediaPreprocess();
	resourcePath = index.name();
	source.seek(index.offset());
	QByteArray packageBytes = source.read(index.size());
	videoBuffer.reset(new QBuffer);
	videoBuffer->setData(packageBytes);
	videoBuffer->open(QIODevice::ReadOnly);
	setSourceDevice(videoBuffer.get());
}

void CrystalPlayer::setMediaUrl(const QUrl& url) {
	mediaPreprocess();
	resourcePath = url.toString();
	setSource(url);
}

void CrystalPlayer::setDynamicDesktop(bool isDynamic) {
#ifdef Q_OS_WIN
	video.adaptiveFullScreen(isDynamic);

	if (!isVideo) {
		video.hide();
	}

	if (isDynamic) {
		dynamicEngine.display();
	}
	else {
		dynamicEngine.release();
	}
#endif
}

CrystalPlayer::IOExpection CrystalPlayer::saveThumbnailAs(const QString& dir) {
	if (thumbnail.isNull()) {
		return IOExpection::thumbnailEmpty;
	}
	QString resourceName = resourcePath.mid(resourcePath.lastIndexOf('/'), resourcePath.length() - 1);
	if (!thumbnail.save(dir + resourceName + thumbnailFormat)) {
		return IOExpection::accessError;
	}
	return IOExpection::none;
}

void CrystalPlayer::switchPlayMode() {
	if (currentMode < PlayMode::shuffle) {
		currentMode = static_cast<PlayMode>(static_cast<uint16_t>(currentMode) + 1);
	}
	else {
		currentMode = PlayMode::singleTrack;
	}
}

CrystalPlayer::PlayMode CrystalPlayer::playMode() {
	return currentMode;
}

const QPixmap& CrystalPlayer::thumbnailImage() {
	return thumbnail;
}

qint64 CrystalPlayer::mediaDuration() {
	return retainedDuration;
}

void CrystalPlayer::updateVideo() {
	QSize resolution = metaData().value(QMediaMetaData::Resolution).value<QSize>();
	QMimeDatabase database;
	isVideo = database.mimeTypeForFile(resourcePath).name().startsWith("video/");

	if (resolution.isValid()) {
		video.display(resolution);
	}
	else if (video.isHidden() && isVideo) {
		video.display();
	}
	else if (video.isVisible() && !isVideo) {
		video.hide();
	}
}

void CrystalPlayer::close() {
	setMediaUrl(QUrl()); // <- if it not in front, video will not hide after videoEnd, because mfunc updateVideo
	video.hide();
}

void CrystalPlayer::timerEvent(QTimerEvent* timerEvent) {
#ifdef Q_OS_WIN
	if (timerEvent->timerId() == timerId && dynamicEngine.isActive()) {
		dynamicEngine.validationCheck();
	}
#endif

	// for music play without mistake while high-frequency file flip.
	if (!duration() && !resourcePath.isEmpty()) {
		setMediaUrl(resourcePath);
		play();
	}
}

void CrystalPlayer::onDurationChanged(qint64  _duration) {
	if (_duration) {
		retainedDuration = _duration;
	}
}

#include <QLabel>
void CrystalPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus _status) {
	thumbnail = metaData().value(QMediaMetaData::ThumbnailImage).value<QPixmap>();
}

void CrystalPlayer::onVideoClosed() {
	video.hide();
	setSource(QUrl());
	videoBuffer.reset();
}

void CrystalPlayer::mediaPreprocess() {
	if (isVideo) {
		setPosition(retainedDuration);// <- make sure special foramt meida complete before reset Source, 
		// otherwise mp4, webm etc. file may cause player heap corruption.
	}
	setSource(QUrl());	// player couldn't play, until videoBuffer reset.
}

