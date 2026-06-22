#ifndef IMGUI_PAINGINE_EXPORT_H
#define IMGUI_PAINGINE_EXPORT_H

#if IMGUI_PAINGINE_SHARED_LIB
#if _WIN32
#ifdef IMGUI_PAINGINE_EXPORTS
#define IMGUI_PAINGINE_API __declspec(dllexport)
#define IMGUI_API __declspec(dllexport)
#else
#define IMGUI_PAINGINE_API __declspec(dllimport)
#define IMGUI_API __declspec(dllexport)
#endif
#elif __GNUC__ >= 4
#define IMGUI_PAINGINE_API __attribute__((visibility("default")))
#define IMGUI_API __attribute__((visibility("default")))
#else
#define IMGUI_PAINGINE_API
#define IMGUI_API
#endif
#else
#define IMGUI_PAINGINE_API
#define IMGUI_API
#endif

#endif