#ifndef _ICONMACRO_H_
#define _ICONMACRO_H_

#ifdef __WXMSW__
    #define ACTI_ICON_SIZED(icon, size) (wxIcon(icon, wxICON_DEFAULT_TYPE, size, size))
    #define ACTI_ICON(icon) (wxIcon(icon))
#else
    #define ACTI_ICON_SIZED(icon, size) (ACTI_ICON(icon))
    #define ACTI_ICON(icon) (wxIcon(wxStandardPaths::Get().GetDataDir() + wxFileName::GetPathSeparator() + icon ".png", wxBITMAP_TYPE_PNG))
#endif


#endif
