#include <string>
#include "components/ButtonComponent.h"

#include "components/ImageComponent.h"
#include "resources/Font.h"
#include "utils/StringUtil.h"

ButtonComponent::ButtonComponent(Window* window, const std::string& text, const std::string& helpText, const std::function<void()>& func, bool upperCase) : GuiComponent(window),
	mBox(window, ThemeData::getMenuTheme()->Icons.button),
	mFont(Font::get(FONT_SIZE_MEDIUM)), 
	mFocused(false), 
	mEnabled(true), 
	mTextColorFocused(0xFFFFFFFF), mTextColorUnfocused(0x777777FF)
{
	auto menuTheme = ThemeData::getMenuTheme();

	mFont = menuTheme->Text.font;
	mTextColorUnfocused = menuTheme->Text.color;
	mTextColorFocused = menuTheme->Text.selectedColor;	
	mColor = menuTheme->Text.color;
	mColorFocused = menuTheme->Text.selectorColor;
	mRenderNonFocusedBackground = true;
	
	if (Renderer::isSmallScreen())
		mBox.setCornerSize(8, 8);

	setPressedFunc(func);
	setText(text, helpText, upperCase);
	updateImage();
}

void ButtonComponent::onSizeChanged()
{
	auto sz = mBox.getCornerSize();
	mBox.fitTo(mSize, Vector3f::Zero(), Vector2f(-sz.x() * 2, -sz.y() * 2));
}

void ButtonComponent::setPressedFunc(std::function<void()> f)
{
	mPressedFunc = f;
}

bool ButtonComponent::input(InputConfig* config, Input input)
{
	if(config->isMappedTo(BUTTON_OK, input) && input.value != 0)
	{
		if(mPressedFunc && mEnabled)
			mPressedFunc();
		return true;
	}

	return GuiComponent::input(config, input);
}

void ButtonComponent::setText(const std::string& text, const std::string& helpText, bool upperCase)
{
	mText = upperCase ? Utils::String::toUpper(text) : text;
	mHelpText = helpText;
	
	mTextCache = std::unique_ptr<TextCache>(mFont->buildTextCache(mText, 0, 0, getCurTextColor()));

	float minWidth = mFont->sizeText("DELETE").x() + 12;
	setSize(Math::max(mTextCache->metrics.size.x() + 12, minWidth), mTextCache->metrics.size.y());

	updateHelpPrompts();
}

void ButtonComponent::setIcon(const std::string& path, float pixelSize)
{
	if (path.empty())
	{
		mIcon.reset();
		return;
	}

	mIconSize = pixelSize;
	mIcon = std::make_shared<ImageComponent>(mWindow);
	mIcon->setImage(path);
	// tinted to the current label color; sizing/coloring is refreshed each frame in render()
	mIcon->setColorShift(getCurTextColor());
}

void ButtonComponent::setKeyFill(unsigned int unfocusedBackColor, float insetPx, float radiusPx)
{
	// Give unfocused keys a solid (dark) fill instead of a transparent background,
	// while the focused key keeps its themed selected fill. Text colors are untouched.
	// Drawn as a rounded-rect inset by insetPx (so the gap between keys is 2x insetPx),
	// giving an exact corner radius independent of the ninepatch button art.
	mColor = unfocusedBackColor;
	mForceFilledBackground = true;
	mRoundRectFill = true;
	mKeyInset = insetPx;
	mCornerRadius = radiusPx;
	mRenderNonFocusedBackground = true;
	updateImage();
}

void ButtonComponent::onFocusGained()
{
	mFocused = true;
	updateImage();
}

void ButtonComponent::onFocusLost()
{
	mFocused = false;
	updateImage();
}

void ButtonComponent::setEnabled(bool enabled)
{
	mEnabled = enabled;
	updateImage();
}

void ButtonComponent::updateImage()
{
	if(!mEnabled || !mPressedFunc)
	{
		mBox.setImagePath(ThemeData::getMenuTheme()->Icons.button_filled);
		mBox.setCenterColor(0x770000FF);
		mBox.setEdgeColor(0x770000FF);
		return;
	}

	// If a new color has been set.  
	if (mNewColor) {
		mBox.setImagePath(ThemeData::getMenuTheme()->Icons.button_filled);
		mBox.setCenterColor(mModdedColor);
		mBox.setEdgeColor(mModdedColor);
		return;
	}

	mBox.setCenterColor(getCurBackColor());
	mBox.setEdgeColor(getCurBackColor());
	// Force-filled keys use the solid box in both states so the unfocused fill is visible.
	if (mForceFilledBackground)
		mBox.setImagePath(ThemeData::getMenuTheme()->Icons.button_filled);
	else
		mBox.setImagePath(mFocused ? ThemeData::getMenuTheme()->Icons.button_filled : ThemeData::getMenuTheme()->Icons.button);
	//mBox.setImagePath(mFocused ? ":/button_filled.png" : ":/button.png");
}

void ButtonComponent::render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();

	if (mRoundRectFill)
	{
		// Solid rounded-rect key background (exact radius, real gaps from the inset).
		// Skip the fill for keys that are blank in the current layer (no text, no icon) —
		// e.g. accent-layer slots with no character — unless focused, so the cursor stays
		// visible if you navigate onto one.
		bool blank = mText.empty() && !mIcon;
		if (!blank || mFocused)
		{
			float pad = mKeyInset;
			Renderer::setMatrix(trans);
			Renderer::drawRoundRect(pad, pad, mSize.x() - 2.0f * pad, mSize.y() - 2.0f * pad, mCornerRadius, getCurBackColor());
		}
	}
	else if (mRenderNonFocusedBackground || mFocused)
		mBox.render(trans);

	if(mIcon)
	{
		// fixed icon size (identical on every key regardless of the key's width),
		// aspect-preserved and tinted to the label color
		float box = (mIconSize > 0.0f) ? mIconSize : (mSize.y() * 0.5f);
		mIcon->setMaxSize(box, box);
		mIcon->setColorShift(getCurTextColor());
		Vector3f centerOffset((mSize.x() - mIcon->getSize().x()) / 2, (mSize.y() - mIcon->getSize().y()) / 2, 0);
		mIcon->setPosition(centerOffset);
		mIcon->render(trans);
	}
	else if(mTextCache)
	{
		Vector3f centerOffset((mSize.x() - mTextCache->metrics.size.x()) / 2, (mSize.y() - mTextCache->metrics.size.y()) / 2, 0);
		trans = trans.translate(centerOffset);

		Renderer::setMatrix(trans);
		mTextCache->setColor(getCurTextColor());
		mFont->renderTextCache(mTextCache.get());
		trans = trans.translate(-centerOffset);
	}

	renderChildren(trans);
}

unsigned int ButtonComponent::getCurTextColor() const
{
	if(!mFocused)
		return mTextColorUnfocused;
	else
		return mTextColorFocused;
}

unsigned int ButtonComponent::getCurBackColor() const
{
	if (!mFocused)
		return mColor;
	else
		return mColorFocused;
}

std::vector<HelpPrompt> ButtonComponent::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt(BUTTON_OK, mHelpText.empty() ? mText.c_str() : mHelpText.c_str()));
	return prompts;
}

void ButtonComponent::setPadding(const Vector4f padding)
{
	if (mPadding == padding)
		return;

	mPadding = padding;
	onSizeChanged();
}
