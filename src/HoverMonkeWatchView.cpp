
#include "HoverMonkeWatchView.hpp"
#include "config.hpp"
#include "monkecomputer/shared/ViewLib/MonkeWatch.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "UnityEngine/Vector3.hpp"

DEFINE_TYPE(HoverMonke, HoverMonkeWatchView);

using namespace GorillaUI;
using namespace UnityEngine;

extern bool isRoom;
extern void powerCheck();
extern void carSpeedCheck();

namespace HoverMonke
{
    void HoverMonkeWatchView::Awake()
    {
        toggleHandler = new UIToggleInputHandler(EKeyboardKey::Enter, EKeyboardKey::Enter, true);
        settingSelector = new UISelectionHandler(EKeyboardKey::Up, EKeyboardKey::Down, EKeyboardKey::Enter, true, false);
        powerSelector = new UISelectionHandler(EKeyboardKey::Left, EKeyboardKey::Right, EKeyboardKey::Enter, false, true);
        carSpeedSelector = new UISelectionHandler(EKeyboardKey::Left, EKeyboardKey::Right, EKeyboardKey::Enter, false, true);

        settingSelector->max = 4;
        powerSelector->max = 5;
        carSpeedSelector->max = 5;

        powerSelector->currentSelectionIndex = config.power;
        carSpeedSelector->currentSelectionIndex = config.carSpeed;
    }

    void HoverMonkeWatchView::DidActivate(bool firstActivation)
    {
        std::function<void(bool)> fun = std::bind(&HoverMonkeWatchView::OnEnter, this, std::placeholders::_1);
        settingSelector->selectionCallback = fun;
        Redraw();
    }

    void HoverMonkeWatchView::OnEnter(int index)
    {
        if(index == 0) config.enabled ^= 1;
        if(index == 1) config.carMode ^= 1;
    }

    void HoverMonkeWatchView::Redraw()
    {
        text = "";

        DrawHeader();
        DrawBody();

        MonkeWatch::Redraw();
    }

    void HoverMonkeWatchView::DrawHeader()
    {
        text += "<color=#000000><<</color> <color=#FF0000>Ho</color><color=#FF8700>v</color><color=#FFFB00>er</color><color=#0FFF00>M</color><color=#0036FF>on</color><color=#B600FF>k</color><color=#FF00B6>e</color> <color=#000000>>></color>\n";
    }

    void HoverMonkeWatchView::DrawBody()
    {
        text += settingSelector->currentSelectionIndex == 0 ? "<color=#fd0000>></color>" : " ";
        text += config.enabled ? "<color=#00ff00>Enabled</color>" : "<color=#ff0000>Disabled</color>";

        text += "\n";
        text += "<b><i>Car Mode:</i></b>\n";
        text += settingSelector->currentSelectionIndex == 1 ? "<color=#fd0000>></color>" : " ";
        text += config.carMode ? "<color=#00ff00>Enabled</color>" : "<color=#ff0000>Disabled</color>";

        text += "\n\n";
        text += "<b><i>Hover Speed:</i></b>\n";
        text += settingSelector->currentSelectionIndex == 2 ? "<color=#ff0000><></color> " : " ";
        text += "<color=#AADDAA><></color> ";
        switch (powerSelector->currentSelectionIndex) {
            case 0:
                text += "Default";
                break;
            case 1:
                text += "2";
                break;
            case 2:
                text += "3";
                break;
            case 3:
                text += "4";
                break;
            case 4:
                text += "Maximum";
                break;
            default:
                break;
        }
        text += "\n\n";
        text += "<b><i>Car Acceleration:</i></b>\n";
        text += settingSelector->currentSelectionIndex == 3 ? "<color=#ff0000><></color> " : " ";
        text += "<color=#AADDAA><></color> ";
        switch (carSpeedSelector->currentSelectionIndex) {
            case 0:
                text += "Default";
                break;
            case 1:
                text += "2";
                break;
            case 2:
                text += "3";
                break;
            case 3:
                text += "4";
                break;
            case 4:
                text += "Maximum";
                break;
            default:
                break;
        }

        if (config.enabled && !isRoom)
        {
            text += "\n\nBut is disabled\ndue to not being in\na private room\n";
        }
        text += "\nMade by Buzz Bzzz bzz BZZZ The 18th#0431\nBLM\nQueer/Trans Pride";
    }

    void HoverMonkeWatchView::OnKeyPressed(int value)
    {
        EKeyboardKey key = (EKeyboardKey)value;
        if (!settingSelector->HandleKey(key)) // if it was not up/down/enter
        {
            switch (settingSelector->currentSelectionIndex)
            {
                case 0:
                    break;
                case 1:
                    break;
                case 2:
                    powerSelector->HandleKey(key);
                    break;
                case 3:
                    carSpeedSelector->HandleKey(key);
                    break;
                default:
                    break;
            }

            config.power = powerSelector->currentSelectionIndex;
            config.carSpeed = carSpeedSelector->currentSelectionIndex;
            carSpeedCheck();
            powerCheck();
            SaveConfig();
        }
        Redraw();
    }
}