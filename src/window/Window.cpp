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

#include "window/Window.h"

#include <algorithm>

bool DisplayMode::operator<(const DisplayMode & o) const noexcept {
	if(resolution.x != o.resolution.x) {
		return (resolution.x < o.resolution.x);
	} else if(resolution.y != o.resolution.y) {
		return (resolution.y < o.resolution.y);
	} else {
		return (refresh < o.refresh);
	}
}

std::ostream & operator<<(std::ostream & os, const DisplayMode & mode) {
	os << mode.resolution.x << 'x' << mode.resolution.y;
	if(mode.refresh != 0) {
		os << '@' << mode.refresh << "Hz";
	}
	return os;
}

void Window::Listener::onCreateWindow(const Window & /* window */) { }
bool Window::Listener::onCloseWindow(const Window & /* window */) { return true; }
void Window::Listener::onDestroyWindow(const Window & /* window */) { }
void Window::Listener::onMoveWindow(const Window & /* window */) { }
void Window::Listener::onResizeWindow(const Window & /* window */) { }
void Window::Listener::onMinimizeWindow(const Window & /* window */) { }
void Window::Listener::onMaximizeWindow(const Window & /* window */) { }
void Window::Listener::onRestoreWindow(const Window & /* window */) { }
void Window::Listener::onToggleFullscreen(const Window & /* window */) { }
void Window::Listener::onWindowGotFocus(const Window & /* window */) { }
void Window::Listener::onWindowLostFocus(const Window & /* window */) { }
void Window::Listener::onPaintWindow(const Window & /*window*/) { }
void Window::Listener::onDroppedFile(const Window & /*window*/, const fs::path & /* path */) { }

Window::Window()
	: m_position(0, 0)
#ifndef __vita__
	, m_mode(Vec2i(640, 480))
#else
	, m_mode(Vec2i(720, 408))
#endif
	, m_minimized(false)
	, m_maximized(false)
	, m_visible(false)
	, m_fullscreen(false)
	, m_focused(false)
{ }

void Window::addListener(Listener * listener) {
	m_listeners.push_back(listener);
}

void Window::removeListener(Listener * listener) {
	Listeners::iterator it = std::find(m_listeners.begin(), m_listeners.end(), listener);
	if(it != m_listeners.end()) {
		m_listeners.erase(it);
	}
}

bool Window::onClose() {
	for(Listener * listener : m_listeners) {
		bool shouldClose = listener->onCloseWindow(*this);
		if(!shouldClose) {
			return false;
		}
	}
	return true;
}

void Window::onCreate() {
	for(Listener * listener : m_listeners) {
		listener->onCreateWindow(*this);
	}
}

void Window::onDestroy() {
	for(Listener * listener : m_listeners) {
		listener->onDestroyWindow(*this);
	}
}

void Window::onMove(s32 x, s32 y) {
	m_position = Vec2i(x, y);
	for(Listener * listener : m_listeners) {
		listener->onMoveWindow(*this);
	}
}

void Window::onResize(const Vec2i & size) {
	m_mode.resolution = size;
	for(Listener * listener : m_listeners) {
		listener->onResizeWindow(*this);
	}
}

void Window::onMinimize() {
	m_minimized = true, m_maximized = false;
	for(Listener * listener : m_listeners) {
		listener->onMinimizeWindow(*this);
	}
}
	
void Window::onMaximize() {
	m_minimized = false, m_maximized = true;
	for(Listener * listener : m_listeners) {
		listener->onMaximizeWindow(*this);
	}
}

void Window::onRestore() {
	m_minimized = false, m_maximized = false;
	for(Listener * listener : m_listeners) {
		listener->onRestoreWindow(*this);
	}
}

void Window::onShow(bool isVisible) {
	m_visible = isVisible;
}
	
void Window::onToggleFullscreen(bool fullscreen) {
	m_fullscreen = fullscreen;
	for(Listener * listener : m_listeners) {
		listener->onToggleFullscreen(*this);
	}
}
	
void Window::onFocus(bool hasFocus) {
	m_focused = hasFocus;
	if(hasFocus) {
		for(Listener * listener : m_listeners) {
			listener->onWindowGotFocus(*this);
		}
	} else {
		for(Listener * listener : m_listeners) {
			listener->onWindowLostFocus(*this);
		}
	}
}

void Window::onPaint() {
	for(Listener * listener : m_listeners) {
		listener->onPaintWindow(*this);
	}
}

void Window::onDroppedFile(const fs::path & path) {
	for(Listener * listener : m_listeners) {
		listener->onDroppedFile(*this, path);
	}
}
