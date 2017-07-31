#include <boost/asio.hpp>
#include <iostream>
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <stdio.h>
#include <fmt/format.h>

#include <windows.h>
#include <stdio.h>

HANDLE hStdin; 
DWORD fdwSaveOldMode;

VOID ErrorExit(LPSTR);
VOID KeyEventProc(KEY_EVENT_RECORD); 
VOID MouseEventProc(MOUSE_EVENT_RECORD); 
VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD); 
 
int main(VOID) 
{ 
    DWORD cNumRead, fdwMode, i; 
    INPUT_RECORD irInBuf[128]; 
    int counter=0;
 
    // Get the standard input handle. 
 
    hStdin = GetStdHandle(STD_INPUT_HANDLE); 
    if (hStdin == INVALID_HANDLE_VALUE) 
        ErrorExit("GetStdHandle"); 
 
    // Save the current input mode, to be restored on exit. 
 
    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) ) 
        ErrorExit("GetConsoleMode"); 

    // Enable the window and mouse input events. 
 
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT; 
    if (! SetConsoleMode(hStdin, fdwMode) ) 
        ErrorExit("SetConsoleMode"); 
 
    // Loop to read and handle the next 100 input events. 
 
    while (counter++ <= 100) 
    { 
        // Wait for the events. 
 
        if (! ReadConsoleInput( 
                hStdin,      // input buffer handle 
                irInBuf,     // buffer to read into 
                128,         // size of read buffer 
                &cNumRead) ) // number of records read 
            ErrorExit("ReadConsoleInput"); 
 
        // Dispatch the events to the appropriate handler. 
 
        for (i = 0; i < cNumRead; i++) 
        {
            switch(irInBuf[i].EventType) 
            { 
                case KEY_EVENT: // keyboard input 
                    KeyEventProc(irInBuf[i].Event.KeyEvent); 
                    break; 
 
                case MOUSE_EVENT: // mouse input 
                    MouseEventProc(irInBuf[i].Event.MouseEvent); 
                    break; 
 
                case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing 
                    ResizeEventProc( irInBuf[i].Event.WindowBufferSizeEvent ); 
                    break; 
 
                case FOCUS_EVENT:  // disregard focus events 
 
                case MENU_EVENT:   // disregard menu events 
                    break; 
 
                default: 
                    ErrorExit("Unknown event type"); 
                    break; 
            } 
        }
    } 

    // Restore input mode on exit.

    SetConsoleMode(hStdin, fdwSaveOldMode);
 
    return 0; 
}

VOID ErrorExit (LPSTR lpszMessage) 
{ 
    fprintf(stderr, "%s\n", lpszMessage); 

    // Restore input mode on exit.

    SetConsoleMode(hStdin, fdwSaveOldMode);

    ExitProcess(0); 
}

VOID KeyEventProc(KEY_EVENT_RECORD ker)
{
    printf("Key event: ");

    if(ker.bKeyDown)
        printf("key pressed\n");
    else printf("key released\n");
}

VOID MouseEventProc(MOUSE_EVENT_RECORD mer)
{
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif
    printf("Mouse event: ");
    
    switch(mer.dwEventFlags)
    {
        case 0:

            if(mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
            {
                printf("left button press \n");
            }
            else if(mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
            {
                printf("right button press \n");
            }
            else
            {
                printf("button press\n");
            }
            break;
        case DOUBLE_CLICK:
            printf("double click\n");
            break;
        case MOUSE_HWHEELED:
            printf("horizontal mouse wheel\n");
            break;
        case MOUSE_MOVED:
            printf("mouse moved\n");
            break;
        case MOUSE_WHEELED:
            printf("vertical mouse wheel\n");
            break;
        default:
            printf("unknown\n");
            break;
    }
}

VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD wbsr)
{
    printf("Resize event\n");
    printf("Console screen buffer is %d columns by %d rows.\n", wbsr.dwSize.X, wbsr.dwSize.Y);
}

// int main()
// {
//     // STARTUPINFO info;
//     // GetStartupInfo(&info);
//     // fmt::print("hStdInput: {}\n", info.hStdInput);
//     // fmt::print("hStdOutput: {}\n", info.hStdOutput);
//     // fmt::print("hStdError: {}\n", info.hStdError);
//     // auto herror = GetStdHandle(STD_ERROR_HANDLE);
//     // auto houtput = GetStdHandle(STD_OUTPUT_HANDLE);
//     // fmt::print("hinput: {}\n", hinput);
//     // fmt::print("houtput: {}\n", houtput);
//     // fmt::print("herror: {}\n", herror);
//     auto hinput  = GetStdHandle(STD_INPUT_HANDLE);
//     boost::asio::io_service ios;
//     boost::asio::windows::object_handle conin(ios,hinput);
//     fmt::print("is_open: {}\n", conin.is_open());
//     fmt::print("native_handle: {}\n", conin.native_handle());
//     // boost::asio::streambuf buf;
//     // std::istream is(&buf);
//     // while (true) {
//         boost::system::error_code ec;
//         // conin.wait(ec);
//         auto r = ::WaitForSingleObject(hinput, INFINITE);
//         switch (r) {
//         case WAIT_FAILED:
//             ec = boost::system::error_code(
//                 ::GetLastError(),
//                 boost::asio::error::get_system_category()
//                 );
//             break;
//         default:
//             ec = boost::system::error_code();
//             break;
//         }
//         fmt::print("result: {}\n", r);
//         fmt::print("value: {}\n", ec.value());
//         // if (ec) return 0;
//         // auto n = boost::asio::read_until(conin,buf,'\n',ec);
//         // std::string line;
//         // std::getline(is, line);
//         // fmt::print("line: {}", line);
//     // }
//     return 0;
// }

// int main2(void) 
// { 
//    CHAR chBuf[BUFSIZE]; 
//    DWORD dwRead, dwWritten; 
//    HANDLE hStdin, hStdout; 
//    BOOL bSuccess; 
 
//    hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
//    hStdin = GetStdHandle(STD_INPUT_HANDLE); 
//    if ( 
//        (hStdout == INVALID_HANDLE_VALUE) || 
//        (hStdin == INVALID_HANDLE_VALUE) 
//       ) 
//       ExitProcess(1); 
 
//    // Send something to this process's stdout using printf.
//    printf("\n ** This is a message from the child process. ** \n");

//    // This simple algorithm uses the existence of the pipes to control execution.
//    // It relies on the pipe buffers to ensure that no data is lost.
//    // Larger applications would use more advanced process control.

//    for (;;) 
//    { 
//    // Read from standard input and stop on error or no data.
//       bSuccess = ReadFile(hStdin, chBuf, BUFSIZE, &dwRead, NULL); 
      
//       if (! bSuccess || dwRead == 0) 
//          break; 
 
//    // Write to standard output and stop on error.
//       bSuccess = WriteFile(hStdout, chBuf, dwRead, &dwWritten, NULL); 
      
//       if (! bSuccess) 
//          break; 
//    } 
//    return 0;
// }


// struct console
// {
//     std::array<HANDLE,2> handles;
//     console();
//     ~console();
//     void cancel();
//     void run();
// };

// void console::console()
//     : m_hstdin(nullptr)
//     , m_hcancel(nullptr)
//     , m_hread(nullptr)
// {}

// void console::run()
// {
//     while (true) {
//         DWORD ret;
//         HANDLE handles[] = {h_cancel, h_read};
//         enum {
//             event_cancel = WAIT_OBJECT_0 + 0,
//             event_stop   = WAIT_OBJECT_0 + 0,
//             event_read   = WAIT_OBJECT_0 + 1,
//         };
//         ret = ::WaitForMultipleObjects(2, handles, FALSE, INFINITE);
//         switch (ret) {
//         case event_stop:
//             return;
//         case event_read:
//         default:
            
//         }
//     }
// }
