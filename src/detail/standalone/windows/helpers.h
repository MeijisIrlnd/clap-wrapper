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

namespace freeaudio::clap_wrapper::standalone::windows::helpers::detail
{
struct DefaultWindowProcedure
{
  static auto CALLBACK wndProc(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam) -> ::LRESULT;
};
}  // namespace freeaudio::clap_wrapper::standalone::windows::helpers::detail

namespace freeaudio::clap_wrapper::standalone::windows::helpers
{
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

template <typename T = detail::DefaultWindowProcedure>
auto registerWindowClass(const wchar_t* name, T* self = nullptr) -> const wchar_t*
{
  ::WNDCLASSEXW windowClass{sizeof(::WNDCLASSEXW)};

  auto hInstance{getInstance()};

  if (!::GetClassInfoExW(hInstance, name, &windowClass))
  {
    auto iconFromResource{loadIconFromResource()};

    windowClass.lpszClassName = name;
    windowClass.lpszMenuName = nullptr;
    windowClass.lpfnWndProc = self ? self->wndProc : ::DefWindowProcA;
    windowClass.style = 0;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = sizeof(intptr_t);
    windowClass.hInstance = hInstance;
    windowClass.hbrBackground = loadBrushFromSystem();
    windowClass.hCursor = loadCursorFromSystem();
    windowClass.hIcon = iconFromResource ? iconFromResource : loadIconFromSystem();
    windowClass.hIconSm = iconFromResource ? iconFromResource : loadIconFromSystem();

    auto atom{::RegisterClassExW(&windowClass)};

    if (!atom)
    {
      helpers::errorBox("Window registration failed");
      helpers::abort();
    }
  }

  return name;
}

template <typename T = detail::DefaultWindowProcedure>
auto createWindow(const wchar_t* name = L"Window", T* self = nullptr) -> ::HWND
{
  registerWindowClass(name, self);

  return ::CreateWindowExW(0, name, name, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT,
                           CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, getInstance(),
                           self);
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
