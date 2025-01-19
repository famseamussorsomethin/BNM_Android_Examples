#include "../ByNameModding/BNM.hpp"

// To avoid writing
// BNM::Structures:: for each type
using namespace BNM::Structures::Unity; // Vector3, Vector2, etc.
using namespace BNM::Structures::Mono; // monoString, monoArray, etc.

void MonoArray() {
    // Type[] - an array of some objects
    monoArray<int> *array = nullptr;

    //! It can be created in two ways:
    // * monoArray<Type>::Create(size or std::vector<Type>) - does not go into the garbage collector
    // * LoadClass().NewArray<Type>(size) - goes into the garbage collector
    // Create it using the first method
    array = monoArray<int>::Create(10);

    //! Data can be accessed using:
    auto dataPtr = array->GetData(); // Pointer to a C array
    // or
    auto dataVec = array->ToVector(); // std::vector<int>
    // or
    auto firstData = array->At(0); // First element of the array

    //! Delete the array to free up memory
    //! Required ONLY when created via monoArray<Type>::Create!
    array->Destroy();
}

void MonoList() {
    // System.Collections.Generic.List<Type> - a list of some objects
    monoList<int> *list = nullptr;

    //! It can only be created through the class, e.g.
    // LoadClass().NewList<Type>(size) - goes into the garbage collector

    //! To avoid searching for the System.Int32 class (the class for int values in C#),
    //! you can use BNM::GetType<Type>().
    //! BNM::GetType supports only basic types.
    auto intClass = BNM::GetType<int>().ToLC();

    list = intClass.NewList<int>();

    //! Data can be accessed using:
    auto dataPtr = list->GetData(); // Pointer to a C array
    // or
    auto dataVec = list->ToVector(); // std::vector<int>
    // or
    auto firstData = list->At(0); // First element of the list
}

void MonoDictionary() {
    // System.Collections.Generic.Dictionary<KeyType, ValueType> - a dictionary
    monoDictionary<int, int> *dictionary = nullptr;
    //! Cannot be created through BNM

    //! Data can be accessed using:
    auto keys = dictionary->GetKeys(); // std::vector<KeyType>
    // or
    auto values = dictionary->GetValues(); // std::vector<ValueType>
    // or
    auto map = dictionary->ToMap(); // std::map<KeyType, ValueType>
    // or
    int value = 0;
    if (dictionary->TryGet(1, &value))
        ; // Value found
}

void OnLoaded_Example_02() {
    using namespace BNM;

    //! Unity structures

    // Mathematical structures
    // Mathematical operations can be performed on these structures similar to those in Unity
    Vector2 vector2;
    Vector3 vector3;
    Vector4 vector4;
    Matrix3x3 matrix3x3;
    Matrix4x4 matrix4x4;
    Quaternion quaternion;

    // Structures for Raycast
    Ray ray;
    RaycastHit raycastHit;

    //! Mono structures

    //! System.String, described in more detail in Example 01
    monoString *string;

    //! monoArray is described in the method
    MonoArray();

    //! monoList is described in the method
    MonoList();

    //! monoDictionary is described in the method
    // MonoDictionary();
}

[[maybe_unused]] __attribute__((constructor))
void Example_02_main() {
    // Run immediately after il2cpp is loaded from its thread
    BNM::AddOnLoadedEvent(OnLoaded_Example_02);
}
