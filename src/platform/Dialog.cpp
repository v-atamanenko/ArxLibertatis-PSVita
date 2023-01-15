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
 
#include "platform/Dialog.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string_view>
#include <vector>

#include "platform/Platform.h"

#include "Configure.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
#include <windows.h>
#endif

#if ARX_HAVE_SDL2
#include <SDL.h>
#endif

#include "core/Version.h"
#include "platform/Process.h"
#include "platform/WindowsUtils.h"
#include "util/String.h"


namespace platform {

#if ARX_PLATFORM == ARX_PLATFORM_WIN32

static bool showDialog(DialogType type, const std::string & message,
                       const std::string & title) {
	
	UINT flags;
	switch(type) {
		case DialogInfo:      flags = MB_ICONINFORMATION | MB_OK; break;
		case DialogWarning:   flags = MB_ICONWARNING | MB_OK; break;
		case DialogError:     flags = MB_ICONERROR | MB_OK; break;
		case DialogYesNo:     flags = MB_ICONQUESTION | MB_YESNO; break;
		case DialogWarnYesNo: flags = MB_ICONWARNING | MB_YESNO; break;
		case DialogOkCancel:  flags = MB_ICONQUESTION | MB_OKCANCEL; break;
	}
	
	int ret = MessageBoxW(nullptr, platform::WideString(message), platform::WideString(title),
	                      flags | MB_SETFOREGROUND | MB_TOPMOST);
	
	switch(ret) {
		case IDCANCEL:
		case IDNO:
			return false;
		case IDYES:
		case IDOK:
			return true;
	}
	
	return false;
}

#elif ARX_PLATFORM == ARX_PLATFORM_MACOS

// See Dialog.mm for the implementation of showDialog
bool showDialog(DialogType type, const std::string & message, const std::string & title);

#elif defined(__vita__)

// TODO: Do proper dialogs

static bool showDialog(DialogType type, const std::string & message,
                       const std::string & title) {

	std::string msg = "Game wants to show dialog of type ";
	
	unsigned int flags;
	switch(type) {
		case DialogInfo:      msg += "DialogInfo"; break;
		case DialogWarning:   msg += "DialogWarning"; break;
		case DialogError:     msg += "DialogError"; break;
		case DialogYesNo:     msg += "DialogYesNo"; break;
		case DialogWarnYesNo: msg += "DialogWarnYesNo"; break;
		case DialogOkCancel:  msg += "DialogOkCancel"; break;
	}
	
	msg += ", with title \"" + title + "\", and message: " + message;

	printf("%s\n", msg.c_str());
	
	return false;
}

#else

static bool isAllowedInUrl(char c) {
	return !isspace(c) && c != '"' && c != '\'' && c != ')';
}

static void closeLink(std::stringstream & oss, size_t start) {
	size_t end = oss.tellp();
	std::vector<char> url(end - start);
	oss.seekg(start).read(&url.front(), end - start);
	oss << "\">";
	oss.write(&url.front(), end - start);
	oss << "</a>";
}

/*!
 * Minimal HTML formatter for error messages
 *
 * Features:
 * ' * ' => html link or nicer bullet point
 * 'http://' / 'https://' => link
 * "..." => "<b>...</b>"
 *
 * \param newline Keep don't convert newlines to &lt;br&gr; tags.
 * \param ul      Use HTML lists.
 */
static std::string formatAsHtml(std::string_view text, bool newline, bool ul = false) {
	
	std::stringstream oss;
	
	bool list = false, first = true;
	
	for(std::string_view line : util::split(text, '\n')) {
		
		size_t i = 0;
		
		if(line.length() >= 3 && line.substr(0, 3) == " * ") {
			i += 3;
			
			if(ul && !list) {
				oss << "<ul>";
				list = true;
			} else if(!ul && !first) {
				oss << (newline ? "\n" : "<br>");
			}
			
			oss << (ul ? "<li>" : " &#8226; "); // &bull;
			
		} else if(list) {
			oss << "</ul>";
			list = false;
		} else if(!first) {
			oss << (newline ? "\n" : "<br>");
		}
		first = false;
		
		bool italic = false;
		if(line.length() >= i + 3 && line.substr(i, 3) == "-> ") {
			i += 3;
			oss << "&#8594;&#160; <i>"; // &rarr;&nbsp;
			italic = true;
		}
		
		bool quote = false, link = false;
		
		size_t link_start = 0;
		
		for(; i < line.length(); i++) {
			
			if(link && !isAllowedInUrl(line[i])) {
				closeLink(oss, link_start);
				link = false;
			}
			
			if(line[i] == '<') {
				oss << "&lt;";
			} else if(line[i] == '>') {
				oss << "&gt;";
			} else if(line[i] == '"') {
				if(!quote) {
					oss << "\"<b>";
				} else {
					oss << "</b>\"";
				}
				quote = !quote;
			} else if(!link && line.substr(i, 7) == "http://") {
				oss << "<a href=\"";
				link_start = oss.tellp(), link = true;
				oss << "http://";
				i += 6;
			} else if(!link && line.substr(i, 8) == "https://") {
				oss << "<a href=\"";
				link_start = oss.tellp(), link = true;
				oss << "https://";
				i += 7;
			} else {
				oss << line[i];
			}
			
		}
		
		if(link) {
			closeLink(oss, link_start);
		}
		
		if(quote) {
			oss << "</b>";
		}
		
		if(italic) {
			oss << "</i>";
		}
		
	}
	
	return oss.str();
}

static int zenityCommand(DialogType type, const std::string & message,
                         const std::string & title) {
	
	std::vector<const char *> command;
	command.push_back("zenity");
	switch(type) {
		case DialogInfo:      command.push_back("--info"); break;
		case DialogWarning:   command.push_back("--warning"); break;
		case DialogError:     command.push_back("--error"); break;
		case DialogYesNo: {
			command.push_back("--question");
			command.push_back("--ok-label=Yes");
			command.push_back("--cancel-label=No");
			break;
		}
		case DialogWarnYesNo: {
			command.push_back("--question");
			command.push_back("--ok-label=Yes");
			command.push_back("--cancel-label=No");
			command.push_back("--icon-name=dialog-warning");
			command.push_back("--window-icon=warning");
			break;
		}
		case DialogOkCancel:  {
			command.push_back("--question");
			command.push_back("--ok-label=OK");
			command.push_back("--cancel-label=Cancel");
			break;
		}
	}
	command.push_back("--no-wrap");
	std::string messageArg = "--text=" + formatAsHtml(message, true);
	command.push_back(messageArg.c_str());
	std::string titleArg = "--text=" + title;
	command.push_back(messageArg.c_str());
	command.push_back(nullptr);
	
	return platform::run(command.data());
}

static int kdialogCommand(DialogType type, const std::string & message,
                          const std::string & title) {
	
	std::vector<const char *> command;
	command.push_back("kdialog");
	switch(type) {
		case DialogInfo:      command.push_back("--msgbox"); break;
		case DialogWarning:   command.push_back("--sorry"); break;
		case DialogError:     command.push_back("--error"); break;
		case DialogYesNo:     command.push_back("--yesno"); break;
		case DialogWarnYesNo: command.push_back("--warningyesno"); break;
		case DialogOkCancel:  command.push_back("--continuecancel"); break;
	}
	std::string messageArg = formatAsHtml(message, false);
	command.push_back(messageArg.c_str());
	command.push_back("--title");
	command.push_back(title.c_str());
	command.push_back("--icon");
	command.push_back(arx_icon_name.c_str());
	command.push_back(nullptr);
	
	return platform::run(command.data());
}

static void xmessageButtons(std::vector<const char *> & command, DialogType type) {
	
	command.push_back("-center");
	command.push_back("-buttons");
	switch(type) {
		default:             command.push_back("OK"); break;
		case DialogWarnYesNo: [[fallthrough]];
		case DialogYesNo:    command.push_back("Yes:0,No:1"); break;
		case DialogOkCancel: command.push_back("OK:0,Cancel:1"); break;
	}
	
}

static int gxmessageCommand(DialogType type, const std::string & message,
                            const std::string & title) {
	
	std::vector<const char *> command;
	command.push_back("gxmessage");
	command.push_back("-geometry");
	command.push_back("550x300");
	xmessageButtons(command, type);
	command.push_back("-title");
	command.push_back(title.c_str());
	command.push_back(message.c_str());
	command.push_back(nullptr);
	
	return platform::run(command.data());
}

static int xdialogCommand(DialogType type, const std::string & message,
                          const std::string & title) {
	
	std::vector<const char *> command;
	command.push_back("Xdialog");
	command.push_back("--left");
	command.push_back("--title");
	command.push_back(title.c_str());
	switch(type) {
		default:             command.push_back("--msgbox"); break;
		case DialogWarnYesNo: [[fallthrough]];
		case DialogYesNo:    command.push_back("--yesno"); break;
		case DialogOkCancel: {
			command.push_back("--ok-label"), command.push_back("OK");
			command.push_back("--cancel-label"), command.push_back("Cancel");
			command.push_back("--yesno");
			break;
		}
	}
	command.push_back(message.c_str());
	command.push_back("0");
	command.push_back("0");
	command.push_back(nullptr);
	
	return platform::run(command.data());
}

static int xmessageCommand(DialogType type, const std::string & message,
                           const std::string & title) {
	
	ARX_UNUSED(title);
	
	std::vector<const char *> command;
	command.push_back("xmessage");
	xmessageButtons(command, type);
	command.push_back(message.c_str());
	command.push_back(nullptr);
	
	return platform::run(command.data());
}

#if ARX_HAVE_SDL2
static int sdlDialogCommand(DialogType type, const std::string & message,
                            const std::string & title) {
	
	bool wasInitialized = (SDL_WasInit(SDL_INIT_VIDEO) != 0);
	if(!wasInitialized && SDL_Init(SDL_INIT_VIDEO) != 0) {
		return -2;
	}
	
	int ret = -2;
	
	SDL_MessageBoxData box = {
		SDL_MESSAGEBOX_INFORMATION,
		nullptr,
		title.c_str(),
		message.c_str(),
		0, nullptr,
		nullptr
	};
	
	switch(type) {
		case DialogInfo:      box.flags = SDL_MESSAGEBOX_INFORMATION; break;
		case DialogWarning:   box.flags = SDL_MESSAGEBOX_WARNING; break;
		case DialogError:     box.flags = SDL_MESSAGEBOX_ERROR; break;
		case DialogYesNo:     box.flags = SDL_MESSAGEBOX_INFORMATION; break;
		case DialogWarnYesNo: box.flags = SDL_MESSAGEBOX_WARNING; break;
		case DialogOkCancel:  box.flags = SDL_MESSAGEBOX_INFORMATION; break;
	}
	
	const SDL_MessageBoxButtonData buttonsOK[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "OK" },
	};
	const SDL_MessageBoxButtonData buttonsYesNo[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Yes" },
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "No" },
	};
	const SDL_MessageBoxButtonData buttonsOKCancel[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "OK" },
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Cancel" },
	};
	switch(type) {
		case DialogInfo:
		case DialogWarning:
		case DialogError: {
			box.buttons = buttonsOK,
			box.numbuttons = std::size(buttonsOK);
			break;
		}
		case DialogYesNo:
		case DialogWarnYesNo: {
			box.buttons = buttonsYesNo,
			box.numbuttons = std::size(buttonsYesNo);
			break;
		}
		case DialogOkCancel: {
			box.buttons = buttonsOKCancel,
			box.numbuttons = std::size(buttonsOKCancel);
			break;
		}
	}
	
	int buttonid;
	if(SDL_ShowMessageBox(&box, &buttonid) >= 0) {
		ret = (buttonid == -1) ? 2 : buttonid;
	}
	
	if(!wasInitialized) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}
	
	return ret;
}
#endif

static bool showDialog(DialogType type, const std::string & message,
                       const std::string & title) {
	
	typedef int (*dialogCommand_t)(DialogType type, const std::string & message,
	                               const std::string & title);
	
	// This may not be the best way
	const char * session = getenv("DESKTOP_SESSION");
	bool usingKDE = (session != nullptr) && !strcasecmp(session, "kde");
	usingKDE = usingKDE || (getenv("KDE_FULL_SESSION") != nullptr);
	usingKDE = usingKDE || (getenv("KDE_SESSION_UID") != nullptr);
	usingKDE = usingKDE || (getenv("KDE_SESSION_VERSION") != nullptr);
	
	const dialogCommand_t commands[] = {
		usingKDE ? &kdialogCommand : &zenityCommand,
		usingKDE ? &zenityCommand : &kdialogCommand,
		&gxmessageCommand,
		&xdialogCommand,
		#if ARX_HAVE_SDL2
		&sdlDialogCommand,
		#endif
		&xmessageCommand
	};
	
	for(dialogCommand_t command : commands) {
		int code = command(type, message, title);
		if(code >= 0) {
			return code == 0;
		}
	}
	
	/*
	 * If we have no native way to display a message box, fall back to SDL.
	 * This will look ugly on Linux, so do this only if we really have to.
	 */
	#if ARX_HAVE_SDL2
	Uint32 flags = 0;
	switch(type) {
		case DialogInfo:       flags = SDL_MESSAGEBOX_INFORMATION; break;
		case DialogWarning:    flags = SDL_MESSAGEBOX_WARNING;     break;
		case DialogError:      flags = SDL_MESSAGEBOX_ERROR;       break;
		case DialogWarnYesNo:  flags = SDL_MESSAGEBOX_WARNING;     break;
		default: /* unsupported */ break;
	}
	if(flags && !SDL_ShowSimpleMessageBox(flags, title.c_str(), message.c_str(), nullptr)) {
		return true;
	}
	#endif
	
	std::cerr << "Failed to show a dialog: " << title << ": " << message << std::endl;
	return true;
}

#endif

void showInfoDialog(const std::string & message, const std::string & title) {
	showDialog(DialogInfo, message, title);
}

void showWarningDialog(const std::string & message, const std::string & title) {
	showDialog(DialogWarning, message, title);
}

void showErrorDialog(const std::string & message, const std::string & title) {
	showDialog(DialogError, message, title);
}

bool askYesNo(const std::string & question, const std::string & title) {
	return showDialog(DialogYesNo, question, title);
}

bool askYesNoWarning(const std::string & question, const std::string & title) {
	return showDialog(DialogWarnYesNo, question, title);
}

bool askOkCancel(const std::string & question, const std::string & title) {
	return showDialog(DialogOkCancel, question, title);
}

} // namespace platform
