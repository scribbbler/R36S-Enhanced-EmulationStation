#include <string>
#pragma once
#ifndef ES_CORE_COMPONENTS_BUTTON_COMPONENT_H
#define ES_CORE_COMPONENTS_BUTTON_COMPONENT_H

#include "components/NinePatchComponent.h"
#include "GuiComponent.h"

class TextCache;
class ImageComponent;

class ButtonComponent : public GuiComponent
{
public:
	ButtonComponent(Window* window, const std::string& text = "", const std::string& helpText = "", const std::function<void()>& func = nullptr, bool upperCase = true);

	void setPressedFunc(std::function<void()> f);

	void setEnabled(bool enable);

	bool input(InputConfig* config, Input input) override;
	void render(const Transform4x4f& parentTrans) override;

	void setText(const std::string& text, const std::string& helpText, bool upperCase = true);
	void setIcon(const std::string& path, float pixelSize = 0.0f); // render an SVG/PNG icon (fixed size) instead of the text label
	void setKeyFill(unsigned int unfocusedBackColor, float insetPx = 0.0f, float radiusPx = 0.0f); // solid rounded-rect fill (both states) inset by insetPx, dark unfocused background; text colors unchanged

	inline const std::string& getText() const { return mText; };
	inline const std::function<void()>& getPressedFunc() const { return mPressedFunc; };

	void onSizeChanged() override;
	void onFocusGained() override;
	void onFocusLost() override;

	void setColorShift(unsigned int color) { mModdedColor = color; mNewColor = true; updateImage(); }
	void removeColorShift() { mNewColor = false; updateImage(); }

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

	void setRenderNonFocusedBackground(bool value) { mRenderNonFocusedBackground = value; }

	Vector4f getPadding() { return mPadding; }
	void setPadding(const Vector4f padding);

private:
	std::shared_ptr<Font> mFont;
	std::function<void()> mPressedFunc;

	bool mFocused;
	bool mEnabled;
	bool mNewColor = false;
	bool mForceFilledBackground = false; // always draw the solid (filled) box, even when unfocused
	bool mRoundRectFill = false;         // draw the background as a solid rounded-rect (drawRoundRect) instead of the ninepatch
	float mKeyInset = 0.0f;              // inset (px) of the rounded-rect fill on every side => gap between keys is 2x this
	float mCornerRadius = 0.0f;          // rounded-rect corner radius in px
	bool mRenderNonFocusedBackground;

	Vector4f	mPadding;

	unsigned int mTextColorFocused;
	unsigned int mTextColorUnfocused;
	unsigned int mModdedColor;

	unsigned int getCurTextColor() const;
	unsigned int getCurBackColor()  const;

	void updateImage();

	std::string mText;
	std::string mHelpText;
	std::unique_ptr<TextCache> mTextCache;
	std::shared_ptr<ImageComponent> mIcon;
	float mIconSize = 0.0f; // fixed icon side length in pixels; <=0 falls back to half the button height
	NinePatchComponent mBox;
	
	unsigned int mColor;
	unsigned int mColorFocused;
};

#endif // ES_CORE_COMPONENTS_BUTTON_COMPONENT_H
