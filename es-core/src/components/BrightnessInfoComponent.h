#include <string>
#pragma once

#include <mutex>
#include "GuiComponent.h"

class ComponentGrid;
class NinePatchComponent;
class TextComponent;
class ImageComponent;
class Window;

class BrightnessInfoComponent : public GuiComponent
{
public:
	BrightnessInfoComponent(Window* window, const std::string& iconPath = "", const std::string& fontPath = "");
	~BrightnessInfoComponent();

	void render(const Transform4x4f& parentTrans) override;
	void update(int deltaTime) override;

	void reset() { mBrightness = 1; }

private:
	NinePatchComponent* mFrame;
	TextComponent*		mLabel;
	ImageComponent*		mIcon;
	float				mBarTop;
	float				mBarBottom;

	int mBrightness;

	int mCheckTime;
	int mDisplayTime;
};
