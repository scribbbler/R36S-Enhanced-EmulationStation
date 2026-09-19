#include <string>
#include "BrightnessInfoComponent.h"
#include "ThemeData.h"
#include "PowerSaver.h"
#include "components/ComponentGrid.h"
#include "components/NinePatchComponent.h"
#include "components/TextComponent.h"
#include "components/ImageComponent.h"
#include "resources/Font.h"
#include "resources/ResourceManager.h"
#include "EsLocale.h"
#include "BrightnessControl.h"
#include "Window.h"
#include "DisplayPanelControl.h"

#define PADDING_PX			(Renderer::getScreenWidth()*0.006)
#define PADDING_BAR			(Renderer::isSmallScreen() ? Renderer::getScreenWidth()*0.02 : Renderer::getScreenWidth()*0.006)

#define VISIBLE_TIME		2650
#define FADE_TIME			350
#define BASEOPACITY			255
#define CHECKBRIGHTNESSDELAY	40

BrightnessInfoComponent::BrightnessInfoComponent(Window* window, const std::string& iconPath, const std::string& fontPath)
	: GuiComponent(window)
{
	mDisplayTime = -1;
	mBrightness = -1;
	mCheckTime = 0;

	// Figma "Toast / Vertical": 58 x 184 panel (black 64%), % on top,
	// a 24 x 100 bar (white 32% track / 80% fill, 4px radius), 24 x 24 icon.
	Vector2f fullSize(58.0f, 184.0f);
	setSize(fullSize);

	mFrame = nullptr; // background is drawn as a rounded rect in render()

	// label (%) at the top: BPreplay-Bold 17px, 24px band at y=10
	auto font = fontPath.empty() ? Font::get(17) : Font::get(17, fontPath);
	mLabel = new TextComponent(mWindow, "", font, 0xFFFFFFFF, ALIGN_CENTER);
	mLabel->setPosition(0, 10);
	mLabel->setSize(fullSize.x(), 24);
	addChild(mLabel);

	// bar area (24 x 100 at 17,42)
	mBarTop = 42.0f;
	mBarBottom = 142.0f;

	// icon (24 x 24 at 17,150)
	mIcon = nullptr;
	if (!iconPath.empty() && ResourceManager::getInstance()->fileExists(iconPath))
	{
		mIcon = new ImageComponent(mWindow);
		mIcon->setImage(iconPath);
		mIcon->setColorShift(0xFFFFFFFF);
		mIcon->setMaxSize(24.0f, 24.0f);
		mIcon->setPosition(17.0f + (24.0f - mIcon->getSize().x()) / 2.0f,
		                   150.0f + (24.0f - mIcon->getSize().y()) / 2.0f);
		addChild(mIcon);
	}

	// top-left corner, 12px from the left edge and 12px from the top
	setPosition(12.0f, 12.0f, 0);
	setOpacity(BASEOPACITY);
}

BrightnessInfoComponent::~BrightnessInfoComponent()
{
	delete mLabel;
	if (mFrame)
		delete mFrame;
	if (mIcon)
		delete mIcon;
}

void BrightnessInfoComponent::update(int deltaTime)
{
	GuiComponent::update(deltaTime);

	if (mDisplayTime >= 0)
	{
		mDisplayTime += deltaTime;
		if (mDisplayTime > VISIBLE_TIME + FADE_TIME)
		{
			mDisplayTime = -1;

			if (isVisible())
			{
				setVisible(false);
				PowerSaver::resume();
			}
		}
	}

	mCheckTime += deltaTime;
	if (mCheckTime < CHECKBRIGHTNESSDELAY)
		return;

	mCheckTime = 0;

	int brightness = DisplayPanelControl::getInstance()->getBrightnessLevel();
	if (brightness != mBrightness)
	{
		bool firstTime = (mBrightness < 0);

		mBrightness = brightness;

		mLabel->setText(std::to_string(mBrightness) + "%");

		if (!firstTime)
		{
			mDisplayTime = 0;

			if (!isVisible())
			{
				setVisible(true);
				PowerSaver::pause();
			}
		}
	}
}

void BrightnessInfoComponent::render(const Transform4x4f& parentTrans)
{
	if (!mVisible || mDisplayTime < 0)
		return;

	int opacity = BASEOPACITY - Math::max(0, (mDisplayTime - VISIBLE_TIME) * BASEOPACITY / FADE_TIME);
	setOpacity(opacity);

	Transform4x4f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);

	float f = opacity / 255.0f;
	// exact Figma colors, faded by the popup opacity
	auto C = [f](unsigned int rgb, float baseA) -> unsigned int {
		unsigned int al = (unsigned int)(baseA * 255.0f * f + 0.5f);
		if (al > 255) al = 255;
		return (rgb << 8) | (al & 0xFF);
	};
	// panel black @ 64%, radius 12; bar track white @ 32%, radius 4; fill white @ 80%, radius 4
	Renderer::drawRoundRect(0.0f, 0.0f, 58.0f, 184.0f, 12.0f, C(0x000000, 0.80f));
	Renderer::drawRoundRect(17.0f, 42.0f, 24.0f, 100.0f, 4.0f, C(0xFFFFFF, 0.32f));

	int lvl = Math::max(0, Math::min(100, mBrightness));
	float fillH = 100.0f * lvl / 100.0f;
	if (fillH > 1.0f)
	{
		float fr = Math::min(4.0f, fillH * 0.5f);
		Renderer::drawRoundRect(17.0f, 42.0f + (100.0f - fillH), 24.0f, fillH, fr, C(0xFFFFFF, 0.80f));
	}

	GuiComponent::render(parentTrans); // % label + icon
}
