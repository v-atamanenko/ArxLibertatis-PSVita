/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "core/Config.h"

#include <stddef.h>
#include <algorithm>
// #include <exception>
#include <sstream>
#include <string_view>

#include <boost/preprocessor/stringize.hpp>

#include "input/Input.h"
#include "input/Keyboard.h"
#include "input/Mouse.h"

#include "io/fs/Filesystem.h"
#include "io/fs/FileStream.h"
#include "io/IniReader.h"
#include "io/IniSection.h"
#include "io/IniWriter.h"
#include "io/log/Logger.h"

#include "math/Vector.h"

#include "platform/CrashHandler.h"

#include "util/Number.h"
#include "util/String.h"


// To avoid conflicts with potential other classes/namespaces
namespace {

/* Default values for config */
namespace Default {

#define ARX_DEFAULT_WIDTH 640
#define ARX_DEFAULT_HEIGHT 480
#define THUMBNAIL_DEFAULT_WIDTH 320
#define THUMBNAIL_DEFAULT_HEIGHT 200

#ifdef __vita__
#undef ARX_DEFAULT_WIDTH
#undef ARX_DEFAULT_HEIGHT
#define ARX_DEFAULT_WIDTH 720
#define ARX_DEFAULT_HEIGHT 408
#endif

constexpr const std::string_view
	language,
	audio,
	renderer = "auto",
	resolution = "auto",
	audioBackend = "auto",
	audioDevice = "auto",
	windowSize = BOOST_PP_STRINGIZE(ARX_DEFAULT_WIDTH) "x"
	             BOOST_PP_STRINGIZE(ARX_DEFAULT_HEIGHT),
	debugLevels,
	realtimeOverride,
	bufferUpload,
	extensionOverride,
	thumbnailSize = BOOST_PP_STRINGIZE(THUMBNAIL_DEFAULT_WIDTH) "x"
	                BOOST_PP_STRINGIZE(THUMBNAIL_DEFAULT_HEIGHT);

constexpr const int
	refreshRate = 0,
#ifndef __vita__
	levelOfDetail = 2,
#else
	levelOfDetail = 0,
#endif
	vsync = -1,
	fpsLimit = -1,
	maxAnisotropicFiltering = 9001,
	alphaCutoutAntialiasing = 2,
	cinematicWidescreenMode = CinematicFadeEdges,
	hudScaleFilter = UIFilterBilinear,
	fontWeight = 2,
	hrtf = audio::HRTFDefault,
	autoReadyWeapon = AutoReadyWeaponNearEnemies,
	mouseSensitivity = 6,
	mouseAcceleration = 0,
	migration = Config::OriginalAssets,
	quicksaveSlots = 3,
	bufferSize = 0,
	quickLevelTransition = JumpToChangeLevel;

constexpr const bool
	fullscreen = true,
	viewBobbing = true,
	screenShake = true,
	showCrosshair = true,
#ifndef __vita__
	antialiasing = true,
	colorkeyAntialiasing = true,
#else
	antialiasing = false,
	colorkeyAntialiasing = false,
#endif
	limitSpeechWidth = true,
	hudScaleInteger = true,
	bookScaleInteger = false,
	cursorScaleInteger = true,
	minimizeOnFocusLost = true,
	eax = true,
	muteOnFocusLost = false,
	invertMouse = false,
	mouseLookToggle = true,
	autoDescription = true,
	forceToggle = false,
	rawMouseInput = true,
	borderTurning = true,
	useAltRuneRecognition = true,
	improvedBowAim = true;

#ifdef ARX_DEBUG
const bool allowConsole = true;
#else
const bool allowConsole = false;
#endif

constexpr const float
#ifndef __vita__
	fogDistance = 10.f,
	hudScale = 0.5f,
	fontSize = 1.f,
#else
	fogDistance = 3.f,
	hudScale = 1.5f,
	fontSize = 125.f,
#endif
	gamma = 5.f,
	fov = 75.f,
	bookScale = 1.f,
	cursorScale = 0.5f,
	volume = 10.f,
	sfxVolume = 10.f,
	speechVolume = 10.f,
	ambianceVolume = 10.f;

constexpr const ActionKey actions[NUM_ACTION_KEY] = {
	ActionKey(Keyboard::Key_Spacebar), // JUMP
	ActionKey(Keyboard::Key_LeftCtrl, Keyboard::Key_RightCtrl), // MAGICMODE
	ActionKey(Keyboard::Key_LeftShift, Keyboard::Key_RightShift), // STEALTHMODE
	ActionKey(Keyboard::Key_W, Keyboard::Key_UpArrow), // WALKFORWARD
	ActionKey(Keyboard::Key_S, Keyboard::Key_DownArrow), // WALKBACKWARD
	ActionKey(Keyboard::Key_A), // STRAFELEFT
	ActionKey(Keyboard::Key_D), // STRAFERIGHT
	ActionKey(Keyboard::Key_Q), // LEANLEFT
	ActionKey(Keyboard::Key_E), // LEANRIGHT
	ActionKey(Keyboard::Key_X), // CROUCH
	ActionKey(Keyboard::Key_F, Keyboard::Key_Enter), // USE
	ActionKey(Mouse::Button_0), // ACTION
	ActionKey(Keyboard::Key_I), // INVENTORY
	ActionKey(Keyboard::Key_Backspace), // BOOK
	ActionKey(Keyboard::Key_F1), // BOOKCHARSHEET
	ActionKey(Keyboard::Key_F2), // BOOKSPELL
	ActionKey(Keyboard::Key_F3), // BOOKMAP
	ActionKey(Keyboard::Key_F4), // BOOKQUEST
	ActionKey(Keyboard::Key_H), // DRINKPOTIONLIFE
	ActionKey(Keyboard::Key_G), // DRINKPOTIONMANA
	ActionKey(),                // DRINKPOTIONCURE
	ActionKey(Keyboard::Key_T), // TORCH
	ActionKey(Keyboard::Key_1), // PRECAST1
	ActionKey(Keyboard::Key_2), // PRECAST2
	ActionKey(Keyboard::Key_3), // PRECAST3
	ActionKey(Keyboard::Key_Tab, Keyboard::Key_NumPad0), // WEAPON
	ActionKey(Keyboard::Key_F9), // QUICKLOAD
	ActionKey(Keyboard::Key_F5), // QUICKSAVE
	ActionKey(Keyboard::Key_LeftArrow), // TURNLEFT
	ActionKey(Keyboard::Key_RightArrow), // TURNRIGHT
	ActionKey(Keyboard::Key_PageUp), // LOOKUP
	ActionKey(Keyboard::Key_PageDown), // LOOKDOWN
	ActionKey(Keyboard::Key_LeftAlt), // STRAFE
	ActionKey(Keyboard::Key_End), // CENTERVIEW
	ActionKey(Keyboard::Key_L, Mouse::Button_1), // FREELOOK
	ActionKey(Keyboard::Key_Minus), // PREVIOUS
	ActionKey(Keyboard::Key_Equals), // NEXT
	ActionKey(Keyboard::Key_C), // CROUCHTOGGLE
	ActionKey(Keyboard::Key_B), // UNEQUIPWEAPON
	ActionKey(Keyboard::Key_4), // CANCELCURSPELL
	ActionKey(Keyboard::Key_R, Keyboard::Key_M), // MINIMAP
	ActionKey((Keyboard::Key_LeftAlt << 16) | Keyboard::Key_Enter, (Keyboard::Key_RightAlt << 16) | Keyboard::Key_Enter), // TOGGLE_FULLSCREEN
	ActionKey(Keyboard::Key_Grave), // CONSOLE
	ActionKey(Keyboard::Key_ScrollLock, Keyboard::Key_Backslash), // DEBUG
};

} // namespace Default

namespace Section {

constexpr const std::string_view
	Language = "language",
	Video = "video",
	Interface = "interface",
	Window = "window",
	Audio = "audio",
	Input = "input",
	Key = "key",
	Misc = "misc";

} // namespace Section

namespace Key {

// Language options
constexpr const std::string_view
	language = "string",
	audio = "audio";

// Video options
constexpr const std::string_view
	renderer = "renderer",
	resolution = "resolution",
	refreshRate = "refresh_rate",
	fullscreen = "full_screen",
	levelOfDetail = "others_details",
	fogDistance = "fog",
	gamma = "gamma",
	vsync = "vsync",
	fpsLimit = "fps_limit",
	fov = "fov",
	viewBobbing = "view_bobbing",
	screenShake = "screen_shake",
	antialiasing = "antialiasing",
	maxAnisotropicFiltering = "max_anisotropic_filtering",
	colorkeyAntialiasing = "colorkey_antialiasing",
	alphaCutoutAntialiasing = "alpha_cutout_antialiasing",
	bufferSize = "buffer_size",
	bufferUpload = "buffer_upload",
	extensionOverride = "extension_override";

// Interface options
constexpr const std::string_view
	showCrosshair = "show_crosshair",
	limitSpeechWidth = "limit_speech_width",
	cinematicWidescreenMode = "cinematic_widescreen_mode",
	hudScale = "hud_scale",
	hudScaleInteger = "hud_scale_integer",
	bookScale = "book_scale",
	bookScaleInteger = "book_scale_integer",
	cursorScale = "cursor_scale",
	cursorScaleInteger = "cursor_scale_integer",
	hudScaleFilter = "scale_filter",
	fontSize = "font_size",
	fontWeight = "font_weight",
	thumbnailSize = "save_thumbnail_size";

// Window options
constexpr const std::string_view
	windowSize = "size",
	minimizeOnFocusLost = "minimize_on_focus_lost";

// Audio options
constexpr const std::string_view
	audioBackend = "backend",
	audioDevice = "device",
	volume = "master_volume",
	sfxVolume = "effects_volume",
	speechVolume = "speech_volume",
	ambianceVolume = "ambiance_volume",
	eax = "eax",
	hrtf = "hrtf",
	muteOnFocusLost = "mute_on_focus_lost";

// Input options
constexpr const std::string_view
	invertMouse = "invert_mouse",
	autoReadyWeapon = "auto_ready_weapon",
	mouseLookToggle = "mouse_look_toggle",
	mouseSensitivity = "mouse_sensitivity",
	mouseAcceleration = "mouse_acceleration",
	rawMouseInput = "raw_mouse_input",
	autoDescription = "auto_description",
	borderTurning = "border_turning",
	useAltRuneRecognition = "improved_rune_recognition",
	improvedBowAim = "improved_bow_aim",
	quickLevelTransition = "quick_level_transition",
	allowConsole = "allow_console";

// Input key options
constexpr const std::string_view actions[NUM_ACTION_KEY] = {
	"jump",
	"magic_mode",
	"stealth_mode",
	"walk_forward",
	"walk_backward",
	"strafe_left",
	"strafe_right",
	"lean_left",
	"lean_right",
	"crouch",
	"mouselook", // TODO rename to "use"?
	"action_combine",
	"inventory",
	"book",
	"char_sheet",
	"magic_book",
	"map",
	"quest_book",
	"drink_potion_life",
	"drink_potion_mana",
	"drink_potion_cure",
	"torch",
	"precast_1",
	"precast_2",
	"precast_3",
	"draw_weapon",
	"quickload",
	"quicksave",
	"turn_left",
	"turn_right",
	"look_up",
	"look_down",
	"strafe",
	"center_view",
	"freelook",
	"previous",
	"next",
	"crouch_toggle",
	"unequip_weapon",
	"cancel_current_spell",
	"minimap",
	"toggle_fullscreen",
	"console",
	"debug",
};

// Misc options
constexpr const std::string_view
	forceToggle = "forcetoggle",
	migration = "migration",
	quicksaveSlots = "quicksave_slots",
	debugLevels = "debug",
	realtimeOverride = "realtime_override";

} // namespace Key

class ConfigReader : public IniReader {
	
public:
	
	void getActionKey(std::string_view section, std::string_view key, InputKeyId & binding) const;
	
	ActionKey getActionKey(std::string_view section, ControlAction index) const;
	
};

class ConfigWriter : public IniWriter {
	
public:
	
	explicit ConfigWriter(std::ostream & _output) : IniWriter(_output) { }
	
	void writeActionKey(ControlAction index, const ActionKey & value);

};

void ConfigWriter::writeActionKey(ControlAction index, const ActionKey & value) {
	
	std::string v1 = Input::getKeyName(value.key[0]);
	writeKey(std::string(Key::actions[index]) += "_k0", v1);
	
	std::string v2 = Input::getKeyName(value.key[1]);
	writeKey(std::string(Key::actions[index]) += "_k1", v2);
}

void ConfigReader::getActionKey(std::string_view section, std::string_view key,
                                InputKeyId & binding) const {
	
	const IniKey * setting = getKey(section, key);
	if(setting) {
		InputKeyId id = Input::getKeyId(setting->getValue());
		if(id == ActionKey::UNUSED && !setting->getValue().empty() && setting->getValue() != Input::KEY_NONE) {
			LogWarning << "Error parsing key name for " <<  key << ": \"" << setting->getValue()
			           << "\", resetting to \"" << Input::getKeyName(binding) << "\"";
		} else if(id == InputKeyId(Keyboard::Key_Escape)) {
			LogWarning << "Invalid key for " <<  key << ": \"" << setting->getValue()
			           << "\", resetting to \"" << Input::getKeyName(binding) << "\"";
		} else {
			binding = id;
		}
	}
	
}

ActionKey ConfigReader::getActionKey(std::string_view section, ControlAction index) const {
	
	std::string_view key = Key::actions[index];
	ActionKey action_key = Default::actions[index];
	
	getActionKey(section, std::string(key) += "_k0", action_key.key[0]);
	getActionKey(section, std::string(key) += "_k1", action_key.key[1]);
	
	LogDebug("[" << section << "] " << key << " = \"" << Input::getKeyName(action_key.key[0]) << "\", \"" << Input::getKeyName(action_key.key[1]) << "\"");
	
	return action_key;
}

} // anonymous namespace

Config config;

void Config::setDefaultActionKeys() {
	
	for(size_t i = 0; i < NUM_ACTION_KEY; i++) {
		actions[i] = Default::actions[i];
	}
}

bool Config::setActionKey(ControlAction actionId, size_t index, InputKeyId key) {
	
	if(actionId >= NUM_ACTION_KEY || index > 1) {
		arx_assert(false);
		return false;
	}
	
	bool changed = false;
	
	// remove existing key assignments
	for(size_t i = 0; i < NUM_ACTION_KEY; i++) {
		for(size_t k = 0; k < 2; k++) {
			if((ControlAction(i) != actionId || k != index) && actions[i].key[k] == key) {
				actions[i].key[k] = ActionKey::UNUSED;
				changed = true;
			}
		}
	}
	
	if(index == 1 && actions[actionId].key[0] == ActionKey::UNUSED) {
		actions[actionId].key[index] = ActionKey::UNUSED;
		changed = true;
		index = 0;
	}
	
	if(actions[actionId].key[index] != key) {
		actions[actionId].key[index] = key;
		changed = true;
	}
	
	return changed;
}

void Config::setOutputFile(const fs::path & file) {
	m_file = file;
	CrashHandler::addAttachedFile(m_file);
}

bool Config::save() {
	
	// Finally save it all to file/stream
	fs::ofstream out(m_file);
	if(!out.is_open()) {
		return false;
	}
	
	ConfigWriter writer(out);
	
	// language
	writer.beginSection(Section::Language);
	writer.writeKey(Key::language, interface.language);
	writer.writeKey(Key::audio, audio.language);
	
	// video
	writer.beginSection(Section::Video);
	writer.writeKey(Key::renderer, video.renderer);
	if(video.mode.resolution == Vec2i(0)) {
		writer.writeKey(Key::resolution, Default::resolution);
	} else {
		std::ostringstream oss;
		oss << video.mode.resolution.x << 'x' << video.mode.resolution.y;
		writer.writeKey(Key::resolution, oss.str());
	}
	writer.writeKey(Key::refreshRate, int(video.mode.refresh));
	writer.writeKey(Key::fullscreen, video.fullscreen);
	writer.writeKey(Key::levelOfDetail, video.levelOfDetail);
	writer.writeKey(Key::fogDistance, video.fogDistance);
	writer.writeKey(Key::gamma, video.gamma);
	writer.writeKey(Key::vsync, video.vsync);
	writer.writeKey(Key::fpsLimit, video.fpsLimit);
	writer.writeKey(Key::fov, video.fov);
	writer.writeKey(Key::viewBobbing, video.viewBobbing);
	writer.writeKey(Key::screenShake, video.screenShake);
	writer.writeKey(Key::antialiasing, video.antialiasing);
	writer.writeKey(Key::maxAnisotropicFiltering, video.maxAnisotropicFiltering);
	writer.writeKey(Key::colorkeyAntialiasing, video.colorkeyAntialiasing);
	writer.writeKey(Key::alphaCutoutAntialiasing, video.alphaCutoutAntialiasing);
	writer.writeKey(Key::bufferSize, video.bufferSize);
	writer.writeKey(Key::bufferUpload, video.bufferUpload);
	writer.writeKey(Key::extensionOverride, video.extensionOverride);
	
	// interface
	writer.beginSection(Section::Interface);
	writer.writeKey(Key::showCrosshair, interface.showCrosshair);
	writer.writeKey(Key::limitSpeechWidth, interface.limitSpeechWidth);
	writer.writeKey(Key::cinematicWidescreenMode, int(interface.cinematicWidescreenMode));
	writer.writeKey(Key::hudScale, interface.hudScale);
	writer.writeKey(Key::hudScaleInteger, interface.hudScaleInteger);
	writer.writeKey(Key::bookScale, interface.bookScale);
	writer.writeKey(Key::bookScaleInteger, interface.bookScaleInteger);
	writer.writeKey(Key::cursorScale, interface.cursorScale);
	writer.writeKey(Key::cursorScaleInteger, interface.cursorScaleInteger);
	writer.writeKey(Key::fontSize, interface.fontSize);
	writer.writeKey(Key::fontWeight, interface.fontWeight);
	writer.writeKey(Key::hudScaleFilter, interface.scaleFilter);
	std::ostringstream osst;
	osst << interface.thumbnailSize.x << 'x' << interface.thumbnailSize.y;
	writer.writeKey(Key::thumbnailSize, osst.str());
	
	// window
	writer.beginSection(Section::Window);
	std::ostringstream oss;
	oss << window.size.x << 'x' << window.size.y;
	writer.writeKey(Key::windowSize, oss.str());
	writer.writeKey(Key::minimizeOnFocusLost, window.minimizeOnFocusLost);
	
	// audio
	writer.beginSection(Section::Audio);
	writer.writeKey(Key::audioBackend, audio.backend);
	writer.writeKey(Key::audioDevice, audio.device);
	writer.writeKey(Key::volume, audio.volume);
	writer.writeKey(Key::sfxVolume, audio.sfxVolume);
	writer.writeKey(Key::speechVolume, audio.speechVolume);
	writer.writeKey(Key::ambianceVolume, audio.ambianceVolume);
	writer.writeKey(Key::eax, audio.eax);
	writer.writeKey(Key::hrtf, int(audio.hrtf));
	writer.writeKey(Key::muteOnFocusLost, audio.muteOnFocusLost);
	
	// input
	writer.beginSection(Section::Input);
	writer.writeKey(Key::invertMouse, input.invertMouse);
	writer.writeKey(Key::autoReadyWeapon, int(input.autoReadyWeapon));
	writer.writeKey(Key::mouseLookToggle, input.mouseLookToggle);
	writer.writeKey(Key::mouseSensitivity, input.mouseSensitivity);
	writer.writeKey(Key::mouseAcceleration, input.mouseAcceleration);
	writer.writeKey(Key::rawMouseInput, input.rawMouseInput);
	writer.writeKey(Key::autoDescription, input.autoDescription);
	writer.writeKey(Key::borderTurning, input.borderTurning);
	writer.writeKey(Key::useAltRuneRecognition, input.useAltRuneRecognition);
	writer.writeKey(Key::improvedBowAim, input.improvedBowAim);
	writer.writeKey(Key::quickLevelTransition, int(input.quickLevelTransition));
	if(input.allowConsole) {
		// Only write this if true so that switching from release to debug builds enables the console
		writer.writeKey(Key::allowConsole, input.allowConsole);
	}
	
	// key
	writer.beginSection(Section::Key);
	for(size_t i = 0; i < NUM_ACTION_KEY; i++) {
		writer.writeActionKey(ControlAction(i), actions[i]);
	}
	
	// misc
	writer.beginSection(Section::Misc);
	writer.writeKey(Key::forceToggle, misc.forceToggle);
	writer.writeKey(Key::migration, misc.migration);
	writer.writeKey(Key::quicksaveSlots, misc.quicksaveSlots);
	writer.writeKey(Key::debugLevels, misc.debug);
	writer.writeKey(Key::realtimeOverride, misc.realtimeOverride);
	
	return writer.flush();
}

static Vec2i parseResolution(std::string_view resolution, Vec2i defaultResolution) {
	
	size_t pos = resolution.find('x');
	if(pos != std::string_view::npos) {
		Vec2i result(util::toInt(resolution.substr(0, pos)).value_or(0),
		             util::toInt(resolution.substr(pos + 1)).value_or(0));
		if(result.x > 0 && result.y > 0) {
			return result;
		}
	}
	
	LogWarning << "Bad resolution string: " << resolution;
	return defaultResolution;
}

bool Config::init(const fs::path & file) {
	
	std::string data = fs::read(file);
	
	bool loaded = !data.empty();
	
	ConfigReader reader;
	
	if(!reader.read(data)) {
		LogWarning << "Errors while parsing config file";
	}
	
	// Get locale language
	interface.language = util::toLowercase(reader.getKey(Section::Language, Key::language, Default::language));
	audio.language = util::toLowercase(reader.getKey(Section::Language, Key::audio, Default::audio));
	
	// Get video settings
	video.renderer = reader.getKey(Section::Video, Key::renderer, Default::renderer);
	std::string_view resolution = reader.getKey(Section::Video, Key::resolution, Default::resolution);
	if(resolution == "auto") {
		video.mode.resolution = Vec2i(0);
	} else {
		video.mode.resolution = parseResolution(resolution, Vec2i(ARX_DEFAULT_WIDTH, ARX_DEFAULT_HEIGHT));
	}
	video.mode.refresh = reader.getKey(Section::Video, Key::refreshRate, Default::refreshRate);
	video.fullscreen = reader.getKey(Section::Video, Key::fullscreen, Default::fullscreen);
	video.levelOfDetail = reader.getKey(Section::Video, Key::levelOfDetail, Default::levelOfDetail);
	video.fogDistance = reader.getKey(Section::Video, Key::fogDistance, Default::fogDistance);
	video.gamma = reader.getKey(Section::Video, Key::gamma, Default::gamma);
	int vsync = reader.getKey(Section::Video, Key::vsync, Default::vsync);
	video.vsync = glm::clamp(vsync, -1, 1);
	int fpsLimit = reader.getKey(Section::Video, Key::fpsLimit, Default::fpsLimit);
	video.fpsLimit = std::max(fpsLimit, -1);
	float fov = reader.getKey(Section::Video, Key::fov, Default::fov);
	video.fov = std::max(fov, 10.f);
	video.viewBobbing = reader.getKey(Section::Video, Key::viewBobbing, Default::viewBobbing);
	video.screenShake = reader.getKey(Section::Video, Key::screenShake, Default::screenShake);
	video.antialiasing = reader.getKey(Section::Video, Key::antialiasing, Default::antialiasing);
	int anisoFiltering = reader.getKey(Section::Video, Key::maxAnisotropicFiltering, Default::maxAnisotropicFiltering);
	video.maxAnisotropicFiltering = std::max(anisoFiltering, 1);
	video.colorkeyAntialiasing = reader.getKey(Section::Video, Key::colorkeyAntialiasing, Default::colorkeyAntialiasing);
	int alphaCutoutAA = reader.getKey(Section::Video, Key::alphaCutoutAntialiasing, Default::alphaCutoutAntialiasing);
	video.alphaCutoutAntialiasing = glm::clamp(alphaCutoutAA, 0, 2);
	video.bufferSize = std::max(reader.getKey(Section::Video, Key::bufferSize, Default::bufferSize), 0);
	video.bufferUpload = reader.getKey(Section::Video, Key::bufferUpload, Default::bufferUpload);
	video.extensionOverride = reader.getKey(Section::Video, Key::extensionOverride, Default::extensionOverride);
	
	// Get interface settings
	bool oldCrosshair = reader.getKey(Section::Video, Key::showCrosshair, Default::showCrosshair);
	interface.showCrosshair = reader.getKey(Section::Interface, Key::showCrosshair, oldCrosshair);
	interface.limitSpeechWidth = reader.getKey(Section::Interface, Key::limitSpeechWidth, Default::limitSpeechWidth);
	int cinematicMode = reader.getKey(Section::Interface, Key::cinematicWidescreenMode, Default::cinematicWidescreenMode);
	interface.cinematicWidescreenMode = CinematicWidescreenMode(glm::clamp(cinematicMode, 0, 2));
	float hudScale = reader.getKey(Section::Interface, Key::hudScale, Default::hudScale);
	interface.hudScale = glm::clamp(hudScale, 0.f, 1.f);
	interface.hudScaleInteger = reader.getKey(Section::Interface, Key::hudScaleInteger, Default::hudScaleInteger);
	float bookScale = reader.getKey(Section::Interface, Key::bookScale, Default::bookScale);
	interface.bookScale = glm::clamp(bookScale, 0.f, 1.f);
	interface.bookScaleInteger = reader.getKey(Section::Interface, Key::bookScaleInteger, Default::bookScaleInteger);
	float cursorScale = reader.getKey(Section::Interface, Key::cursorScale, Default::cursorScale);
	interface.cursorScale = glm::clamp(cursorScale, 0.f, 1.f);
	interface.cursorScaleInteger = reader.getKey(Section::Interface, Key::cursorScaleInteger, Default::cursorScaleInteger);
	float fontSize = reader.getKey(Section::Interface, Key::fontSize, Default::fontSize);
	interface.fontSize = glm::clamp(fontSize, 0.5f, 2.f);
	int fontWeight = reader.getKey(Section::Interface, Key::fontWeight, Default::fontWeight);
	interface.fontWeight = glm::clamp(fontWeight, 0, 5);
	int hudScaleFilter = reader.getKey(Section::Interface, Key::hudScaleFilter, Default::hudScaleFilter);
	interface.scaleFilter = UIScaleFilter(glm::clamp(hudScaleFilter, 0, 1));
	std::string_view thumbnailSize = reader.getKey(Section::Interface, Key::thumbnailSize, Default::thumbnailSize);
	interface.thumbnailSize = parseResolution(thumbnailSize, Vec2i(THUMBNAIL_DEFAULT_WIDTH, THUMBNAIL_DEFAULT_HEIGHT));
	
	// Get window settings
	std::string_view windowSize = reader.getKey(Section::Window, Key::windowSize, Default::windowSize);
	window.size = parseResolution(windowSize, Vec2i(ARX_DEFAULT_WIDTH, ARX_DEFAULT_HEIGHT));
	window.minimizeOnFocusLost = reader.getKey(Section::Window, Key::minimizeOnFocusLost,
	                                           Default::minimizeOnFocusLost);
	
	// Get audio settings
	audio.backend = reader.getKey(Section::Audio, Key::audioBackend, Default::audioBackend);
	audio.device = reader.getKey(Section::Audio, Key::audioDevice, Default::audioDevice);
	audio.volume = reader.getKey(Section::Audio, Key::volume, Default::volume);
	audio.sfxVolume = reader.getKey(Section::Audio, Key::sfxVolume, Default::sfxVolume);
	audio.speechVolume = reader.getKey(Section::Audio, Key::speechVolume, Default::speechVolume);
	audio.ambianceVolume = reader.getKey(Section::Audio, Key::ambianceVolume, Default::ambianceVolume);
	audio.eax = reader.getKey(Section::Audio, Key::eax, Default::eax);
	int hrtf = reader.getKey(Section::Audio, Key::hrtf, int(Default::hrtf));
	audio.hrtf = audio::HRTFAttribute(glm::clamp(hrtf, -1, 1));
	audio.muteOnFocusLost = reader.getKey(Section::Audio, Key::muteOnFocusLost, Default::muteOnFocusLost);
	
	// Get input settings
	input.invertMouse = reader.getKey(Section::Input, Key::invertMouse, Default::invertMouse);
	int autoReadyWeapon = reader.getKey(Section::Input, Key::autoReadyWeapon, Default::autoReadyWeapon);
	input.autoReadyWeapon = AutoReadyWeapon(glm::clamp(autoReadyWeapon, 0, 2));
	input.mouseLookToggle = reader.getKey(Section::Input, Key::mouseLookToggle, Default::mouseLookToggle);
	input.mouseSensitivity = reader.getKey(Section::Input, Key::mouseSensitivity, Default::mouseSensitivity);
	input.mouseAcceleration = reader.getKey(Section::Input, Key::mouseAcceleration, Default::mouseAcceleration);
	input.rawMouseInput = reader.getKey(Section::Input, Key::rawMouseInput, Default::rawMouseInput);
	input.autoDescription = reader.getKey(Section::Input, Key::autoDescription, Default::autoDescription);
	input.borderTurning = reader.getKey(Section::Input, Key::borderTurning, Default::borderTurning);
	input.useAltRuneRecognition = reader.getKey(Section::Input, Key::useAltRuneRecognition, Default::useAltRuneRecognition);
	input.improvedBowAim = reader.getKey(Section::Input, Key::improvedBowAim, Default::improvedBowAim);
	int quickLevelTransition = reader.getKey(Section::Input, Key::quickLevelTransition, Default::quickLevelTransition);
	input.quickLevelTransition = QuickLevelTransition(glm::clamp(quickLevelTransition, 0, 2));
	input.allowConsole = reader.getKey(Section::Input, Key::allowConsole, Default::allowConsole);
	
	// Get action key settings
	for(size_t i = 0; i < NUM_ACTION_KEY; i++) {
		actions[i] = reader.getActionKey(Section::Key, ControlAction(i));
	}
	
	// Get miscellaneous settings
	misc.forceToggle = reader.getKey(Section::Misc, Key::forceToggle, Default::forceToggle);
	misc.migration = MigrationStatus(reader.getKey(Section::Misc, Key::migration, Default::migration));
	misc.quicksaveSlots = std::max(reader.getKey(Section::Misc, Key::quicksaveSlots, Default::quicksaveSlots), 1);
	misc.debug = reader.getKey(Section::Misc, Key::debugLevels, Default::debugLevels);
	misc.realtimeOverride = reader.getKey(Section::Misc, Key::realtimeOverride, Default::realtimeOverride);
	
	return loaded;
}
