#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <ddraw.h>

namespace ViewZoom
{

// The renderer uses an inverse scale: 4.0 displays the map at 25%.
inline constexpr float MinimumScale = 0.1f;
inline constexpr float MaximumScale = 4.0f;
inline constexpr std::uint64_t SceneBufferBudget = 256ULL * 1024 * 1024;
inline constexpr DWORD MaximumSurfaceDimension = 8192;

struct BufferSize
{
	DWORD width;
	DWORD height;
};

inline BufferSize GetBufferSize(const DDSURFACEDESC2& primary, const RECT& display, float scale)
{
	const auto width = std::max<LONG>(1, display.right - display.left);
	const auto height = std::max<LONG>(1, display.bottom - display.top);
	return {
		std::max(primary.dwWidth, static_cast<DWORD>(std::ceil(std::max<LONG>(0, display.left) + width * scale))),
		std::max(primary.dwHeight, static_cast<DWORD>(std::ceil(std::max<LONG>(0, display.top) + height * scale)))
	};
}

inline bool FitsBudget(const DDSURFACEDESC2& primary, BufferSize size)
{
	// Reserve row alignment in addition to the pixels; verify the real pitch after allocation.
	const auto bytesPerPixel = std::max<DWORD>(1, (primary.ddpfPixelFormat.dwRGBBitCount + 7) / 8);
	const std::uint64_t pitch = (static_cast<std::uint64_t>(size.width) * bytesPerPixel + 63) & ~63ULL;
	return size.width <= MaximumSurfaceDimension && size.height <= MaximumSurfaceDimension
		&& pitch * size.height * 2 <= SceneBufferBudget;
}

inline float GetMaximumScale(IDirectDrawSurface4* primarySurface, const RECT& display)
{
	DDSURFACEDESC2 primary = {};
	primary.dwSize = sizeof(primary);
	if (!primarySurface || primarySurface->GetSurfaceDesc(&primary) != DD_OK)
		return 1.0f;

	float low = 1.0f;
	float high = MaximumScale;
	if (FitsBudget(primary, GetBufferSize(primary, display, high)))
		return high;
	for (int i = 0; i < 24; ++i)
	{
		const float middle = (low + high) * 0.5f;
		if (FitsBudget(primary, GetBufferSize(primary, display, middle)))
			low = middle;
		else
			high = middle;
	}
	return low;
}

inline bool EnsureSceneBuffers(IDirectDraw4* directDraw, IDirectDrawSurface4* primarySurface,
	IDirectDrawSurface4*& scene, IDirectDrawSurface4*& temporary, const RECT& display, float scale)
{
	if (!directDraw || !primarySurface || !scene || !temporary || !std::isfinite(scale)
		|| scale < MinimumScale || scale > MaximumScale)
		return false;

	DDSURFACEDESC2 primary = {};
	DDSURFACEDESC2 current = {};
	DDSURFACEDESC2 saved = {};
	primary.dwSize = sizeof(primary);
	current.dwSize = sizeof(current);
	saved.dwSize = sizeof(saved);
	if (primarySurface->GetSurfaceDesc(&primary) != DD_OK || scene->GetSurfaceDesc(&current) != DD_OK
		|| temporary->GetSurfaceDesc(&saved) != DD_OK)
		return false;

	const auto required = GetBufferSize(primary, display, scale);
	if (current.dwWidth >= required.width && current.dwHeight >= required.height
		&& saved.dwWidth >= required.width && saved.dwHeight >= required.height)
		return true;
	if (!FitsBudget(primary, required))
		return false;

	// Grow in quarter-scale increments so smooth zoom does not allocate on every wheel event.
	const float allocationScale = std::min(GetMaximumScale(primarySurface, display), std::ceil(scale * 4.0f) / 4.0f);
	auto size = GetBufferSize(primary, display, allocationScale);
	if (!FitsBudget(primary, size))
		size = required;

	DDSURFACEDESC2 description = {};
	description.dwSize = sizeof(description);
	description.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
	description.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	description.dwWidth = size.width;
	description.dwHeight = size.height;
	description.ddpfPixelFormat = primary.ddpfPixelFormat;

	IDirectDrawSurface4* newScene = nullptr;
	IDirectDrawSurface4* newTemporary = nullptr;
	if (directDraw->CreateSurface(&description, &newScene, nullptr) != DD_OK)
		return false;
	if (directDraw->CreateSurface(&description, &newTemporary, nullptr) != DD_OK)
	{
		newScene->Release();
		return false;
	}

	DDSURFACEDESC2 sceneDescription = {};
	DDSURFACEDESC2 temporaryDescription = {};
	sceneDescription.dwSize = sizeof(sceneDescription);
	temporaryDescription.dwSize = sizeof(temporaryDescription);
	const bool validDescriptions = newScene->GetSurfaceDesc(&sceneDescription) == DD_OK
		&& newTemporary->GetSurfaceDesc(&temporaryDescription) == DD_OK;
	const auto surfaceBytes = [](const DDSURFACEDESC2& value) {
		return static_cast<std::uint64_t>(std::abs(static_cast<std::int64_t>(value.lPitch))) * value.dwHeight;
	};
	if (!validDescriptions || surfaceBytes(sceneDescription) + surfaceBytes(temporaryDescription) > SceneBufferBudget)
	{
		newTemporary->Release();
		newScene->Release();
		return false;
	}

	// Keep the old pair intact until both allocations succeed. The caller redraws the scene.
	scene->Release();
	temporary->Release();
	scene = newScene;
	temporary = newTemporary;
	return true;
}

}
