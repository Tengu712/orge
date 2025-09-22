#pragma once

enum OrgeDialogType {
	ORGE_DIALOG_TYPE_ERROR = 0,
	ORGE_DIALOG_TYPE_WARNING,
	ORGE_DIALOG_TYPE_INFORMATION,
};

struct OrgeShowDialogParam {
	OrgeDialogType dtype;
	const char *title;
	const char *message;
};
