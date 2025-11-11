#pragma once
#ifdef BUILDING_DLL
#define VDS_API __declspec(dllexport)
#else
#define VDS_API __declspec(dllimport)
#endif
