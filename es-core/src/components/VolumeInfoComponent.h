#include <string>
#pragma once

#include <mutex>
#include "GuiComponent.h"

class ComponentGrid;
class NinePatchComponent;
class TextComponent;
class ImageComponent;
class Window;

class VolumeInfoComponent : public GuiComponent
{
public:
	VolumeInfoComponent(Window* window, const std::string& iconPath = "", const std::string& fontPath = "");
	~VolumeInfoComponent();

	void render(const Transform4x4f& parentTrans) override;
	void update(int deltaTime) override;

	void reset() { mVolume = -1; }

private:
	NinePatchComponent* mFrame;
	TextComponent*		mLabel;
	ImageComponent*		mIcon;
	float				mBarTop;
	float				mBarBottom;

	int mVolume;

	int mCheckTime;
	int mDisplayTime;
};