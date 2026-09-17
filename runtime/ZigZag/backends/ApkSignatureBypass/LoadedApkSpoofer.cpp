// LoadedApkSpoofer — runtime signature bypass via LoadedApk field spoofing.
// Hooks ActivityThread.mLoadedApk to replace ApplicationInfo.signingInfo at runtime.

#include "LoadedApkSpoofer.h"

#include <cstdio>
#include <cstring>
#include <dlfcn.h>

namespace omnibyte::runtime::backends {

// --- Lifecycle ---

bool LoadedApkSpoofer::initialize() {
    if (initialized_) return true;

    apiLevel_ = getApiLevel();
    if (apiLevel_ < 24) return false; // Need at least API 24 for reliable JNI field access.

    if (!resolveActivityThread()) return false;

    initialized_ = true;
    return true;
}

bool LoadedApkSpoofer::spoof(const std::string& packageName, const std::string& signatureData) {
    if (!initialized_ || signatureData.empty()) return false;

    JNIEnv* env = getEnv();
    if (!env) return false;

    // Get ActivityThread.currentActivityThread() → static method.
    jmethodID currentAT = env->GetStaticMethodID(
        activityThreadClass_, "currentActivityThread", "()Landroid/app/ActivityThread;");
    if (!currentAT) return false;

    jobject activityThread = env->CallStaticObjectMethod(activityThreadClass_, currentAT);
    if (!activityThread) return false;

    // Resolve mBoundApplication field (ActivityThread.AppBindData).
    jclass appBindDataClass = env->FindClass("android/app/ActivityThread$AppBindData");
    if (!appBindDataClass) {
        // Fallback: try ActivityThread$AppBindInfo (older naming).
        appBindDataClass = env->FindClass("android/app/ActivityThread$AppBindInfo");
    }
    if (!appBindDataClass) return false;

    jfieldID mBoundApp = env->GetFieldID(
        activityThreadClass_, "mBoundApplication",
        "Landroid/app/ActivityThread$AppBindData;");
    if (!mBoundApp) {
        mBoundApp = env->GetFieldID(
            activityThreadClass_, "mBoundApplication",
            "Landroid/app/ActivityThread$AppBindInfo;");
    }
    if (!mBoundApp) return false;

    jobject appBindData = env->GetObjectField(activityThread, mBoundApp);
    if (!appBindData) return false;

    // Get mApplicationInfo from AppBindData.
    jfieldID appInfoField = env->GetFieldID(appBindDataClass, "mApplicationInfo",
                                             "Landroid/content/pm/ApplicationInfo;");
    if (!appInfoField) return false;

    jobject appInfo = env->GetObjectField(appBindData, appInfoField);
    if (!appInfo) return false;

    jclass appInfoClass = env->GetObjectClass(appInfo);

    if (apiLevel_ >= 28) {
        // API 28+: Replace signingInfo field.
        jfieldID signingInfoField = env->GetFieldID(
            appInfoClass, "signingInfo", "Landroid/content/pm/SigningInfo;");
        if (!signingInfoField) return false;

        // Save original for restore.
        jobject origSigningInfo = env->GetObjectField(appInfo, signingInfoField);
        if (origSigningInfo) {
            // Store reference — not ideal but functional for spoof/restore pair.
            savedSigningInfo_.assign(reinterpret_cast<const char*>(&origSigningInfo), sizeof(origSigningInfo));
        }

        // Create spoofed SigningInfo via PackageManager.getPackageInfo().signingInfo.
        // We build it from the certificate data by creating a SignerInfo and wrapping it.
        jclass signerInfoClass = env->FindClass("android/content/pm/SigningInfo");
        if (!signerInfoClass) return false;

        // Build spoofed signingInfo from our certificate data.
        // Use reflection: SigningInfo(SignerInfo[])
        jclass signerDetailsClass = env->FindClass("android/content/pm/SignerDetails");
        if (signerDetailsClass) {
            jmethodID ctor = env->GetMethodID(signerDetailsClass, "<init>",
                "([Landroid/content/pm/Signature;)V");
            if (ctor) {
                // Create Signature objects from our DER data.
                jclass signatureClass = env->FindClass("android/content/pm/Signature");
                jmethodID sigCtor = env->GetMethodID(signatureClass, "<init>", "([B)V");

                jbyteArray sigBytes = env->NewByteArray(static_cast<jsize>(signatureData.size()));
                env->SetByteArrayRegion(sigBytes, 0, static_cast<jsize>(signatureData.size()),
                                        reinterpret_cast<const jbyte*>(signatureData.data()));

                jobject signature = env->NewObject(signatureClass, sigCtor, sigBytes);
                if (signature) {
                    jobjectArray sigArray = env->NewObjectArray(1, signatureClass, signature);
                    jobject signerDetails = env->NewObject(signerDetailsClass, ctor, sigArray);

                    // Wrap in SigningInfo.
                    jmethodID signingInfoCtor = env->GetMethodID(signerInfoClass, "<init>",
                        "([Landroid/content/pm/SignerDetails;)V");
                    if (signingInfoCtor) {
                        jobjectArray detailsArray = env->NewObjectArray(
                            1, signerDetailsClass, signerDetails);
                        jobject spoofedSigningInfo = env->NewObject(
                            signerInfoClass, signingInfoCtor, detailsArray);
                        if (spoofedSigningInfo) {
                            env->SetObjectField(appInfo, signingInfoField, spoofedSigningInfo);
                            hooked_ = true;
                            targetPackage_ = packageName;
                        }
                    }
                }
            }
        }
    } else {
        // API < 28: Replace signatures field (Signature[]).
        jfieldID sigField = env->GetFieldID(
            appInfoClass, "signatures", "[Landroid/content/pm/Signature;");
        if (!sigField) return false;

        jclass signatureClass = env->FindClass("android/content/pm/Signature");
        jmethodID sigCtor = env->GetMethodID(signatureClass, "<init>", "([B)V");

        jbyteArray sigBytes = env->NewByteArray(static_cast<jsize>(signatureData.size()));
        env->SetByteArrayRegion(sigBytes, 0, static_cast<jsize>(signatureData.size()),
                                reinterpret_cast<const jbyte*>(signatureData.data()));

        jobject signature = env->NewObject(signatureClass, sigCtor, sigBytes);
        jobjectArray sigArray = env->NewObjectArray(1, signatureClass, signature);

        env->SetObjectField(appInfo, sigField, sigArray);
        hooked_ = true;
        targetPackage_ = packageName;
    }

    return hooked_;
}

bool LoadedApkSpoofer::restore(const std::string& packageName) {
    if (!hooked_ || targetPackage_ != packageName) return false;

    JNIEnv* env = getEnv();
    if (!env) return false;

    // Re-acquire ActivityThread and ApplicationInfo, then clear spoofed fields.
    jmethodID currentAT = env->GetStaticMethodID(
        activityThreadClass_, "currentActivityThread", "()Landroid/app/ActivityThread;");
    if (!currentAT) return false;

    jobject activityThread = env->CallStaticObjectMethod(activityThreadClass_, currentAT);
    if (!activityThread) return false;

    jclass appBindDataClass = env->FindClass("android/app/ActivityThread$AppBindData");
    if (!appBindDataClass) appBindDataClass = env->FindClass("android/app/ActivityThread$AppBindInfo");
    if (!appBindDataClass) return false;

    jfieldID mBoundApp = env->GetFieldID(
        activityThreadClass_, "mBoundApplication",
        "Landroid/app/ActivityThread$AppBindData;");
    if (!mBoundApp) {
        mBoundApp = env->GetFieldID(
            activityThreadClass_, "mBoundApplication",
            "Landroid/app/ActivityThread$AppBindInfo;");
    }
    if (!mBoundApp) return false;

    jobject appBindData = env->GetObjectField(activityThread, mBoundApp);
    if (!appBindData) return false;

    jfieldID appInfoField = env->GetFieldID(appBindDataClass, "mApplicationInfo",
                                             "Landroid/content/pm/ApplicationInfo;");
    if (!appInfoField) return false;

    jobject appInfo = env->GetObjectField(appBindData, appInfoField);
    if (!appInfo) return false;

    jclass appInfoClass = env->GetObjectClass(appInfo);

    if (apiLevel_ >= 28) {
        jfieldID signingInfoField = env->GetFieldID(
            appInfoClass, "signingInfo", "Landroid/content/pm/SigningInfo;");
        if (signingInfoField && !savedSigningInfo_.empty()) {
            jobject* savedPtr = reinterpret_cast<jobject*>(
                const_cast<char*>(savedSigningInfo_.data()));
            env->SetObjectField(appInfo, signingInfoField, *savedPtr);
        }
    } else {
        jfieldID sigField = env->GetFieldID(
            appInfoClass, "signatures", "[Landroid/content/pm/Signature;");
        if (sigField) {
            env->SetObjectField(appInfo, sigField, nullptr);
        }
    }

    hooked_ = false;
    savedSigningInfo_.clear();
    targetPackage_.clear();
    return true;
}

bool LoadedApkSpoofer::isSupported() const {
    return apiLevel_ >= 28;
}

// --- Internal ---

bool LoadedApkSpoofer::resolveActivityThread() {
    // Use JNI to find ActivityThread class.
    JNIEnv* env = getEnv();
    if (!env) return false;

    activityThreadClass_ = env->FindClass("android/app/ActivityThread");
    if (!activityThreadClass_) return false;

    return true;
}

bool LoadedApkSpoofer::resolveApplicationInfoFields() {
    JNIEnv* env = getEnv();
    if (!env) return false;

    jclass appInfoClass = env->FindClass("android/content/pm/ApplicationInfo");
    if (!appInfoClass) return false;

    if (apiLevel_ >= 28) {
        signingInfoField_ = env->GetFieldID(
            appInfoClass, "signingInfo", "Landroid/content/pm/SigningInfo;");
    } else {
        signingInfoField_ = env->GetFieldID(
            appInfoClass, "signatures", "[Landroid/content/pm/Signature;");
    }

    return signingInfoField_ != nullptr;
}

int LoadedApkSpoofer::getApiLevel() const {
    // Try __system_property_get first (no process spawn).
    char sdkVer[92] = {};
    // Android SDK version is in "ro.build.version.sdk".
    // __system_property_get is in libc — we link against it.
    typedef int (*PropGetFn)(const char*, char*);
    auto propGet = reinterpret_cast<PropGetFn>(dlsym(RTLD_DEFAULT, "__system_property_get"));
    if (propGet) {
        propGet("ro.build.version.sdk", sdkVer);
    } else {
        // Fallback to popen.
        FILE* pipe = popen("getprop ro.build.version.sdk 2>/dev/null", "r");
        if (!pipe) return 0;
        fgets(sdkVer, sizeof(sdkVer), pipe);
        pclose(pipe);
    }

    int level = 0;
    for (char* p = sdkVer; *p >= '0' && *p <= '9'; ++p) {
        level = level * 10 + (*p - '0');
    }
    return level;
}

JNIEnv* LoadedApkSpoofer::getEnv() {
    if (!jvm_) {
        // Get JavaVM via JNI_OnLoad pattern — try dladdr on a known JNI function
        // or use JNI_GetCreatedJavaVMs.
        typedef jint (*GetCreatedVMsFn)(JavaVM**, jsize, jsize*);
        auto getVMs = reinterpret_cast<GetCreatedVMsFn>(
            dlsym(RTLD_DEFAULT, "JNI_GetCreatedJavaVMs"));
        if (getVMs) {
            jsize count = 0;
            getVMs(&jvm_, 1, &count);
        }
    }
    if (!jvm_) return nullptr;

    JNIEnv* env = nullptr;
    jint status = jvm_->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    if (status == JNI_EDETACHED) {
        jvm_->AttachCurrentThread(&env, nullptr);
    }
    return env;
}

} // namespace omnibyte::runtime::backends
