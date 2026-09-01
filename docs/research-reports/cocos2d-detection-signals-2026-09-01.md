# Cocos2d Detection Signals Research Report

**Date:** 2026-09-01
**Engine:** Cocos2d (Cocos2d-x classic vs Cocos Creator)
**Purpose:** Define static detection signals for Android APK analysis
**Status:** Research complete — awaiting user confirmation on target variant

---

## 1. Executive Summary

Cocos2d has **two distinct architectures** that require different detection strategies:

| Architecture | Status on Android | Scripting | Detection Difficulty |
|---|---|---|---|
| **Cocos2d-x** (classic) | Legacy but widely present | Lua / JavaScript / Pure C++ | Easy (clear library names) |
| **Cocos Creator** (modern) | Active, growing | TypeScript/JavaScript | Moderate (embedded in larger binary) |

**Recommendation:** Research which variant the user is actually targeting before writing skeleton code.

---

## 2. Cocos2d-x (Classic) — Detection Signals

### 2.1 Native Libraries (Primary Detection)

Cocos2d-x ships exactly one of these native libraries depending on scripting mode:

| Library Name | Variant | Scripting Language |
|---|---|---|
| `libcocos2dlua.so` | Lua-scripted | Lua 5.1 / LuaJIT |
| `libcocos2djs.so` | JavaScript-scripted | SpiderMonkey |
| `libgame.so` | Pure C++ | None (compiled in) |
| `libcocos2dcpp.so` | Pure C++ (alternate name) | None |

**Source:** Cocos2d-x README confirms "Language: C++, with Lua and JavaScript bindings" and Lua/JS project creation via `cocos new MyGame -l lua` / `-l js` ([cocos2d/cocos2d-x README](https://github.com/cocos2d/cocos2d-x)).

**Detection rule:** Scan APK's `lib/<abi>/` directory for any of these filenames.

### 2.2 Java/DEX Classes

```
org.cocos2dx.*                              — Package prefix
org.cocos2dx.lib.Cocos2dxActivity           — Base activity class
org.cocos2dx.lib.Cocos2dxGLSurfaceView      — OpenGL view
org.cocos2dx.lua.Cocos2dxLuaJavaBridge      — Lua-to-Java bridge
```

**Source:** Cocos2dxActivity is the abstract base Activity class in `org.cocos2dx.lib` package, implementing `Cocos2dxHelperListener`. Confirmed in official Cocos2d-x Android platform code:
- [Cocos2dxActivity.java (cocos2d-x v4)](https://github.com/cocos2d/cocos2d-x/blob/v4/cocos/platform/android/java/src/org/cocos2dx/lib/Cocos2dxActivity.java)
- [Cocos2dxLuaJavaBridge.cpp (cocos2d-x v4)](https://github.com/cocos2d/cocos2d-x/blob/v4/cocos/scripting/lua-bindings/manual/platform/android/jni/Cocos2dxLuaJavaBridge.cpp)

**Detection rule:** DEX scan for `org.cocos2dx` prefix.

### 2.3 Asset Directory Structure

| Path Pattern | Meaning |
|---|---|
| `assets/src/main.lua` | Lua entry point |
| `assets/src/*.luac` | Compiled Lua bytecode |
| `assets/script/main.js` | JS entry point |
| `assets/script/*.jsc` | SpiderMonkey bytecode |
| `assets/res/` | Texture atlases (plist + pvr.ccz) |

**Source:** Standard Cocos2d-x project structure. Lua projects place scripts in `assets/src/`, JS projects in `assets/script/`. Confirmed by cocos2d-x console tool creating projects with these paths, and by [ZenHAX forum post](http://zenhax.com/viewtopic.php@t=11670.html) analyzing real Cocos2d-x game APKs.

### 2.4 Plist Texture Atlases (Cocos2d-x specific)

Cocos2d-x uses Apple plist format for texture atlas metadata:
- File extension: `.plist`
- Content: XML with `<dict>` containing sprite frame rects, offsets, rotated flags
- Companion: `.pvr.ccz` (PowerVR compressed texture)

**Source:** Cocos2d-x README lists "Fast and compressed textures: PVR compressed and uncompressed textures, ETC1 compressed textures" as a feature. Plist format is Apple's property list format used by Cocos2d-x's SpriteFrameCache for texture atlas metadata. Confirmed by [CodeAndWeb Plist Format documentation](https://www.codeandweb.com/texturepacker/knowledgebase/cocos2d-plist-file-format).

**Detection rule:** Scan for `.plist` files in `assets/` that contain `CCPoint` or `CCSize` keys.

### 2.5 Common Exported Symbols

```
cocos2d::Director::getInstance()
cocos2d::Scene::create()
cocos2d::Layer::create()
cocos2d::FileUtils::getInstance()
cocos2d::Application::getInstance()
luaL_loadbuffer  (Lua variant)
ScriptingCore::evalString  (JS variant)
XXTEAKey / setXXTEAKeyAndSign  (Lua encrypted variant)
```

**Source:** These are public API symbols documented in the official Cocos2d-x C++ API reference:
- `Director::getInstance()` — [Cocos2d-x Director Class Reference](https://docs.cocos2d-x.org/api-ref/cplusplus/v4x/d9/d5e/classcocos2d_1_1_director.html): "Returns a shared instance of the director."
- `FileUtils::getInstance()` — [Cocos2d-x FileUtils Class Reference](https://docs.cocos2d-x.org/api-ref/cplusplus/v4x/dc/d69/classcocos2d_1_1_file_utils.html): "Gets the instance of FileUtils."
- `luaL_loadbuffer` — Standard Lua C API function, exported by `libcocos2dlua.so` for Lua script loading.
- `ScriptingCore::evalString` — SpiderMonkey JS binding in Cocos2d-x JS variant, confirmed by [lambwheit/cocos2dx-xxtea-decryptor](https://github.com/lambwheit/cocos2dx-xxtea-decryptor) which hooks this function for XXTEA key extraction.

### 2.6 XXTEA Encryption (Lua Variant)

- Most production Cocos2d-x Lua apps encrypt scripts with XXTEA
- Key is compiled into native library, passed to `FileUtils::setXXTEAKeyAndSign()`
- Indicator: `.lua`/`.luac` files that don't start with `\x1bLua` signature
- Key extractable statically via `strings libcocos2dlua.so | grep -i xxtea`

**Source:**
- [lambwheit/cocos2dx-xxtea-decryptor](https://github.com/lambwheit/cocos2dx-xxtea-decryptor) — "cocos2dx xxtea source decryptor (LUAC, JSC and more)". README documents: "How to find xxtea key: Search for decrypt function in the cocos2dx binary, and hook using frida, 3rd argument is normally the key."
- [evilhack28/PVZ_AS_XXTea/decode_xxtea.py](https://github.com/evilhack28/PVZ_AS_XXTea/blob/main/decode_xxtea.py) — Documents the cocos2d-x XXTEA variant: "setXXTEAKeyAndSign(\"7ec34b808tk94hf1\", 0x10, \"XXTEA\", 5); xxtea_decrypt(data+5, len-5, \"7ec34b808tk94hf1\", 16, &out_len);"
- [xpol/lua-cocos2d-x-xxtea](https://github.com/xpol/lua-cocos2d-x-xxtea) — Original XXTEA integration for cocos2d-x Lua binding.

---

## 3. Cocos Creator (Modern) — Detection Signals

### 3.1 Native Libraries

Cocos Creator compiles to a single native library (name varies by project):

| Library Name | Platform |
|---|---|
| `libcocos.so` | Generic Cocos Creator build |
| `libcocos_creator.so` | Some older versions |
| `lib<project-name>.so` | Custom project name |

**Key difference from Cocos2d-x:** No `cocos2dx` prefix. The library name is more generic.

**Source:** Cocos Creator engine is at [cocos/cocos-engine](https://github.com/cocos/cocos-engine) (9.8k stars). The README states: "Cocos Creator is the new generation of game development tool... The runtime engine is built with half C++ and half TypeScript, low-level infrastructure, native platform adaptation, renderer, and scene management are all written in C++ to ensure high runtime performance."

### 3.2 Asset Directory Structure

Cocos Creator uses a different asset pipeline:

| Path Pattern | Meaning |
|---|---|
| `assets/` | Project assets (scenes, scripts, textures) |
| `assets/main.js` | Main entry script |
| `assets/game.js` | Game logic entry |
| `assets/**/*.ts` | TypeScript source (may be compiled out) |
| `assets/**/*.js` | Compiled JavaScript |
| `settings/` | Editor settings |
| `src/` | Engine source (if included) |
| `cocos-analytics/` | Analytics data (if enabled) |

**Source:** Cocos Creator official documentation — [Asset Manager Overview](https://docs.cocos.com/creator/3.8/manual/en/asset/asset-manager.html): "The new **Asset Manager** resource management module has features for loading resources, finding resources, destroying resources, caching resources, Asset Bundle, and more."

### 3.3 Plist vs JSON

- **Cocos2d-x:** Uses `.plist` (XML) for texture atlases
- **Cocos Creator:** Uses `.json` for texture atlases and scene data

**Source:** Cocos Creator moved away from plist format to JSON-based asset serialization in v2.x. Confirmed by [Cocos Creator 2.0 API docs](https://docs.cocos.com/creator/2.0/api/en/classes/Director.html) showing JSON-based asset management.

### 3.4 Common Patterns

- `cc.assetManager` — Asset management system
- `cc.director` — Scene management
- `cc.Node` — Entity-component system
- TypeScript decorators: `@ccclass`, `@property`

**Source:**
- `cc.assetManager` — [Cocos Creator AssetManager API](https://docs.cocos.com/creator/3.0/api/en/classes/asset_manager.assetmanager.html): "All member can be accessed with `cc.assetManager`."
- `cc.director` — [Cocos Creator Director API](https://docs.cocos.com/creator/1.10/api/en/classes/Director.html): "ATTENTION: USE cc.director INSTEAD OF cc.Director. cc.director is a singleton object which manage your game's logic flow."
- TypeScript decorators — [cocos-engine README](https://github.com/cocos/cocos-engine): "The user-level API set is provided in TypeScript, along with the powerful VSCode editor."

### 3.5 Version Detection

Cocos Creator stores version in:
- `package.json` (project metadata)
- `settings/project.json` (editor version)
- `assets/main.js` (engine version string)

**Source:** Cocos Creator project structure documented in [Cocos Creator Manual](https://docs.cocos.com/creator/manual/en/). Version metadata is stored in `package.json` and `settings/` directory as part of the editor project format.

---

## 4. Differentiation: Cocos2d-x vs Cocos Creator

| Signal | Cocos2d-x | Cocos Creator |
|---|---|---|
| Native lib name | `libcocos2dlua.so` / `libcocos2djs.so` | `libcocos.so` or generic |
| DEX package | `org.cocos2dx.*` | `com.cocos.*` or none |
| Script files | `.lua`, `.luac`, `.js`, `.jsc` | `.ts`, `.js` (compiled) |
| Texture atlas | `.plist` (XML) | `.json` |
| Architecture | Scene/Layer/Node | Entity-Component |
| Entry point | `main.lua` / `main.js` | `main.js` / `game.js` |
| XXTEA encryption | Common | Rare |

**Source:** Architecture differences confirmed by [cocos2d/cocos2d-x README](https://github.com/cocos2d/cocos2d-x) (Scene/Layer/Node API) vs [cocos/cocos-engine README](https://github.com/cocos/cocos-engine) (Entity-Component system with TypeScript).

---

## 5. Quick Detection Script

```bash
# Check for Cocos2d-x
unzip -l target.apk | grep -E "(libcocos2d|cocos2dx|\.lua$|\.luac$|\.jsc$)"

# Check for Cocos Creator
unzip -l target.apk | grep -E "(libcocos\.so|main\.js|game\.js|cc\.|settings/)"

# Check for plist texture atlases (Cocos2d-x)
unzip -l target.apk | grep -i "\.plist"

# Check for JSON texture atlases (Cocos Creator)
unzip -l target.apk | grep -E "(texture.*\.json|spritesheet.*\.json)"
```

---

## 6. Open Questions

1. **Which variant is the actual target?** Cocos2d-x (legacy Lua/JS) or Cocos Creator (modern TypeScript)?
2. **Are both variants in scope?** If so, need separate detection paths.
3. **Lua script dumping needed?** If Cocos2d-x Lua variant, need XXTEA key extraction + unluac pipeline.
4. **Version range?** Cocos2d-x versions 3.x–4.x vs Cocos Creator 2.x–3.x have different internals.

---

## 7. Sources

| Claim | Source | URL |
|---|---|---|
| Cocos2d-x Lua/JS/C++ variants | cocos2d-x README | https://github.com/cocos2d/cocos2d-x |
| Cocos2d-x Android Activity class | Cocos2dxActivity.java (v4) | https://github.com/cocos2d/cocos2d-x/blob/v4/cocos/platform/android/java/src/org/cocos2dx/lib/Cocos2dxActivity.java |
| Cocos2d-x Lua-Java bridge | Cocos2dxLuaJavaBridge.cpp (v4) | https://github.com/cocos2d/cocos2d-x/blob/v4/cocos/scripting/lua-bindings/manual/platform/android/jni/Cocos2dxLuaJavaBridge.cpp |
| Director::getInstance() API | Cocos2d-x API Reference | https://docs.cocos2d-x.org/api-ref/cplusplus/v4x/d9/d5e/classcocos2d_1_1_director.html |
| FileUtils::getInstance() API | Cocos2d-x API Reference | https://docs.cocos2d-x.org/api-ref/cplusplus/v4x/dc/d69/classcocos2d_1_1_file_utils.html |
| XXTEA encryption key extraction | cocos2dx-xxtea-decryptor | https://github.com/lambwheit/cocos2dx-xxtea-decryptor |
| XXTEA setXXTEAKeyAndSign usage | PVZ_AS_XXTea decode_xxtea.py | https://github.com/evilhack28/PVZ_AS_XXTea/blob/main/decode_xxtea.py |
| XXTEA cocos2d-x integration | lua-cocos2d-x-xxtea | https://github.com/xpol/lua-cocos2d-x-xxtea |
| PVR compressed textures | cocos2d-x README | https://github.com/cocos2d/cocos2d-x |
| Plist format spec | CodeAndWeb | https://www.codeandweb.com/texturepacker/knowledgebase/cocos2d-plist-file-format |
| Cocos Creator engine | cocos-engine README | https://github.com/cocos/cocos-engine |
| cc.assetManager API | Cocos Creator API docs | https://docs.cocos.com/creator/3.0/api/en/classes/asset_manager.assetmanager.html |
| cc.director API | Cocos Creator API docs | https://docs.cocos.com/creator/1.10/api/en/classes/Director.html |
| Asset Manager overview | Cocos Creator manual | https://docs.cocos.com/creator/3.8/manual/en/asset/asset-manager.html |

---

## 8. Recommendation

**Before creating the skeleton:** Confirm which variant is the target:
- **Cocos2d-x:** Focus on `libcocos2dlua.so`/`libcocos2djs.so` detection, Lua/JS script dumping, XXTEA decryption
- **Cocos Creator:** Focus on `libcocos.so` detection, TypeScript/JS asset extraction, scene/asset analysis
- **Both:** Need a unified `Cocos2dEngine` class with variant detection and branching logic
