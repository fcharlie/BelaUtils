///
#include "xamldesktop.hpp"

extern "C" IMAGE_DOS_HEADER __ImageBase;

using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::Foundation::Numerics;

namespace xamldesktop {
struct Window : DesktopWindow<Window> {
  Window() noexcept {
    WNDCLASS wc{};
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hInstance = reinterpret_cast<HINSTANCE>(&__ImageBase);
    wc.lpszClassName = L"XamlDesktop";
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    RegisterClass(&wc);
    WINRT_ASSERT(!m_window);

    WINRT_VERIFY(CreateWindowW(wc.lpszClassName, L"XamlDesktop",
                               WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                               CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                               nullptr, nullptr, wc.hInstance, this));

    WINRT_ASSERT(m_window);

    m_manager = InitXaml(m_window, m_rootGrid, m_scale, m_xamlSource);
    NewScale(m_currentDpi);
  }

  ~Window() { m_manager.Close(); }

  Windows::UI::Xaml::Hosting::WindowsXamlManager
  InitXaml(HWND wind, Windows::UI::Xaml::Controls::Grid &root,
           Windows::UI::Xaml::Media::ScaleTransform &dpiScale,
           DesktopWindowXamlSource &source) {

    auto manager = Windows::UI::Xaml::Hosting::WindowsXamlManager::
        InitializeForCurrentThread();

    // Create the desktop source
    DesktopWindowXamlSource desktopSource;
    auto interop = desktopSource.as<IDesktopWindowXamlSourceNative>();
    check_hresult(interop->AttachToWindow(wind));

    // stash the interop handle so we can resize it when the main hwnd is
    // resized
    HWND h = nullptr;
    interop->get_WindowHandle(&h);
    m_interopWindowHandle = h;

    // setup a root grid that will be used to apply DPI scaling
    Windows::UI::Xaml::Media::ScaleTransform dpiScaleTransform;
    Windows::UI::Xaml::Controls::Grid dpiAdjustmentGrid;
    dpiAdjustmentGrid.RenderTransform(dpiScaleTransform);
    Windows::UI::Xaml::Media::SolidColorBrush background{
        Windows::UI::Colors::White()};

    // Set the content of the rootgrid to the DPI scaling grid
    desktopSource.Content(dpiAdjustmentGrid);

    // Update the window size, DPI layout correction
    OnSize(h, dpiAdjustmentGrid, m_currentWidth, m_currentHeight);

    // set out params
    root = dpiAdjustmentGrid;
    dpiScale = dpiScaleTransform;
    source = desktopSource;
    return manager;
  }

  void OnSize(HWND interopHandle,
              winrt::Windows::UI::Xaml::Controls::Grid &rootGrid, UINT width,
              UINT height) {

    // update the interop window size
    SetWindowPos(interopHandle, 0, 0, 0, width, height, SWP_SHOWWINDOW);
    rootGrid.Width(width);
    rootGrid.Height(height);
  }

  LRESULT MessageHandler(UINT const message, WPARAM const wparam,
                         LPARAM const lparam) noexcept {
    // TODO: handle messages here...
    return base_type::MessageHandler(message, wparam, lparam);
  }

  void NewScale(UINT dpi) {

    auto scaleFactor = (float)dpi / 100;

    if (m_scale != nullptr) {
      m_scale.ScaleX(scaleFactor);
      m_scale.ScaleY(scaleFactor);
    }

    ApplyCorrection(scaleFactor);
  }

  void ApplyCorrection(double scaleFactor) {
    double rightCorrection =
        (m_rootGrid.Width() * scaleFactor - m_rootGrid.Width()) / scaleFactor;
    double bottomCorrection =
        (m_rootGrid.Height() * scaleFactor - m_rootGrid.Height()) / scaleFactor;

    m_rootGrid.Padding(Windows::UI::Xaml::ThicknessHelper::FromLengths(
        0, 0, rightCorrection, bottomCorrection));
  }

  void DoResize(UINT width, UINT height) {
    m_currentWidth = width;
    m_currentHeight = height;
    if (nullptr != m_rootGrid) {
      OnSize(m_interopWindowHandle, m_rootGrid, m_currentWidth,
             m_currentHeight);
      ApplyCorrection(m_scale.ScaleX());
    }
  }

  void SetRootContent(Windows::UI::Xaml::UIElement content) {
    m_rootGrid.Children().Clear();
    m_rootGrid.Children().Append(content);
  }

private:
  UINT m_currentWidth = 600;
  UINT m_currentHeight = 600;
  HWND m_interopWindowHandle = nullptr;
  Windows::UI::Xaml::Media::ScaleTransform m_scale{nullptr};
  Windows::UI::Xaml::Controls::Grid m_rootGrid{nullptr};
  DesktopWindowXamlSource m_xamlSource{nullptr};
  Windows::UI::Xaml::Hosting::WindowsXamlManager m_manager{nullptr};
};

Windows::UI::Xaml::UIElement CreateDefaultContent() {

  Windows::UI::Xaml::Media::AcrylicBrush acrylicBrush;
  acrylicBrush.BackgroundSource(
      Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
  acrylicBrush.TintOpacity(0.5);
  acrylicBrush.TintColor(Windows::UI::Colors::Red());

  Windows::UI::Xaml::Controls::Grid container;
  container.Margin(
      Windows::UI::Xaml::ThicknessHelper::FromLengths(100, 100, 100, 100));
  /*container.Background(Windows::UI::Xaml::Media::SolidColorBrush{
   * Windows::UI::Colors::LightSlateGray() });*/
  container.Background(acrylicBrush);

  Windows::UI::Xaml::Controls::Button b;
  b.Width(600);
  b.Height(60);
  b.SetValue(Windows::UI::Xaml::FrameworkElement::VerticalAlignmentProperty(),
             box_value(Windows::UI::Xaml::VerticalAlignment::Center));

  b.SetValue(Windows::UI::Xaml::FrameworkElement::HorizontalAlignmentProperty(),
             box_value(Windows::UI::Xaml::HorizontalAlignment::Center));
  b.Foreground(
      Windows::UI::Xaml::Media::SolidColorBrush{Windows::UI::Colors::White()});

  Windows::UI::Xaml::Controls::TextBlock tb;
  tb.Text(L"Hello Win32 love XAML and C++/WinRT xx");
  b.Content(tb);
  tb.FontSize(30.0f);
  container.Children().Append(b);

  Windows::UI::Xaml::Controls::TextBlock dpitb;
  dpitb.Text(L"(p.s. high DPI just got much easier for win32 devs)");
  dpitb.Foreground(
      Windows::UI::Xaml::Media::SolidColorBrush{Windows::UI::Colors::White()});
  dpitb.Margin(Windows::UI::Xaml::ThicknessHelper::FromLengths(10, 10, 10, 10));
  dpitb.SetValue(
      Windows::UI::Xaml::FrameworkElement::VerticalAlignmentProperty(),
      box_value(Windows::UI::Xaml::VerticalAlignment::Bottom));

  dpitb.SetValue(
      Windows::UI::Xaml::FrameworkElement::HorizontalAlignmentProperty(),
      box_value(Windows::UI::Xaml::HorizontalAlignment::Right));
  container.Children().Append(dpitb);

  return container;
}

} // namespace xamldesktop

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
  init_apartment(apartment_type::single_threaded);

  xamldesktop::Window window;

  auto defaultContent = xamldesktop::CreateDefaultContent();
  window.SetRootContent(defaultContent);

  MSG message;

  while (GetMessage(&message, nullptr, 0, 0)) {
    DispatchMessage(&message);
  }
}
