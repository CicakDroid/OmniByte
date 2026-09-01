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

**Detection rule:** Scan APK's `lib/<abi>/` directory for any of these filenames.

### 2.2 Java/DEX Classes

```
org.cocos2dx.*                              — Package prefix
org.cocos2dx.lib.Cocos2dxActivity           — Base activity class
org.cocos2dx.lib.Cocos2dxGLSurfaceView      — OpenGL view
org.cocos2dx.lua.Cocos2dxLuaJavaBridge      — Lua-to-Java bridge
```

**Detection rule:** DEX scan for `org.cocos2dx` prefix.

### 2.3 Asset Directory Structure

| Path Pattern | Meaning |
|---|---|
| `assets/src/main.lua` | Lua entry point |
| `assets/src/*.luac` | Compiled Lua bytecode |
| `assets/script/main.js` | JS entry point |
| `assets/script/*.jsc` | SpiderMonkey bytecode |
| `assets/res/` | Texture atlases (plist + pvr.ccz) |

### 2.4 Plist Texture Atlases (Cocos2d-x specific)

Cocos2d-x uses Apple plist format for texture atlas metadata:
- File extension: `.plist`
- Content: XML with `<dict>` containing sprite frame rects, offsets, rotated flags
- Companion: `.pvr.ccz` (PowerVR compressed texture)

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

### 2.6 XXTEA Encryption (Lua Variant)

- Most production Cocos2d-x Lua apps encrypt scripts with XXTEA
- Key is compiled into native library, passed to `FileUtils::setXXTEAKeyAndSign()`
- Indicator: `.lua`/`.luac` files that don't start with `\x1bLua` signature
- Key extractable statically via `strings libcocos2dlua.so | grep -i xxtea`

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

### 3.3 Plist vs JSON

- **Cocos2d-x:** Uses `.plist` (XML) for texture atlases
- **Cocos Creator:** Uses `.json` for texture atlases and scene data

### 3.4 Common Patterns

- `cc.assetManager` — Asset management system
- `cc.director` — Scene management
- `cc.Node` — Entity-component system
- TypeScript decorators: `@ccclass`, `@property`

### 3.5 Version Detection

Cocos Creator stores version in:
- `package.json` (project metadata)
- `settings/project.json` (editor version)
- `assets/main.js` (engine version string)

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

- https://github.com/cocos2d/cocos2d-x — Classic Cocos2d-x source
- https://github.com/cocos/cocos-engine — Cocos Creator engine source (9.8k stars)
- https://github.com/Anthropicdaddy/Shark/blob/main/docs/reversing/frameworks/cocos2d-x.md — Reversing guide
- https://www.codeandweb.com/texturepacker/knowledgebase/cocos2d-plist-file-format — Plist format spec

---

## 8. Recommendation

**Before creating the skeleton:** Confirm which variant is the target:
- **Cocos2d-x:** Focus on `libcocos2dlua.so`/`libcocos2djs.so` detection, Lua/JS script dumping, XXTEA decryption
- **Cocos Creator:** Focus on `libcocos.so` detection, TypeScript/JS asset extraction, scene/asset analysis
- **Both:** Need a unified `Cocos2dEngine` class with variant detection and branching logic
