#include <string>
#include "GuiComponent.h"

#include "components/NinePatchComponent.h"
#include "components/ButtonComponent.h"
#include "components/ComponentGrid.h"
#include "components/TextEditComponent.h"
#include "components/TextComponent.h"
#include <functional>

class GuiTextEditPopupKeyboard : public GuiComponent
{
public:
	GuiTextEditPopupKeyboard(Window* window, const std::string& title, const std::string& initValue,
		const std::function<void(const std::string&)>& okCallback, bool multiLine, const std::string acceptBtnText = "OK");

	bool input(InputConfig* config, Input input);
	//void update(int deltatime) override;
	void onSizeChanged();
	std::vector<HelpPrompt> getHelpPrompts() override;

private:
	class KeyboardButton
	{
	public:
		std::shared_ptr<ButtonComponent> button;
		const std::string key;
		const std::string shiftedKey;
		const std::string altedKey;
		const std::string altedShiftedKey;
		KeyboardButton(const std::shared_ptr<ButtonComponent> b, const std::string& k, const std::string& sk, const std::string& ak, const std::string& ask) : button(b), key(k), shiftedKey(sk), altedKey(ak), altedShiftedKey(ask) {};
	};
	
	std::shared_ptr<ButtonComponent> makeButton(const std::string& key, const std::string& shiftedKey, const std::string& altedKey, const std::string& altedShiftedKey);
	std::vector<KeyboardButton> keyboardButtons;
	
	std::shared_ptr<ButtonComponent> mShiftButton;	
	std::shared_ptr<ButtonComponent> mAltButton;

	void toggleKeyState(bool& state, std::shared_ptr<ButtonComponent>& button);
	void updateKeyboardButtons();
	void shiftKeys();
	void altKeys();

	NinePatchComponent mBackground;
	ComponentGrid mGrid;

	std::shared_ptr<TextComponent> mTitle;
	std::shared_ptr<TextEditComponent> mText;
	std::shared_ptr<ComponentGrid> mKeyboardGrid;
	
	std::function<void(const std::string&)> mOkCallback;

	bool mMultiLine;
	bool mShift = false;
	bool mAlt = false;

	// Themed SVG icon paths for the special keys (empty => use built-in Unicode glyph)
	std::string mIconBackspace;
	std::string mIconEnter;
	unsigned int mKeyPressColor = 0;
	int mKeyPressMs = 100;
	std::string mIconShift;
	std::string mIconShiftActive;
	std::string mIconAltActive;
	std::string mIconAlt;
	float mIconSizePx = 0.0f; // fixed on-screen icon side length (px); theme 'iconSize' fraction * screen height
	float mKbWidthPx = 0.0f;  // fixed key-grid width (px); theme 'width' fraction * screen width. <=0 => default padding-based width
	bool mHasKeyFill = false;      // theme provided a 'keyColor' => give unfocused keys a solid fill
	unsigned int mKeyFillColor = 0; // unfocused key background color
	float mKbTopPx = 0.0f;        // fixed key-grid top from screen top (px); theme 'posY' * screen height. <=0 => auto
	float mPopupOffsetY = 0.0f;   // the popup's own y offset, so an absolute key-grid top can be converted to local
	std::shared_ptr<Font> mKeyFont;  // themed key-letter font; null = inherit the menu font
	float mTopGap = 0.0f;         // how far the popup sits below the screen top, so the background can cover it
	float mKeyPadPx = -1.0f;      // per-side key padding (px) => gap between keys is 2x this; theme 'keySpacing'. <0 => default
	float mKeyRadiusPx = 0.0f;    // key corner radius (px); theme 'keyRadius' * screen height
};

