#include <string>
#include "guis/GuiTextEditPopupKeyboard.h"
#include "components/MenuComponent.h"
#include "utils/StringUtil.h"
#include "Log.h"
#include "EsLocale.h"
#include "SystemConf.h"
#include "Settings.h"
#include "ThemeData.h"
#include "resources/ResourceManager.h"

#define OSK_WIDTH (Renderer::isSmallScreen() ? Renderer::getScreenWidth() : Renderer::getScreenWidth() * 0.78f)
#define OSK_HEIGHT (Renderer::isSmallScreen() ? Renderer::getScreenHeight() : Renderer::getScreenHeight() * 0.60f)

#define OSK_PADDINGX (Renderer::getScreenWidth() * 0.02f)
#define OSK_PADDINGY (Renderer::getScreenWidth() * 0.01f)

#define BUTTON_GRID_HORIZ_PADDING (Renderer::getScreenWidth()*0.0052083333)
#define BUTTON_LAYER_SIZE (4)

// Simplified search keyboard: two layers only (lowercase + SHIFT for uppercase).
// No ALT/accent layers. SHIFT lives at the bottom-right of the grid; ENTER is a
// 2-wide key on the a-row; the footer is RESET / SPACE / CANCEL with gamepad hints.
// (The 3rd/4th rows of each group are the unused ALT layers, kept == the base rows
//  so BUTTON_LAYER_SIZE stays 4 and the FR/KR layouts below are unaffected.)
std::vector<std::vector<const char*>> kbUs {

	{ "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "DEL" },
	{ "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "DEL" },
	{ "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "DEL" },
	{ "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "DEL" },

	{ "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "\\" },
	{ "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "|" },
	{ "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "\\" },
	{ "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "|" },

	{ "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "OK", "-colspan-" },
	{ "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\"", "OK", "-colspan-" },
	{ "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "OK", "-colspan-" },
	{ "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\"", "OK", "-colspan-" },

	{ "~", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "SHIFT", "-colspan-" },
	{ "~", "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "SHIFT", "-colspan-" },
	{ "~", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "SHIFT", "-colspan-" },
	{ "~", "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "SHIFT", "-colspan-" },

	{ "RESET (X)", "-colspan-", "-colspan-", "SPACE (R1)", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "CANCEL (B)", "-colspan-", "-colspan-" },
	{ "RESET (X)", "-colspan-", "-colspan-", "SPACE (R1)", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "CANCEL (B)", "-colspan-", "-colspan-" },
	{ "RESET (X)", "-colspan-", "-colspan-", "SPACE (R1)", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "CANCEL (B)", "-colspan-", "-colspan-" },
	{ "RESET (X)", "-colspan-", "-colspan-", "SPACE (R1)", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "CANCEL (B)", "-colspan-", "-colspan-" }
};

std::vector<std::vector<const char*>> kbFr {
	{ "&", "é", "\"", "'", "(", "#", "è", "!", "ç", "à", ")", "-", "DEL" },
	{ "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "@", "_", "DEL" },
	{ "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "@", "_", "DEL" },
	{ "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "@", "_", "DEL" },
	
	{ "a", "z", "e", "r", "t", "y", "u", "i", "o", "p", "^", "$", "OK" },
	{ "A", "Z", "E", "R", "T", "Y", "U", "I", "O", "P", "¨", "*", "OK" },
	{ "à", "ä", "ë", "ì", "ï", "ò", "ö", "ü", "\\", "|", "§", "°", "OK" },
	{ "à", "ä", "ë", "ì", "ï", "ò", "ö", "ü", "\\", "|", "§", "°", "OK" },

	{ "q", "s", "d", "f", "g", "h", "j", "k", "l", "m", "ù", "`", "-rowspan-" },
	{ "Q", "S", "D", "F", "G", "H", "J", "K", "L", "M", "%", "£", "-rowspan-" },
	{ "á", "â", "ê", "í", "î", "ó", "ô", "ú", "û", "ñ", "¡", "¿", "-rowspan-" },
	{ "á", "â", "ê", "í", "î", "ó", "ô", "ú", "û", "ñ", "¡", "¿", "-rowspan-" },

	{ "<", "w", "x", "c", "v", "b", "n", ",", ";", ":", "=", "ALT", "-colspan-" },
	{ ">", "W", "X", "C", "V", "B", "N", "?", ".", "/", "+", "ALT", "-colspan-" },
	{ "€", "", "", "", "", "", "", "", "", "", "", "ALT", "-colspan-" },
	{ "€", "", "", "", "", "", "", "", "", "", "", "ALT", "-colspan-" },

	{ "SHIFT", "-colspan-", "SPACE", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "RESET", "-colspan-", "CANCEL", "-colspan-" },
	{ "SHIFT", "-colspan-", "SPACE", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "RESET", "-colspan-", "CANCEL", "-colspan-" },
	{ "SHIFT", "-colspan-", "SPACE", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "RESET", "-colspan-", "CANCEL", "-colspan-" },
	{ "SHIFT", "-colspan-", "SPACE", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "RESET", "-colspan-", "CANCEL", "-colspan-" }
};

std::vector<std::vector<const char*>> kbKr{
	{ "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "DEL" },
	{ "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "DEL" },
	{ "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "DEL" },
	{ "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "DEL" },

	{ "ㅂ", "ㅈ", "ㄷ", "ㄱ", "ㅅ", "ㅛ", "ㅕ", "ㅑ", "ㅐ", "ㅔ", "[", "]", "OK" },
	{ "ㅃ", "ㅉ", "ㄸ", "ㄲ", "ㅆ", "ㅛ", "ㅕ", "ㅑ", "ㅒ", "ㅖ", "{", "}", "OK" },
	{ "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "OK" },
	{ "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "OK" },

	{ "ㅁ", "ㄴ", "ㅇ", "ㄹ", "ㅎ", "ㅗ", "ㅓ", "ㅏ", "ㅣ", ";", "'", "\\", "-rowspan-" },
	{ "ㅁ", "ㄴ", "ㅇ", "ㄹ", "ㅎ", "ㅗ", "ㅓ", "ㅏ", "ㅣ", ":", "\"", "|", "-rowspan-" },
	{ "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "\\", "-rowspan-" },
	{ "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\"", "|", "-rowspan-" },

	{ "ㅋ", "ㅌ", "ㅊ", "ㅍ", "ㅠ", "ㅜ", "ㅡ", ",", ".", "/", "`", "ALT", "-colspan-" },
	{ "ㅋ", "ㅌ", "ㅊ", "ㅍ", "ㅠ", "ㅜ", "ㅡ", "<", ">", "?", "~", "ALT", "-colspan-" },
	{ "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "`", "ALT", "-colspan-" },
	{ "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "~", "ALT", "-colspan-" },

	{ "SHIFT", "-colspan-", "SPACE", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "RESET", "-colspan-", "CANCEL", "-colspan-" },
	{ "SHIFT", "-colspan-", "SPACE", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "RESET", "-colspan-", "CANCEL", "-colspan-" },
	{ "SHIFT", "-colspan-", "SPACE", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "RESET", "-colspan-", "CANCEL", "-colspan-" },
	{ "SHIFT", "-colspan-", "SPACE", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "-colspan-", "RESET", "-colspan-", "CANCEL", "-colspan-" }
};

GuiTextEditPopupKeyboard::GuiTextEditPopupKeyboard(Window* window, const std::string& title, const std::string& initValue,
	const std::function<void(const std::string&)>& okCallback, bool multiLine, const std::string acceptBtnText)
	: GuiComponent(window), mBackground(window, ":/frame.png"), mGrid(window, Vector2i(1, 6)), mMultiLine(multiLine)
{
	setTag("popup");

	mOkCallback = okCallback;

	auto theme = ThemeData::getMenuTheme();
	mBackground.setImagePath(theme->Background.path);
	mBackground.setEdgeColor(theme->Background.color);
	mBackground.setCenterColor(theme->Background.centerColor);
	mBackground.setCornerSize(theme->Background.cornerSize);

	// Optional themed SVG icons for the four special keys (backspace/enter/shift/alt).
	// Falls back to the built-in Unicode glyphs when the theme doesn't provide them.
	// Icons render at a FIXED size (same on every key); theme 'iconSize' is a fraction
	// of screen height, defaulting to ~20px @480 so it reads like the letter keys.
	mIconSizePx = Renderer::getScreenHeight() * 0.0416666667f;
	// The popup itself is nudged down slightly for title alignment; remember that offset so a
	// theme 'posY' (measured from the screen top) can be converted to a popup-local position.
	mPopupOffsetY = Renderer::getScreenHeight() * 0.0178f;
	if (ThemeData* defTheme = ThemeData::getDefaultTheme())
	{
		const ThemeData::ThemeElement* kb = defTheme->getElement("screen", "keyboard", "keyboard");
		if (kb)
		{
			if (kb->has("backspace") && ResourceManager::getInstance()->fileExists(kb->get<std::string>("backspace")))
				mIconBackspace = kb->get<std::string>("backspace");
			if (kb->has("enter") && ResourceManager::getInstance()->fileExists(kb->get<std::string>("enter")))
				mIconEnter = kb->get<std::string>("enter");
			if (kb->has("shift") && ResourceManager::getInstance()->fileExists(kb->get<std::string>("shift")))
				mIconShift = kb->get<std::string>("shift");
			if (kb->has("alt") && ResourceManager::getInstance()->fileExists(kb->get<std::string>("alt")))
				mIconAlt = kb->get<std::string>("alt");
			if (kb->has("iconSize"))
				mIconSizePx = Renderer::getScreenHeight() * kb->get<float>("iconSize");
			if (kb->has("width"))
				mKbWidthPx = Renderer::getScreenWidth() * kb->get<float>("width");
			if (kb->has("keyColor"))
			{
				mKeyFillColor = kb->get<unsigned int>("keyColor");
				mHasKeyFill = true;
			}
			if (kb->has("posY"))
				mKbTopPx = Renderer::getScreenHeight() * kb->get<float>("posY");
			if (kb->has("keySpacing"))
				mKeyPadPx = Renderer::getScreenWidth() * kb->get<float>("keySpacing") / 2.0f; // half each side => full gap between keys
			if (kb->has("keyRadius"))
				mKeyRadiusPx = Renderer::getScreenHeight() * kb->get<float>("keyRadius");
		}
	}

	addChild(&mBackground);
	addChild(&mGrid);

	mTitle = std::make_shared<TextComponent>(mWindow, Utils::String::toUpper(title), theme->Title.font, theme->Title.color, ALIGN_CENTER);
	
	mKeyboardGrid = std::make_shared<ComponentGrid>(mWindow, Vector2i(kbUs[0].size(), kbUs.size() / BUTTON_LAYER_SIZE));

	mText = std::make_shared<TextEditComponent>(mWindow);
	mText->setValue(initValue);

	if (!multiLine)
		mText->setCursor(initValue.size());

	// Header
	mGrid.setEntry(mTitle, Vector2i(0, 0), false, true);

	// Text edit add
	mGrid.setEntry(mText, Vector2i(0, 1), true, false, Vector2i(1, 1), GridFlags::BORDER_TOP);

	std::vector< std::vector< std::shared_ptr<ButtonComponent> > > buttonList;

	// Keyboard
	// Case for if multiline is enabled, then don't create the keyboard.
	if (!mMultiLine) 
	{
		std::vector<std::vector<const char*>>* layout = &kbUs;

		std::string language = Settings::getInstance()->getString("Language");
		if (language == "fr")
			layout = &kbFr;
		else if (language == "ko")
			layout = &kbKr;

		for (unsigned int i = 0; i < layout->size() / BUTTON_LAYER_SIZE; i++)
		{			
			std::vector<std::shared_ptr<ButtonComponent>> buttons;
			for (unsigned int j = 0; j < (*layout)[i].size(); j++)
			{
				std::string lower = (*layout)[BUTTON_LAYER_SIZE * i][j];
				if (lower.empty() || lower == "-rowspan-" || lower == "-colspan-")
					continue;

				const std::string specialKey = lower; // raw label ("DEL"/"OK"/"SHIFT"/"ALT") for themed-icon lookup

				std::string upper = (*layout)[BUTTON_LAYER_SIZE * i + 1][j];
				std::string lowerAlted = (*layout)[BUTTON_LAYER_SIZE * i + 2][j];
				std::string upperAlted = (*layout)[BUTTON_LAYER_SIZE * i + 3][j];

				std::shared_ptr<ButtonComponent> button = nullptr;

				if (lower == "DEL")
				{
					lower = _U("\u232B");
					upper = _U("\u232B");
					lowerAlted = _U("\u232B");
					upperAlted = _U("\u232B");
				}
				else if (lower == "OK")
				{
					lower = _U("\u23CE");
					upper = _U("\u23CE");
					lowerAlted = _U("\u23CE");
					upperAlted = _U("\u23CE");
				}
				else if (lower != "SHIFT" && lower.length() > 1)
				{
					lower = _(lower.c_str());
					upper = _(upper.c_str());
					lowerAlted = _(lowerAlted.c_str());
					upperAlted = _(upperAlted.c_str());
				}

				if (lower == "SHIFT")
				{
					// Special case for shift key
					mShiftButton = std::make_shared<ButtonComponent>(mWindow, _U("\u21E7"), _("SHIFTS FOR UPPER,LOWER, AND SPECIAL"), [this] { shiftKeys(); }, false);					
					button = mShiftButton;
				}
				else if (lower == "ALT")
				{
					mAltButton = std::make_shared<ButtonComponent>(mWindow, _U("\u2387"), _("ALT GR"), [this] { altKeys(); }, false);
					button = mAltButton;
				}
				else
					button = makeButton(lower, upper, lowerAlted, upperAlted);

				// Themed SVG icons override the default Unicode glyphs on the four special keys.
				// All four render at the same fixed size, independent of key width.
				if (specialKey == "DEL" && !mIconBackspace.empty())
					button->setIcon(mIconBackspace, mIconSizePx);
				else if (specialKey == "OK" && !mIconEnter.empty())
					button->setIcon(mIconEnter, mIconSizePx);
				else if (specialKey == "SHIFT" && !mIconShift.empty())
					button->setIcon(mIconShift, mIconSizePx);
				else if (specialKey == "ALT" && !mIconAlt.empty())
					button->setIcon(mIconAlt, mIconSizePx);

				float keyPad = (mKeyPadPx >= 0.0f) ? mKeyPadPx : (BUTTON_GRID_HORIZ_PADDING / 4.0f);
				button->setPadding(Vector4f(keyPad, keyPad, keyPad, keyPad));
				button->setRenderNonFocusedBackground(false);
				// When the theme provides a key fill color, give unfocused keys a solid dark fill
				// (focused key keeps its white pill with black text). The inset creates the gap
				// between keys and the radius rounds each key's corners. Applies to all keys,
				// including the footer action keys (RESET/SPACE/CANCEL).
				if (mHasKeyFill)
				{
					float inset = (mKeyPadPx >= 0.0f) ? mKeyPadPx : (BUTTON_GRID_HORIZ_PADDING / 4.0f);
					button->setKeyFill(mKeyFillColor, inset, mKeyRadiusPx);
				}
				buttons.push_back(button);

				int colSpan = 1;
				for (unsigned int cs = j + 1; cs < (*layout)[i].size(); cs++)
				{
					if (std::string((*layout)[BUTTON_LAYER_SIZE * i][cs]) == "-colspan-")
						colSpan++;
					else
						break;
				}
				
				int rowSpan = 1;
				for (unsigned int cs = (BUTTON_LAYER_SIZE * i) + BUTTON_LAYER_SIZE; cs < layout->size(); cs += BUTTON_LAYER_SIZE)
				{
					if (std::string((*layout)[cs][j]) == "-rowspan-")
						rowSpan++;
					else
						break;
				}

				mKeyboardGrid->setEntry(button, Vector2i(j, i), true, true, Vector2i(colSpan, rowSpan));

				buttonList.push_back(buttons);
			}
		}		
		// END KEYBOARD IF
	}

	// Add keyboard keys
	mGrid.setEntry(mKeyboardGrid, Vector2i(0, 2), true, true, Vector2i(2, 4));

	// Determine size from text size
	float textHeight = mText->getFont()->getHeight();
	if (multiLine)
		textHeight *= 6;

	mText->setSize(0, textHeight);

	mGrid.setUnhandledInputCallback([this](InputConfig* config, Input input) -> bool 
	{		
		if (config->isMappedLike("down", input)) 
		{
			mGrid.setCursorTo(mText);
			return true;
		}
		else if (config->isMappedLike("up", input)) 
		{
			mGrid.moveCursor(Vector2i(0, kbUs.size() / 2));
			return true;
		}
		else if (config->isMappedLike("left", input))
		{		
			if (mGrid.getSelectedComponent() == mKeyboardGrid)
			{
				Vector2i curCursor = mKeyboardGrid->getCursor();
				mKeyboardGrid->setCursorTo(Vector2i(kbUs[0].size() - 1, curCursor.y()));
				return true;
			}
		}
		else if (config->isMappedLike("right", input))
		{
			if (mGrid.getSelectedComponent() == mKeyboardGrid)
			{
				Vector2i curCursor = mKeyboardGrid->getCursor();
				mKeyboardGrid->setCursorTo(Vector2i(0, curCursor.y()));
				return true;
			}
		}

		return false;
	});

	
	// If multiline, set all diminsions back to default, else draw size for keyboard.
	if (mMultiLine) 
	{
		if (Renderer::isSmallScreen())
			setSize(OSK_WIDTH, Renderer::getScreenHeight());
		else
			setSize(OSK_WIDTH, mTitle->getFont()->getHeight() + textHeight + mKeyboardGrid->getSize().y() + 40);

		setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2);
	}
	else
	{
		//setSize(OSK_WIDTH, mTitle->getFont()->getHeight() + textHeight + 40 + (Renderer::getScreenHeight() * 0.085f) * 6);
		setSize(OSK_WIDTH, OSK_HEIGHT);
		// Shift the whole popup down a touch so its title lines up with the
		// top-anchored menu title (which sits below the popup's flush-top title).
		float kbYOffset = mPopupOffsetY; // ~8.5px (2px higher than 0.022)
		setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2 + kbYOffset);
		animateTo(Vector2f((Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2 + kbYOffset));
	}
}


void GuiTextEditPopupKeyboard::onSizeChanged()
{
	mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));

	mText->setSize(mSize.x() - OSK_PADDINGX - OSK_PADDINGX, mText->getSize().y());

	// update grid
	mGrid.setRowHeightPerc(0, mTitle->getFont()->getHeight() / mSize.y());
	mGrid.setRowHeightPerc(2, mKeyboardGrid->getSize().y() / mSize.y());
	mGrid.setSize(mSize);

	auto pos = mKeyboardGrid->getPosition();
	auto sz = mKeyboardGrid->getSize();

	// Key-grid width: theme 'width' (fixed px) if set, else the default padding-based width.
	// The grid is horizontally centered, so the side padding is (screen - gridWidth) / 2
	// (e.g. 624px grid on a 640px screen => 8px each side, 48px per column across 13 columns).
	float kbWidth = (mKbWidthPx > 0.0f) ? mKbWidthPx : (mSize.x() - OSK_PADDINGX - OSK_PADDINGX);
	float kbMarginX = (mSize.x() - kbWidth) / 2.0f;

	// Key-grid top: theme 'posY' pins it to an absolute distance from the screen top
	// (converted to popup-local by subtracting the popup's own offset); else auto-computed.
	float kbTop = (mKbTopPx > 0.0f) ? (mKbTopPx - mPopupOffsetY) : pos.y();

	mKeyboardGrid->setSize(kbWidth, sz.y() - OSK_PADDINGY); // Small margin between buttons
	mKeyboardGrid->setPosition(kbMarginX, kbTop);
}

bool GuiTextEditPopupKeyboard::input(InputConfig* config, Input input)
{
	if (GuiComponent::input(config, input))
		return true;

	// pressing start
	if (config->isMappedTo("start", input) && input.value)
	{
		if (mOkCallback)
			mOkCallback(mText->getValue());

		delete this;
		return true;
	}

	if ((config->getDeviceId() == DEVICE_KEYBOARD && input.id == SDLK_ESCAPE))
	{
		delete this;
		return true;
	}

	// pressing back when not text editing closes us
	if (config->isMappedTo(BUTTON_BACK, input) && input.value)
	{
		delete this;
		return true;
	}

#ifdef _ENABLEEMUELEC
	// For deleting a chara (Left Top Button)
	if (config->isMappedTo("lefttrigger", input) && input.value) {
		mText->startEditing();
		mText->textInput("\b");
		mText->stopEditing();
	}

	// For Adding a space (Right Top Button)
	if (config->isMappedTo("righttrigger", input) && input.value) {
		mText->startEditing();
		mText->textInput(" ");
	}
#else

	// Delete a char (Left shoulder / L1)
	if (config->isMappedTo("pageup", input) && input.value) {
		bool editing = mText->isEditing();
		if (!editing)
			mText->startEditing();

		mText->textInput("\b");

		if (!editing)
			mText->stopEditing();
	}

	// SPACE (Right shoulder / R1) — matches the on-screen "SPACE (R1)" footer hint
	if (config->isMappedTo("pagedown", input) && input.value)
	{
		bool editing = mText->isEditing();
		if (!editing)
			mText->startEditing();

		mText->textInput(" ");

		if (!editing)
			mText->stopEditing();
	}
#endif
	// For Shifting (Y)
	if (config->isMappedTo("y", input) && input.value) 
		shiftKeys();

	if (config->isMappedTo("x", input) && input.value && mOkCallback != nullptr)
	{
		bool editing = mText->isEditing();
		if (!editing)
			mText->startEditing();

		mText->setValue("");

		if (!editing)
			mText->stopEditing();
	}

	// ENTER / accept the search (Right trigger / R2)
	if (config->isMappedTo("righttrigger", input) && input.value && mOkCallback != nullptr)
	{
		mOkCallback(mText->getValue());
		delete this;
		return true;
	}

	return false;
}

void GuiTextEditPopupKeyboard::toggleKeyState(bool& state, std::shared_ptr<ButtonComponent>& button)
{
	state = !state;

	if (state)
	{
		button->setRenderNonFocusedBackground(true);
		button->setColorShift(0xFF0000FF);
	}
	else
	{
		button->setRenderNonFocusedBackground(false);
		button->removeColorShift();
	}

	updateKeyboardButtons();
}

void GuiTextEditPopupKeyboard::updateKeyboardButtons()
{
	for (auto& kb : keyboardButtons)
	{
		const std::string& text = (mAlt && mShift) ? kb.altedShiftedKey
			: (mAlt) ? kb.altedKey
			: (mShift) ? kb.shiftedKey
			: kb.key;

		auto sz = kb.button->getSize();
		kb.button->setText(text, text, false);
		kb.button->setSize(sz);
	}
}

void GuiTextEditPopupKeyboard::shiftKeys()
{
	toggleKeyState(mShift, mShiftButton);
}

void GuiTextEditPopupKeyboard::altKeys()
{
	if (mShift)
		toggleKeyState(mShift, mShiftButton);

	toggleKeyState(mAlt, mAltButton);
}

std::vector<HelpPrompt> GuiTextEditPopupKeyboard::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mGrid.getHelpPrompts();

	if (mOkCallback != nullptr)
		prompts.push_back(HelpPrompt("x", _("RESET")));

	prompts.push_back(HelpPrompt("y", _("SHIFT")));
	prompts.push_back(HelpPrompt("start", _("OK")));
	prompts.push_back(HelpPrompt(BUTTON_BACK, _("BACK")));
	prompts.push_back(HelpPrompt("r", _("SPACE")));
	prompts.push_back(HelpPrompt("l", _("DELETE")));
	return prompts;
}

std::shared_ptr<ButtonComponent> GuiTextEditPopupKeyboard::makeButton(const std::string& key, const std::string& shiftedKey, const std::string& altedKey, const std::string& altedShiftedKey)
{
	std::shared_ptr<ButtonComponent> button = std::make_shared<ButtonComponent>(mWindow, key, key, [this, key, shiftedKey, altedKey, altedShiftedKey]
	{
		if (key == _U("\u23CE") || key.find("OK") != std::string::npos)
		{
			mOkCallback(mText->getValue());
			delete this;
			return;
		}
		else if (key == _U("\u232B") || key == "DEL")
		{
			mText->startEditing(); mText->textInput("\b"); mText->stopEditing();
			return;
		}
		else if (key.find("SPACE") != std::string::npos || key == " ")
		{
			mText->startEditing(); mText->textInput(" "); mText->stopEditing();
			return;
		}
		else if (key.find("RESET") != std::string::npos)
		{
			mText->startEditing();
			mText->setValue("");
			mText->stopEditing();
            return;
		}
		else if (key.find("CANCEL") != std::string::npos)
		{
			delete this;
			return;
		}

		if (mAlt)
		{
			if (mShift && altedShiftedKey.empty())
				return;
			if (altedKey.empty())
				return;
		}

		mText->startEditing();

		const char* text;
		if (mAlt && mShift)
			text = altedShiftedKey.c_str();
		else if (mAlt)
			text = altedKey.c_str();
		else if (mShift)
			text = shiftedKey.c_str();
		else
			text = key.c_str();
		mText->textInput(text);

		mText->stopEditing();

		if (Utils::String::isKorean(text) && mShift)
			shiftKeys();
	}, false);
	
	KeyboardButton kb(button, key, shiftedKey, altedKey, altedShiftedKey);
	keyboardButtons.push_back(kb);
	return button;
}
