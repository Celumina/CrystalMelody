#include "FunctionalLibrary.h"
#include <QMimeDatabase>
#include <QPainter>
#include <QPainterPath>
#include <QGuiApplication>
#include <QScreen>
#include <QImageReader>
#include <QFile>

#include "DefaultWidgetConfigurations.h"
#include <cmath>

CrystalFileType fnlib::FileType(const QString& src) {
	QMimeDatabase mimeDatabase;
	QMimeType mime(mimeDatabase.mimeTypeForFile(src));

	if (mime.name().startsWith("audio/")) {
		return CrystalFileType::audio;
	}
	else if (mime.name().startsWith("video/")) {
		return CrystalFileType::video;
	}
	else if (mime.name().startsWith("image/")) {
		return CrystalFileType::image;
	}
	else if (mime.name().startsWith("text/")) {
		return CrystalFileType::text;
	}
	else if (mime.name().startsWith(".crystal", Qt::CaseInsensitive)) {
		return CrystalFileType::package;
	}
	else {
		return CrystalFileType::unknown;
	}
}

void fnlib::SetPixmap(QLabel& label, const QPixmap& inPixmap, CrystalRadius radiusType) {
	// fix the picture in high dpi device
	qreal pixelRadio = QGuiApplication::primaryScreen()->devicePixelRatio();
	QPixmap pixmap = inPixmap.scaled(label.size() * pixelRadio, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	// Notice: QBitmap mask can't be half transparent, so use QImage
	QImage maskImage(pixmap.size(), QImage::Format_ARGB32);
	QPainter painter(&maskImage);
	QBrush brush(pixmap);

	maskImage.setDevicePixelRatio(pixelRadio);
	maskImage.fill(Qt::transparent);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setBrush(brush);
	QRect imageRect = pixmap.rect();
	QPainterPath maskPath;
	maskPath.setFillRule(Qt::WindingFill); 
	int currentBorderRadius = melodycfg::windowBorderRadius * pixelRadio;
	maskPath.addRoundedRect(imageRect, currentBorderRadius, currentBorderRadius);
	
	auto sharpenLeft = [](QPainterPath& painterPath, const QRect& rect) {
		painterPath.addRect(QRect(rect.left() - rect.height() / 2, rect.top(), rect.width(), rect.height()));
		};

	auto sharpenRight = [](QPainterPath& painterPath, const QRect& rect) {
		painterPath.addRect(QRect(rect.left() + rect.height() / 2, rect.top(), rect.width(), rect.height()));
		};

	auto sharpenTop = [](QPainterPath& painterPath, const QRect& rect) {
		painterPath.addRect(QRect(rect.left(), rect.top() - rect.height() / 2, rect.width(), rect.height()));
		};

	auto sharpenBottom = [](QPainterPath& painterPath, const QRect& rect) {
		painterPath.addRect(QRect(rect.left(), rect.top() + rect.height() / 2, rect.width(), rect.height()));
		};


	switch (radiusType) {
	case CrystalRadius::right:
		sharpenLeft(maskPath, imageRect);
		break;
	case CrystalRadius::left:
		sharpenRight(maskPath, imageRect);
		break;
	case CrystalRadius::topRight:
		sharpenBottom(maskPath, imageRect);
		sharpenLeft(maskPath, imageRect);
		break;
	case CrystalRadius::topLeft:
		sharpenBottom(maskPath, imageRect);
		sharpenRight(maskPath, imageRect);
		break;
	case CrystalRadius::top:
		sharpenBottom(maskPath, imageRect);
		break;
	case CrystalRadius::bottom:
		sharpenTop(maskPath, imageRect);
		break;
	case CrystalRadius::bottomRight:
		sharpenTop(maskPath, imageRect);
		sharpenLeft(maskPath, imageRect);
		break;
	case CrystalRadius::bottomLeft:
		sharpenTop(maskPath, imageRect);
		sharpenRight(maskPath, imageRect);
		break;
	default:
		break;
	}
	
	painter.fillPath(maskPath, brush);
	label.setPixmap(QPixmap::fromImage(maskImage));
}

QColor fnlib::AverageColor(const QImage& image, int sampleSize) {
	size_t sum[3] = { 0 };
	for (qsizetype imageY = 0; imageY < image.height(); imageY += image.height() / sampleSize) {
		for (qsizetype imageX = 1; imageX < image.width(); imageX += image.width() / sampleSize) {
			QColor currentColor = image.pixelColor(imageX, imageY);
			sum[0] += currentColor.red();
			sum[1] += currentColor.green();
			sum[2] += currentColor.blue();
		}
	}
	size_t denominator = sampleSize * sampleSize;
	return QColor(sum[0] / denominator, sum[1] / denominator, sum[2] / denominator);
}

QImage fnlib::FastGaussianBlur(const QImage& image, int precision) {
	QImage blurImage(precision * 2, precision * 2, QImage::Format_ARGB32);
	QPainter painter(&blurImage);
	painter.drawImage(blurImage.rect(), image, image.rect());
	__private::GaussianBlur(blurImage, precision);
	return blurImage;
}

QString fnlib::LoadStyleSheet(const QString& filePath) {
	QFile file(filePath);

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return QString();
	}

	QTextStream in(&file);
	QString content = in.readAll();
	file.close(); 

	return content;
}


// DELOG: WhiteEdge GaussianBlur:
// if out_of_edge: getPxiel = Qt::White
// DELOG: parallel do....
void __private::GaussianBlur(QImage& image, int radius) {
	// GaussianKernel
	double* kernel = new double[2 * radius + 1];
	double sigma = static_cast<double>(radius) / 3;
	double sum = 0;
	for (int i = 0; i < 2 * radius + 1; ++i) {
		kernel[i] = 1 / (sigma * sqrt(2 * 3.141592653)) * exp(-1.0 * ((i - radius) * (i - radius)) / (2 * sigma * sigma));
		sum += kernel[i];
	}
	for (int i = 0; i < 2 * radius + 1; ++i) {
		kernel[i] /= sum;
	}

	QImage tempImage(image.width(), image.height(), QImage::Format_RGB32);
	for (int y = 0; y < image.height(); y++) {
		for (int x = 0; x < image.width(); x++){
			double red = 0, green = 0, blue = 0;
			for (int i = -radius; i <= radius; ++i) {
				int inx = x + i;
				// QColor rgb;
				if (inx  < 0 || inx >= image.width()) {
					inx = x - i;
				// 	rgb = Qt::white;
				}
				// else {
				// 	rgb = QColor(image.pixel(inx, y));
				// }
				QColor rgb = QColor(image.pixel(inx, y));
				red += rgb.red() * kernel[radius + i];
				green += rgb.green() * kernel[radius + i];
				blue += rgb.blue() * kernel[radius + i];
			}
			tempImage.setPixel(x, y, qRgb(red, green, blue));
		}
	}

	for (int y = 0; y < tempImage.height(); y++) {
		for (int x = 0; x < tempImage.width(); x++) {
			double red = 0, green = 0, blue = 0;
			for (int i = -radius; i <= radius; i++) {
				int iny = y + i;
				// QColor rgb;
				if (iny  < 0 || iny >= image.height()) {
					iny = y - i;
				// 	rgb = Qt::white;
				}
				// else {
				// 	rgb = QColor(tempImage.pixel(x, iny));
				// }
				QColor rgb = QColor(tempImage.pixel(x, iny));
				red += rgb.red() * kernel[radius + i];
				green += rgb.green() * kernel[radius + i];
				blue += rgb.blue() * kernel[radius + i];
			}
			image.setPixel(x, y, qRgb(red, green, blue));
		}
	}
	delete[] kernel;
}
