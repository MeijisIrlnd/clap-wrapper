#pragma once

#include <Windows.h>
#include <wil/resource.h>

#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <string>

#include "detail/standalone/entry.h"

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <fmt/xchar.h>

namespace freeaudio::clap_wrapper::standalone::windows::helpers
{
struct Size
{
  uint32_t width;
  uint32_t height;
};

Size getClientSize(::HWND window);

::HMODULE getInstance();

bool activateWindow(::HWND window);
bool showWindow(::HWND window);
bool hideWindow(::HWND window);
void centerWindow(::HWND window, int width, int height);
bool closeWindow(::HWND window);
bool checkWindowVisibility(::HWND window);
unsigned int getCurrentDpi(::HWND window);
double getCurrentScale(::HWND window);

void abort(unsigned int exitCode = EXIT_FAILURE);
void quit(unsigned int exitCode = EXIT_SUCCESS);

int messageLoop();

std::wstring toUTF16(std::string_view utf8);
std::string toUTF8(std::wstring_view utf16);

void log(const std::string& message);
void log(const std::wstring& message);

void messageBox(const std::string& message);
void messageBox(const std::wstring& message);

void errorBox(const std::string& message);
void errorBox(const std::wstring& message);

::HBRUSH loadBrushFromSystem(int name = BLACK_BRUSH);
::HCURSOR loadCursorFromSystem(LPSTR name = IDC_ARROW);
::HICON loadIconFromSystem(LPSTR name = IDI_APPLICATION);
::HICON loadIconFromResource();

template <typename T>
auto createWindow(std::string_view name, T* self) -> ::HWND
{
  auto className{toUTF16(name)};
  auto hInstance{getInstance()};
  auto iconFromResource{loadIconFromResource()};

  ::WNDCLASSEXW windowClass{.cbSize{sizeof(::WNDCLASSEXW)},
                            .style{0},
                            .lpfnWndProc{self->wndProc},
                            .cbClsExtra{0},
                            .cbWndExtra{sizeof(intptr_t)},
                            .hInstance{hInstance},
                            .hIcon{iconFromResource ? iconFromResource : loadIconFromSystem()},
                            .hCursor{loadCursorFromSystem()},
                            .hbrBackground{loadBrushFromSystem()},
                            .lpszMenuName{nullptr},
                            .lpszClassName{className.c_str()},
                            .hIconSm{iconFromResource ? iconFromResource : loadIconFromSystem()}};

  if (::GetClassInfoExW(hInstance, className.c_str(), &windowClass) == 0)
  {
    ::RegisterClassExW(&windowClass);
  }

  return ::CreateWindowExW(0, className.c_str(), className.c_str(),
                           WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT,
                           CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, self);
}

template <typename T>
T* instanceFromWndProc(HWND hWnd, UINT uMsg, LPARAM lParam)
{
  T* self{nullptr};

  if (uMsg == WM_NCCREATE)
  {
    auto lpCreateStruct{reinterpret_cast<::LPCREATESTRUCTW>(lParam)};
    self = static_cast<T*>(lpCreateStruct->lpCreateParams);
    ::SetWindowLongPtrW(hWnd, 0, reinterpret_cast<intptr_t>(self));
    self->m_hWnd.reset(hWnd);
  }

  else
  {
    self = reinterpret_cast<T*>(::GetWindowLongPtrW(hWnd, 0));
  }

  return self;
}

template <typename R, typename T>
auto checkSafeSize(T value) -> R
{
  constexpr R max{std::numeric_limits<R>::max()};

  if (value > static_cast<T>(max))
  {
    throw std::overflow_error("Unsafe size");
  }

  return static_cast<R>(value);
}

template <typename... Args>
auto log(const std::format_string<Args...> fmt, Args&&... args) -> void
{
  ::OutputDebugStringW(toUTF16(std::vformat(fmt.get(), std::make_format_args(args...))).c_str());
  ::OutputDebugStringW(L"\n");
}

template <typename... Args>
auto log(const std::wformat_string<Args...> fmt, Args&&... args) -> void
{
  ::OutputDebugStringW(std::vformat(fmt.get(), std::make_wformat_args(args...)).c_str());
  ::OutputDebugStringW(L"\n");
}

template <typename... Args>
auto messageBox(const std::format_string<Args...> fmt, Args&&... args) -> void
{
  ::MessageBoxW(nullptr, toUTF16(std::vformat(fmt.get(), std::make_format_args(args...))).c_str(),
                nullptr, MB_OK | MB_ICONASTERISK);
}

template <typename... Args>
auto messageBox(const std::wformat_string<Args...> fmt, Args&&... args) -> void
{
  ::MessageBoxW(nullptr, std::vformat(fmt.get(), std::make_wformat_args(args...)).c_str(), nullptr,
                MB_OK | MB_ICONASTERISK);
}

template <typename... Args>
auto errorBox(const std::format_string<Args...> fmt, Args&&... args) -> void
{
  ::MessageBoxW(nullptr, toUTF16(std::vformat(fmt.get(), std::make_format_args(args...))).c_str(),
                nullptr, MB_OK | MB_ICONHAND);
}

template <typename... Args>
auto errorBox(const std::wformat_string<Args...> fmt, Args&&... args) -> void
{
  ::MessageBoxW(nullptr, std::vformat(fmt.get(), std::make_wformat_args(args...)).c_str(), nullptr,
                MB_OK | MB_ICONHAND);
}
}  // namespace freeaudio::clap_wrapper::standalone::windows::helpers
