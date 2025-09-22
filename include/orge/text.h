#pragma once

enum OrgeTextLocationHorizontal {
	ORGE_TEXT_LOCATION_HORIZONTAL_LEFT = 0,
	ORGE_TEXT_LOCATION_HORIZONTAL_CENTER,
	ORGE_TEXT_LOCATION_HORIZONTAL_RIGHT,
};

enum OrgeTextLocationVertical {
	ORGE_TEXT_LOCATION_VERTICAL_TOP = 0,
	ORGE_TEXT_LOCATION_VERTICAL_MIDDLE,
	ORGE_TEXT_LOCATION_VERTICAL_BOTTOM,
};

struct OrgeRasterizeCharactersParam {
	const char *id;
	const char *s;
};

struct OrgeLayoutTextParam {
	const char *renderPassId;
	const char *subpassId;
	const char *fontId;
	const char *text;
	float x;
	float y;
	float height;
	OrgeTextLocationHorizontal horizontal;
	OrgeTextLocationVertical vertical;
};
