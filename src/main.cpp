#include <thread>
#include "modloader/shared/modloader.hpp"
#include "GorillaLocomotion/Player.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/utils-functions.h"
#include "beatsaber-hook/shared/utils/typedefs.h"
#include "beatsaber-hook/shared/utils/utils.h"
#include "beatsaber-hook/shared/utils/il2cpp-utils-methods.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "GlobalNamespace/OVRInput.hpp"
#include "GlobalNamespace/OVRInput_Button.hpp"
#include "gorilla-utils/shared/GorillaUtils.hpp"
#include "gorilla-utils/shared/CustomProperties/Player.hpp"
#include "gorilla-utils/shared/Utils/Player.hpp"
#include "gorilla-utils/shared/Callbacks/InRoomCallbacks.hpp"
#include "gorilla-utils/shared/Callbacks/MatchMakingCallbacks.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/ForceMode.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Camera.hpp"
#include "UnityEngine/Rigidbody.hpp"
#include "UnityEngine/Camera.hpp"
#include "UnityEngine/Collider.hpp"
#include "UnityEngine/CapsuleCollider.hpp"
#include "UnityEngine/SphereCollider.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/RaycastHit.hpp"
#include "UnityEngine/Physics.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/XR/InputDevice.hpp"
#include "monkecomputer/shared/GorillaUI.hpp"
#include "monkecomputer/shared/Register.hpp"
#include "custom-types/shared/register.hpp"
#include "config.hpp"
#include "HoverMonkeWatchView.hpp"
#include "gorilla-utils/shared/Callbacks/MatchMakingCallbacks.hpp"

ModInfo modInfo;

#define INFO(value...) getLogger().info(value)
#define ERROR(value...) getLogger().error(value)

using namespace UnityEngine;
using namespace UnityEngine::XR;
using namespace GorillaLocomotion;

Logger& getLogger()
{
    static Logger* logger = new Logger(modInfo, LoggerOptions(false, true));
    return *logger;
}

bool isRoom = false;
bool fist = false;
bool isFist = false;
float thrust = 0;
float carSpeed = 0;

void powerCheck() {
    if(config.power == 0) {
        thrust = 0.4;
    }
    if(config.power == 1) {
        thrust = 0.7;
    }
    if(config.power == 2) {
        thrust = 0.9;
    }
    if(config.power == 3) {
        thrust = 1.2;
    }
    if(config.power == 4) {
        thrust = 1.4;
    }
}
void carSpeedCheck() {
    if(config.carSpeed == 0) {
        carSpeed = 1.2;
    }
    if(config.carSpeed == 1) {
        carSpeed = 1.4;
    }
    if(config.carSpeed == 2) {
        carSpeed = 1.6;
    }
    if(config.carSpeed == 3) {
        carSpeed = 1.8;
    }
    if(config.carSpeed == 4) {
        carSpeed = 2.3;
    }
}

bool LStick = false;

#include "GlobalNamespace/GorillaTagManager.hpp"
#include "GlobalNamespace/OVRInput_Axis2D.hpp"
#include "GlobalNamespace/OVRInput_RawAxis2D.hpp"

MAKE_HOOK_MATCH(GorillaTagManager_Update, &GlobalNamespace::GorillaTagManager::Update, void, GlobalNamespace::GorillaTagManager* self) {

    using namespace GlobalNamespace;
    using namespace GorillaLocomotion;
    GorillaTagManager_Update(self);
    powerCheck();
    carSpeedCheck();

    Player* playerInstance = Player::get_Instance();
    if(playerInstance == nullptr) return;
    GameObject* go = playerInstance->get_gameObject();
    auto* player = go->GetComponent<GorillaLocomotion::Player*>();

    Rigidbody* playerPhysics = playerInstance->playerRigidBody;
    if(playerPhysics == nullptr) return;

    GameObject* playerGameObject = playerPhysics->get_gameObject();
    if(playerGameObject == nullptr) return;

    Transform* turnParent = playerGameObject->get_transform()->GetChild(0);

    Transform* mainCamera = turnParent->GetChild(0);

    Vector2 inputDir = OVRInput::Get(OVRInput::RawAxis2D::_get_LThumbstick(), OVRInput::Controller::LTouch);
       
    Vector3 velocityForward = mainCamera->get_forward() * (inputDir.y / 10);
    Vector3 velocitySideways = mainCamera->get_right() * (inputDir.x / 10);

    Vector3 newVelocityDirHover = (velocityForward + velocitySideways) * thrust;
    Vector3 newVelocityDirCar = (velocityForward + velocitySideways) * carSpeed;

    if(isRoom && config.enabled) {

        if(LStick) {
            playerPhysics->set_velocity(Vector3::get_zero());
            LStick = false;
            INFO("Brakes used");
        } 

        RaycastHit hit;

        UnityEngine::Transform* transform = playerGameObject->get_transform();

        float distance = 100.0f;

        int layermask = 0b1 << 9;

        if(Physics::Raycast(transform->get_position() + Vector3::get_down().get_normalized() * 0.1f, Vector3::get_down(), hit, distance, layermask) && !config.carMode) {
            float distance = 2.3f;

            float rayDistance = hit.get_distance();
                
            if(rayDistance < distance) {
                playerPhysics->AddForce(Vector3::get_up() * 500);
            }
            if(rayDistance > distance) {
                playerPhysics->AddForce(Vector3::get_down() * 500);
            }
            playerPhysics->set_velocity(playerPhysics->get_velocity() + newVelocityDirHover);
        }

        if(config.carMode) {
            distance = 0.75f;

            RaycastHit ray;

            auto down = Physics::Raycast(playerPhysics->get_transform()->get_position(), Vector3::get_down(), ray, 100.0f, layermask);

            Vector3 car_direction = Vector3::ProjectOnPlane(mainCamera->get_forward(), ray.get_normal());

            if(ray.get_distance() >= 1.8f) {
                playerPhysics->AddForce(Vector3::get_down() * 1500);
            }

            playerPhysics->set_velocity(playerPhysics->get_velocity() + newVelocityDirCar);
        }
    }
}

MAKE_HOOK_MATCH(Player_Awake, &GorillaLocomotion::Player::Awake, void, GorillaLocomotion::Player* self) {
    Player_Awake(self);

    GorillaUtils::MatchMakingCallbacks::onJoinedRoomEvent() += {[&]() {
        Il2CppObject* currentRoom = CRASH_UNLESS(il2cpp_utils::RunMethod("Photon.Pun", "PhotonNetwork", "get_CurrentRoom"));

        if (currentRoom)
        {
            isRoom = !CRASH_UNLESS(il2cpp_utils::RunMethod<bool>(currentRoom, "get_IsVisible"));
        } else isRoom = true;
    }
    };
}

extern "C" void setup(ModInfo& info)
{
    info.id = ID;
    info.version = VERSION;

    modInfo = info;
}

extern "C" void load()
{
    GorillaUI::Init();

    INSTALL_HOOK(getLogger(), Player_Awake);
    INSTALL_HOOK(getLogger(), GorillaTagManager_Update);

    custom_types::Register::AutoRegister();

    GorillaUI::Register::RegisterWatchView<HoverMonke::HoverMonkeWatchView*>("<b><i><color=#FF0000>Ho</color><color=#FF8700>v</color><color=#FFFB00>er</color><color=#0FFF00>M</color><color=#0036FF>on</color><color=#B600FF>k</color><color=#FF00B6>e</color></i></b>", VERSION);

    LoadConfig();
}