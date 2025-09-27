#pragma once
#include <QMap>
#include <type_traits>

// DEV: parse Json, don't custom

class CrystalProfile {
public:
	using Iterator = QMap<QString, QString>::iterator;
	enum : char {
		spliter = '='
	};

	enum class expection {
		fileOpenFailed,
		illegalChar
	};

	CrystalProfile();
	~CrystalProfile();
	QString& operator[] (const QString& key);
	QString value(const QString& key) const;
	bool exists(const QString& key) const;
	Iterator begin();
	Iterator end();

	template<typename ValueType>
	void set(const QString& key, ValueType value);

	void set(const QString& key, const QString& value);

	template<typename FuncType, typename ObjectType = int>
	bool asInt(const QString& key, FuncType func, ObjectType obj = 0) const;

	template<typename FuncType, typename ObjectType = int>
	bool asString(const QString& key, FuncType func, ObjectType obj = 0) const;

private:
	template<typename ParamType, typename FuncType, typename ObjectType = int>
	void asImpl(ParamType param, FuncType func, ObjectType obj = 0) const;


	QMap<QString, QString> data;
};


// Default Set Value Type treat as int
template<typename ValueType>
inline void CrystalProfile::set(const QString& key, ValueType value) {
	data[key] = QString::number(static_cast<int>(value));
}

// this value set a key as int to call the func, if key exists, set and return true.
template<typename FuncType, typename ObjectType>
inline bool CrystalProfile::asInt(const QString& key, FuncType func, ObjectType obj)  const  {
	if (exists(key)) {
		asImpl(data[key].toInt(), func, obj);
		return true;
	}
	return false;
}

template<typename FuncType, typename ObjectType>
inline bool CrystalProfile::asString(const QString& key, FuncType func, ObjectType obj)  const {
	if (exists(key)) {
		asImpl(data[key], func, obj);
		return true;
	}
	return false;
}

template<typename ParamType, typename FuncType, typename ObjectType>
inline void CrystalProfile::asImpl(ParamType param, FuncType func, ObjectType obj)  const {
	// for member function
	if constexpr (!std::is_same<int, ObjectType>::value) {
		(obj->*func)(param);
	}
	// for global function
	else {
		func(param);
	}
}
