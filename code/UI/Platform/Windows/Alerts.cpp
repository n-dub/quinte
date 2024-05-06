#include <Core/Platform/Windows/Utils.hpp>
#include <UI/Alerts.hpp>

namespace quinte::alert
{
    static UINT ConvertKind(Kind kind) noexcept
    {
        switch (kind)
        {
        case Kind::Information:
            return MB_ICONINFORMATION;
        case Kind::Warning:
            return MB_ICONWARNING;
        case Kind::Error:
            return MB_ICONERROR;
        case Kind::Question:
            return MB_ICONQUESTION;
        default:
            return MB_ICONINFORMATION;
        }
    }


    static UINT ConvertButtons(Buttons buttons) noexcept
    {
        switch (buttons)
        {
        case Buttons::OK:
            return MB_OK;
        case Buttons::OKCancel:
            return MB_OKCANCEL;
        case Buttons::YesNo:
            return MB_YESNO;
        default:
            return MB_OK;
        }
    }


    Response Display(void* pPlatformWindow, StringSlice text, StringSlice caption, Kind kind, Buttons buttons)
    {
        const windows::WideStr wideText{ text };
        const windows::WideStr wideCaption{ caption };

        UINT type = ConvertKind(kind) | ConvertButtons(buttons);
        if (pPlatformWindow == nullptr)
            type |= MB_TASKMODAL;

        const int response = MessageBoxW(static_cast<HWND>(pPlatformWindow), wideText.Data, wideCaption.Data, type);
        switch (response)
        {
        case IDOK:
            return Response::OK;
        case IDCANCEL:
            return Response::Cancel;
        case IDYES:
            return Response::Yes;
        case IDNO:
            return Response::No;
        default:
            return Response::None;
        }
    }
} // namespace quinte::alert
