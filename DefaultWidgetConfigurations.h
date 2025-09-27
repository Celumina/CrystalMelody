#pragma once


namespace melodycfg {
	enum : uint16_t {
		maxVisibleItems = 10,	// custom
		windowBorderRadius = 20,
		gaussianBlurPrecision = 8
	};

	static float lowFocusOpacity = 0.5;
};


namespace explorercfg {
	enum : uint16_t {
		addTabButtonPadding = 8, 
		maximumTabWidth = 200
	};
}