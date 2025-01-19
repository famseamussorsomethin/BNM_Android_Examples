#include "../ByNameModding/BNM.hpp"
using namespace BNM::Structures::Unity; // Vector3, Vector2, etc.
using namespace BNM::Structures::Mono; // monoString, monoArray, etc.

/* getExternMethod: example of changing the field of view
 * Code taken from:
 * Il2CppResolver
 * https://github.com/kp7742/IL2CppResolver/blob/master/Android/test/src/demo.cpp
 * IL2CppResolver, made by MJx0, doesn't work on all Unity versions
 * getExternMethod ONLY works with external (extern) methods
 */
void set_fov(float value) {
    int (*Screen$$get_height)();
    int (*Screen$$get_width)();
    InitResolveFunc(Screen$$get_height, OBFUSCATE_BNM("UnityEngine.Screen::get_height")); // #define InitResolveFunc(x, y)
    InitResolveFunc(Screen$$get_width, OBFUSCATE_BNM("UnityEngine.Screen::get_width"));
    if (Screen$$get_height && Screen$$get_width) BNM_LOG_INFO(OBFUSCATE_BNM("[set_fov] %dx%d"), Screen$$get_height(), Screen$$get_width());

    uintptr_t (*Camera$$get_main)(); // Can use void *
    float (*Camera$$get_fieldofview)(uintptr_t);
    void (*Camera$$set_fieldofview)(uintptr_t, float);

    InitResolveFunc(Camera$$get_main, OBFUSCATE_BNM("UnityEngine.Camera::get_main"));
    InitResolveFunc(Camera$$set_fieldofview, OBFUSCATE_BNM("UnityEngine.Camera::set_fieldOfView"));
    InitResolveFunc(Camera$$get_fieldofview, OBFUSCATE_BNM("UnityEngine.Camera::get_fieldOfView"));

    if (Camera$$get_main && Camera$$get_fieldofview && Camera$$set_fieldofview) {
        uintptr_t mainCamera = Camera$$get_main();
        if (BNM::IsUnityObjectAlive(mainCamera)) {
            float oldFOV = Camera$$get_fieldofview(mainCamera);
            Camera$$set_fieldofview(mainCamera, value);
            float newFOV = Camera$$get_fieldofview(mainCamera);
            BNM_LOG_INFO(OBFUSCATE_BNM("[set_fov] Camera address: %p  |  old field of view: %.2f  |  new field of view: %.2f"), (void *)mainCamera, oldFOV, newFOV);
        } else {
            BNM_LOG_ERR(OBFUSCATE_BNM("[set_fov] mainCamera is currently dead"));
        }
    }
}

// Additional variables for customization
bool setName;
bool parseDict = true;
BNM::LoadClass FPSController;

// Example of finding methods and other elements
BNM::Method<void *> get_Transform;
BNM::Property<Vector3> transformPosition;
BNM::Method<void> set_position_Injected;
void *myPlayer;
BNM::Field<void *> LocalPlayer;
BNM::Field<monoString *> PlayerName;
BNM::Field<monoDictionary<monoString *, void *> *> Players;

void (*old_Update)(...);
void Update(void *instance) {
    old_Update(instance);

    // LocalPlayer() can be used to get the object
    myPlayer = LocalPlayer;

    if (BNM::IsSameUnityObject(myPlayer, instance)) { 
        // Perform some actions
    }

    if (BNM::IsA(myPlayer, FPSController)) {
        // Perform some actions
    }

    if (BNM::IsUnityObjectAlive(myPlayer)) {
        set_fov(180.f);

        void *myPlayer_Transform = get_Transform(myPlayer);
        transformPosition[myPlayer_Transform] = Vector3(0, 0, 0);
        Vector3 pos(0, 0, 0);
        set_position_Injected(myPlayer_Transform, &pos); 

        if (!setName) {
            PlayerName[myPlayer]; 
            BNM_LOG_INFO(OBFUSCATE_BNM("Old name myPlayer: %s"), PlayerName()->str().c_str());

            PlayerName = BNM::CreateMonoString(OBFUSCATE_BNM("ByNameModding_Player"));

            BNM_LOG_INFO(OBFUSCATE_BNM("New name myPlayer: %s"), PlayerName()->str().c_str());
            setName = true;
        }

        if (parseDict) {
            auto map = Players()->ToMap();
            for (auto &it : map) {
                if (it.first) {
                    BNM_LOG_INFO(OBFUSCATE_BNM("Found player: [%s, %p]"), it.first->str().c_str(), it.second);
                }
            }
            parseDict = false;
        }
    }
}

// Example of adding a new GameObject and methods
[[maybe_unused]] void *MyGameObject = nullptr;
BNM::LoadClass GameObject;
BNM::Method<void> AddComponent;
BNM::Method<void> DontDestroyOnLoad;

void *Example_NewGameObject() {
    void *new_GameObject = GameObject.CreateNewObject();
#if !BNM_DISABLE_NEW_CLASSES
    AddComponent[new_GameObject](BNM::BNM_ExampleClass::BNMClass.type);
#endif
    DontDestroyOnLoad(new_GameObject);
    return new_GameObject;
}

void hack_thread() {
    using namespace BNM;

    GameObject = LoadClass(OBFUSCATE_BNM("UnityEngine"), OBFUSCATE_BNM("GameObject"));
    AddComponent = GameObject.GetMethodByName(OBFUSCATE_BNM("AddComponent"), 1);
    DontDestroyOnLoad = LoadClass(OBFUSCATE_BNM("UnityEngine"), OBFUSCATE_BNM("Object")).GetMethodByName(OBFUSCATE_BNM("DontDestroyOnLoad"));
}

// JNI Integration
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, [[maybe_unused]] void *reserved) {
    JNIEnv *env;
    vm->GetEnv((void **) &env, JNI_VERSION_1_6);

    BNM::TryForceLoadIl2CppByPath(env);
    return JNI_VERSION_1_6;
}

#include <thread>
[[maybe_unused]] __attribute__((constructor))
void lib_main() {
    BNM::AddOnLoadedEvent(hack_thread);
}
