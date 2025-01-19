#include "../ByNameModding/BNM.hpp"

// To avoid writing
// BNM::Structures:: for each type
using namespace BNM::Structures::Mono; // monoString, monoArray, etc.
using namespace BNM::Operators; // Operators for methods, fields, and properties

// Variable to store the class
BNM::LoadClass ObjectClass{};
// Variable to store the property
BNM::Property<monoString *> ObjectName{};
// Variable to store the method
BNM::Method<monoString *> ObjectToString{};

// Variable to store the field
BNM::Field<void *> PlayerConfig{};

BNM::Field<monoString *> ConfigName{};
BNM::Field<int> ConfigHealth{};
BNM::Field<int> ConfigCoins{};

// Variable to store a pointer to the field
void **PlayerConfigPtr = nullptr;

void (*old_PlayerStart)(BNM::UnityEngine::Object *);
void PlayerStart(BNM::UnityEngine::Object *instance) {
    old_PlayerStart(instance); // Call the original code

    //! monoString - A C# string (string or System.String) in BNM

    // In this case, the field has a type, and the property will automatically call the Get method to get the name.
    // If using auto, the field will be a copy of ObjectName, and you will either need to call Get() or add () to ObjectName, e.g.:
    // (instance >> ObjectName)() // for >> and ->*
    // ObjectName[instance]() // for []
    monoString *playerObjectName =
        // Set instance in ObjectName via the ->* operator
        // Alternatives:
        // * instance >> ObjectName
        // * ObjectName[instance]
        instance->*ObjectName;

    // Get a pointer to the field's data
    PlayerConfigPtr = PlayerConfig[instance].GetPointer();

    // For fields, the ->* operator immediately returns a pointer to the field's data
    auto playerName = *PlayerConfigPtr->*ConfigName;

    // Log the player's object name
    BNM_LOG_INFO("Player object name: \"%s\"; Player name: \"%s\"",
                 // str() - Convert the string to std::string
                 playerObjectName->str().c_str(),
                 // -> - Iterator operator for data validation and warnings in debug mode
                 playerName->str().c_str());

    //! Call ObjectToString, arguments can be passed inside () like any method
    // Alternative way to call the method with an object:
    //     auto objectToStringResult = (instance >> ObjectToString)();

    auto objectToStringResult = ObjectToString[instance]();
    BNM_LOG_INFO("objectToStringResult: \"%s\"", objectToStringResult->str().c_str());

    //! Change the player's name

    // Strings can be created in two ways:
    // * monoString::Create - Does not go into Unity's garbage collector, so you'll need to delete it manually
    // * BNM::CreateMonoString - Goes into Unity's garbage collector, and Unity will delete it when needed

    // In this case, it's better to use BNM::CreateMonoString since this string will be in the game for some time, and manually deleting it would be inconvenient
    *playerName = BNM::CreateMonoString(OBFUSCATE_BNM("BNM_Player"));
}

void (*old_PlayerUpdate)(BNM::UnityEngine::Object *);
void PlayerUpdate(BNM::UnityEngine::Object *instance) {
    old_PlayerUpdate(instance); // Call the original code

    // Validate the pointer to the m_Config field
    if (PlayerConfigPtr == nullptr) return;

    // Set health to 99999 using the ->* operator
    //! *((*PlayerConfigPtr)->*ConfigHealth) = 99999;
    // Or using []
    ConfigHealth[*PlayerConfigPtr] = 99999;

    // Set coins to 99999
    ConfigCoins[*PlayerConfigPtr] = 99999;
}

// Here you can retrieve all the necessary information
void OnLoaded_Example_01() {
    using namespace BNM; // To avoid writing BNM:: in this method

    // Get the UnityEngine.Object class
    ObjectClass = LoadClass(OBFUSCATE_BNM("UnityEngine"), OBFUSCATE_BNM("Object"));

    // Get the UnityEngine.Object::ToString method with 0 parameters
    ObjectToString = ObjectClass.GetMethodByName(OBFUSCATE_BNM("ToString"), 0);

    // Get the UnityEngine.Object::name property
    ObjectName = ObjectClass.GetPropertyByName(OBFUSCATE_BNM("name"));

    /* Assume the game has the following class:
        public class Player : MonoBehaviour {
            private void Start();
            private void Update();
            private Config m_Config;

            public class Config {
                string Name;
                int Health;
                int Coins;
            }
        }
    */
    // Get the Player class
    auto PlayerClass = LoadClass(OBFUSCATE_BNM(""), OBFUSCATE_BNM("Player"));

    // Get the Player::Config class
    auto PlayerConfigClass = PlayerClass.GetInnerClass(OBFUSCATE_BNM("Config"));

    // Get the Update and Start methods of the Player class
    auto Update = PlayerClass.GetMethodByName(OBFUSCATE_BNM("Update"));
    auto Start = PlayerClass.GetMethodByName(OBFUSCATE_BNM("Start"));

    // Get the Player.m_Config field
    PlayerConfig = PlayerClass.GetFieldByName(OBFUSCATE_BNM("m_Config"));

    // Get the Player::Config fields
    ConfigName = PlayerConfigClass.GetFieldByName(OBFUSCATE_BNM("Name"));
    ConfigHealth = PlayerConfigClass.GetFieldByName(OBFUSCATE_BNM("Health"));
    ConfigCoins = PlayerConfigClass.GetFieldByName(OBFUSCATE_BNM("Coins"));

    // Hook the Update and Start methods

    // There are 3 methods for hooking:
    // * HOOK - Hooking through software for method replacement
    // * InvokeHook - Hooking that works for methods directly invoked by the engine (constructors (.ctor), events (Start, Update, etc.))
    // * VirtualHook - Hooking for virtual methods

    // For replacing Start and Update in this class, InvokeHook is the best choice
    InvokeHook(Update, PlayerUpdate, old_PlayerUpdate);
    InvokeHook(Start, PlayerStart, old_PlayerStart);
}

[[maybe_unused]] __attribute__((constructor))
void Example_01_main() {
    // Run immediately after il2cpp is loaded from its thread
    BNM::AddOnLoadedEvent(OnLoaded_Example_01);
}
