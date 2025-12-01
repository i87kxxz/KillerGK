/**
 * @file KillerGK.hpp
 * @brief Main header file for the KillerGK GUI Library
 * 
 * KillerGK is a modern, high-performance C++ GUI library built on Vulkan,
 * designed to provide WPF-level capabilities with the simplicity of a Builder Pattern API.
 */

#pragma once

// Core module
#include "core/Types.hpp"
#include "core/Application.hpp"
#include "core/Window.hpp"
#include "core/Error.hpp"

// Platform abstraction
#include "platform/Platform.hpp"

// Widget system
#include "widgets/Widget.hpp"
#include "widgets/Button.hpp"
#include "widgets/TextField.hpp"
#include "widgets/Label.hpp"
#include "widgets/Image.hpp"

// Layout system
#include "layout/Layout.hpp"

// Animation system
#include "animation/Animation.hpp"

// Theme system
#include "theme/Theme.hpp"

// Rendering
#include "rendering/Renderer.hpp"
#include "rendering/Texture.hpp"
#include "rendering/TextureAtlas.hpp"
#include "rendering/VisualEffects.hpp"

// Text system
#include "text/Font.hpp"
#include "text/TextRenderer.hpp"
#include "text/BiDi.hpp"
#include "text/RichText.hpp"
#include "text/SyntaxHighlighter.hpp"
#include "text/IconFont.hpp"

// Resource management
#include "resources/ResourceManager.hpp"

/**
 * @namespace KGK
 * @brief Main namespace for KillerGK GUI Library
 */
namespace KGK = KillerGK;
