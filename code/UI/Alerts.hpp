#pragma once
#include <Core/Core.hpp>
#include <Core/StringSlice.hpp>

namespace quinte::alert
{
    enum class Kind
    {
        Information,
        Warning,
        Error,
        Question,
    };


    enum class Buttons
    {
        OK,
        OKCancel,
        YesNo,
    };


    enum class Response
    {
        None,
        OK,
        Cancel,
        Yes,
        No,
    };


    //! \brief Display a message box.
    //!
    //! \param pPlatformWindow - Platform window handle, used by WinAPI.
    //! \param text - Main message text.
    //! \param caption - Caption text.
    //! \param kind - Kind - Information, Warning, Error or Question.
    //! \param buttons - Buttons to show - OK, OKCancel or YesNo.
    //!
    //! \return Response - OK, Cancel, Yes or No.
    Response Display(void* pPlatformWindow, StringSlice text, StringSlice caption, Kind kind, Buttons buttons);
} // namespace quinte::alert
