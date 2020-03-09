#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 
#endif // !WIN32_LEAN_AND_MEAN

#ifndef PCH_H
#define PCH_H 

/*Windows API*/
#define NOMINMAX
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP
#define WIN32_LEAN_AND_MEAN
#include <Windows.h> 
#include <dxgi1_4.h>
#include <d3dcompiler.h> 
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <dxgidebug.h>
#include <wrl.h> 
#include <d3d12.h>
#include "d3dx12.h"  

// Common
#include <algorithm>
#include <memory>
#include <cmath>
#include <cstdlib>
#include <filesystem>

//Debug
#include <exception>
#include <stdexcept>
#include <cassert>

//Streams
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <fstream>

// Data structures
#include <vector>
#include <string>
#include <stdint.h>
#include <tuple>
#include <stdint.h> 

/*Exceptions*/
#include "DXException.h" 
using namespace DirectX;

#endif // !PCH_H


/*Inner Used*/
#ifndef FrameBufferCount
#define FrameBufferCount 3 
#endif // !FrameBufferCount
