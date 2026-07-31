#include <list>
#include <vector>
#include <cstring>
#include <pthread.h>
#include <thread>
#include <string>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.hpp"
#include "Menu/Menu.hpp"
#include "Menu/Jni.hpp"
#include "Includes/Macros.h"
#include "dobby.h"

int scoreMul = 1, coinsMul = 1;

// Feature list definition for Java UI
jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;

    const char *features[] = {
            OBFUSCATE("Toggle_No death"),
            OBFUSCATE("Button_Start Invcibility (30 sec duration)"),
            OBFUSCATE("SeekBar_Score multiplier_1_100"),
            OBFUSCATE("SeekBar_Coins multiplier_1_1000"),
            OBFUSCATE("Category_Examples"), //Not counted
            OBFUSCATE("Toggle_The toggle"),
            OBFUSCATE("100_Toggle_True_The toggle 2"),
            OBFUSCATE("110_Toggle_The toggle 3"),
            OBFUSCATE("SeekBar_The slider_1_100"),
            OBFUSCATE("SeekBar_Kittymemory slider example_1_5"),
            OBFUSCATE("Spinner_The spinner_Items 1,Items 2,Items 3"),
            OBFUSCATE("Button_The button"),
            OBFUSCATE("ButtonLink_The button with link_https://www.youtube.com/"),
            OBFUSCATE("ButtonOnOff_The On/Off button"),
            OBFUSCATE("CheckBox_The Check Box"),
            OBFUSCATE("InputValue_Input number"),
            OBFUSCATE("InputValue_1000_Input number 2"),
            OBFUSCATE("1111_InputLValue_Input long number"),
            OBFUSCATE("InputLValue_1000000000000_Input long number 2"),
            OBFUSCATE("InputText_Input text"),
            OBFUSCATE("RadioButton_Radio buttons_OFF,Mod 1,Mod 2,Mod 3"),

            //Collapse sections
            OBFUSCATE("Collapse_Collapse 1"),
            OBFUSCATE("CollapseAdd_Toggle_The toggle"),
            OBFUSCATE("CollapseAdd_Toggle_The toggle"),
            OBFUSCATE("123_CollapseAdd_Toggle_The toggle"),
            OBFUSCATE("122_CollapseAdd_CheckBox_Check box"),
            OBFUSCATE("CollapseAdd_Button_The button"),

            OBFUSCATE("Collapse_Collapse 2_True"),
            OBFUSCATE("CollapseAdd_SeekBar_The slider_1_100"),
            OBFUSCATE("CollapseAdd_InputValue_Input number"),

            OBFUSCATE("RichTextView_This is text view, not fully HTML."
                      "<b>Bold</b> <i>italic</i> <u>underline</u>"
                      "<br />New line <font color='red'>Support colors</font>"
                      "<br/><big>bigger Text</big>"),
            OBFUSCATE("RichWebView_<html><head><style>body{color: white;}</style></head><body>"
                      "This is WebView, with REAL HTML support!"
                      "<div style=\"background-color: darkblue; text-align: center;\">Support CSS</div>"
                      "<marquee style=\"color: green; font-weight:bold;\" direction=\"left\" scrollamount=\"5\" behavior=\"scroll\">This is <u>scrollable</u> text</marquee>"
                      "</body></html>")
    };

    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)
            env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
                                env->NewStringUTF(""));

    for (int i = 0; i < Total_Feature; i++)
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));

    return (ret);
}

bool btnPressed = false;

// Target main library name
#define targetLibName OBFUSCATE("libil2cpp.so")

void Changes(JNIEnv *env, jclass clazz, jobject obj, jint featNum, jstring featName, jint value, jlong Lvalue, jboolean boolean, jstring text) {

    // Prevent patching if target library isn't loaded in memory yet
    if (!isLibraryLoaded(targetLibName)) {
        LOGI("Target library not loaded yet, skipping switch handler.");
        return;
    }

    switch (featNum) {
        case 0:
            // "No death" patch
            PATCH_SWITCH(targetLibName, "0x1079728", "C0 03 5F D6", boolean);
            break;
        case 1:
            btnPressed = true;
            break;
        case 2:
            scoreMul = value;
            break;
        case 3:
            coinsMul = value;
            break;
        case 4:
            if (boolean) {
                PATCH(targetLibName, "0x10709AC", "E0 5F 40 B2 C0 03 5F D6");
            } else {
                RESTORE(targetLibName, "0x10709AC");
            }
            break;
        case 5:
            INST(targetLibName, "0x235630", "AnyNameForDetect2", boolean);
            break;
        default:
            break;
    }
}

// Function pointers & Hooks
void (*StartInvcibility)(void *instance, float duration);

void (*old_Update)(void *instance);
void Update(void *instance) {
    if (instance != nullptr) {
        if (btnPressed) {
            StartInvcibility(instance, 30);
            btnPressed = false;
        }
    }
    return old_Update(instance);
}

install_hook_name(AddScore, void *, void *instance, int score) {
    return orig_AddScore(instance, score + scoreMul);
}

void (*old_AddCoins)(void *instance, int count);
void AddCoins(void *instance, int count) {
    return old_AddCoins(instance, count * coinsMul);
}

// Memory patching thread
void hack_thread() {
    // Wait until target library is loaded into process memory
    while (!isLibraryLoaded(targetLibName)) {
        sleep(1);
    }

#if defined(__aarch64__)
    StartInvcibility = (void (*)(void *, float)) getAbsoluteAddress(targetLibName, OBFUSCATE("0x107A3BC"));

    HOOK(targetLibName, "0x107A2FC", AddCoins, old_AddCoins);

    install_hook_AddScore(getAbsoluteAddress(targetLibName, OBFUSCATE("0x107A2E0")));

    HOOK(targetLibName, "0x1078C44", Update, old_Update);

    INST(targetLibName, "0x23558C", "AnyNameForDetect", true);
#elif defined(__arm__)
    // Code for armv7 if needed
#endif

    LOGI(OBFUSCATE("Done"));
}

// Library entry constructor
__attribute__((constructor))
void lib_main() {
    std::thread(hack_thread).detach();
}
