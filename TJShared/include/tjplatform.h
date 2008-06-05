#ifndef _TJPLATFORM_H
#define _TJPLATFORM_H

#ifdef _WIN32
	/** Playform specific headers not needed by every source unit **/
	#define TJ_DEFAULT_CLASS_NAME (L"TjWndClass")
	#define TJ_DEFAULT_NDBL_CLASS_NAME (L"TjWndClassNdbl")
	#define TJ_PROPERTY_EDIT_CLASS_NAME (L"TjPropertyEditWndClass")
	#define TJ_PROPERTY_EDIT_NUMERIC_CLASS_NAME (L"TjPropertyEditNumericWndClass")
	#define TJ_PROPERTY_EDIT_TIME_CLASS_NAME (L"TjPropertyEditTimeWndClass")
	#define TJ_PROPERTY_LABEL_CLASS_NAME (L"TjPropertyLabelWndClass")
	#define TJ_TAB_PANEL_CLASS_NAME (L"TjTabPanelWndClass")
	#define TJ_GL_CLASS_NAME (L"TjGLWndClass")
	#define TJ_DROPSHADOW_CLASS_NAME (L"TjDropWndClass")
#endif

#endif