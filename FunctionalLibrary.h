#pragma once
#include <QString>
#include <QLabel>
#include <QPixmap>

enum class CrystalFileType : uint16_t {
	unknown,
	audio,
	video,
	image,
	text,
	package
};

enum class CrystalRadius : uint16_t {
	none,
	all,
	right,
	left,
	top,
	bottom,
	topRight, 
	topLeft,
	bottomRight,
	bottomLeft
};

namespace __private {
	void GaussianBlur(QImage& image, int radius);
}


namespace fnlib {
	CrystalFileType FileType(const QString& src);

	void SetPixmap(QLabel& label, const QPixmap& pixmap, CrystalRadius radiusType = CrystalRadius::none);
	QColor AverageColor(const QImage& image, int sampleSize);
	QImage FastGaussianBlur(const QImage& image, int precision);

	QString LoadStyleSheet(const QString& filePath);
}


// master template, necessary.
template <typename FuncType>
struct FuncTraits;

template <typename RetType, typename ...ParamTypes>
struct FuncTraits<std::function<RetType(ParamTypes...)>> {
	static const size_t paramCount = sizeof...(ParamTypes);

	struct Return {
		using Type = RetType;
	};
	
	// param Index
	template <size_t index>
	struct Parameter {
		using Type = typename std::tuple_element<index, std::tuple<ParamTypes...>>::type;
	};

	struct FirstParameter {
		using Type = typename Parameter<0>::Type;
	};
};


template<typename Type>
struct TypeValue {
	static constexpr Type value;
};

template<typename Type, TypeValue<Type>  typeValue = 3>
struct Test {
	static constexpr TypeValue value = typeValue;
};

struct A {};

class B{
	friend class A;
};