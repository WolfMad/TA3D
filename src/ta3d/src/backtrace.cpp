/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/


#include "stdafx.h"
#include <iostream>
#include "misc/string.h"


// Signals should be disabled under OS X, since the system already produces a crash report
// More information are available here :
// http://developer.apple.com/technotes/tn2004/tn2123.html
#ifndef TA3D_PLATFORM_DARWIN


# include "TA3D_NameSpace.h"
# include "misc/paths.h"
# include "gfx/gui/area.h"
# include "backtrace.h"
# include "network/socket.tcp.h"

# ifdef TA3D_PLATFORM_LINUX
#	define TA3D_BUILTIN_BACKTRACE_SUPPORT
# endif

# ifdef TA3D_BUILTIN_BACKTRACE_SUPPORT
#	include <execinfo.h>
# endif

# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <yuni/core/io/file/stream.h>
# include "sdl.h"
# include <SDL/SDL_sgui.h>

/*! \brief a small function implementing an automatic bug reporter
 * Declaration of the bug reporter. It's here because it should be visible only
 * from this module.
 */
void bug_reporter();

/*!
 * \brief Obtain a backtrace and print it to stdout.
 *
 * If GDB can be used to get a backtrace then we use it, otherwise and only
 * if TA3D_BUILTIN_BACKTRACE_SUPPORT is defined, a backtrace is obtained
 * then writen into a log file. It will be displayed in stdout when gdb is missing.
 * After this call, the program will exit with a exit status code equals
 * to `-1`.
 *
 * \param signum Which signal was received
 */
void backtrace_handler (int signum)
{
	// Some functions called at exit may crash, so we must disable signals in order
	// to prevent overwriting a useful log
	clear_signals();

	// Get TA3D's PID
	pid_t mypid = getpid();
	// Try to get a stack trace from GDB
	String cmd;
	cmd << "gdb --pid=" << mypid << " -ex \"info threads\" -ex bt > \"" << TA3D::Paths::Logs << "backtrace.txt\" --batch";
	if (system(cmd.c_str()) == 0)
	{
		bug_reporter();
		exit(-1);
	}

	// If GDB is not available or returned an error we must find another way ... this is now platform dependent

# ifdef TA3D_BUILTIN_BACKTRACE_SUPPORT
	// Retrieving a backtrace
	void *array[400];
	size_t size = backtrace (array, 400);
	char** strings = backtrace_symbols(array, size);

    // Try to log it
	Yuni::Core::IO::File::Stream m_File(TA3D::Paths::Logs + "backtrace.txt", Yuni::Core::IO::OpenMode::write);
	if(m_File.opened())
    {
		m_File << "received signal " << strsignal( signum ) << "\n";
		m_File << "Obtained " << size << " stack frames.\n";
		for (TA3D::uint32 i = 0; i < size; ++i)
			m_File << strings[i] << "\n";
		m_File.flush();
		m_File.close();

		printf("received signal %s\n", strsignal( signum ));
		printf ("Obtained %zd stack frames.\n", size);
		for (TA3D::uint32 i = 0; i < size; ++i)
			printf ("%s\n", strings[i]);

        String szErrReport = "An error has occured.\nDebugging information have been logged to:\n"
            + TA3D::Paths::Logs
            + "backtrace.txt\nPlease report to our forums (http://www.ta3d.org/)\nand keep this file, it'll help us debugging.\n";

		criticalMessage(szErrReport);
	}
	else
    {
        // The file is not opened
        // The backtrace will be directly to stdout instead.
		printf("received signal %s\n", strsignal(signum));
		printf("couldn't open file for writing!!\n");
		printf ("Obtained %zd stack frames.\n", size);
		for (TA3D::uint32 i = 0; i < size; ++i)
			printf ("%s\n", strings[i]);
	}
	free(strings);

	# else // ifdef TA3D_BUILTIN_BACKTRACE_SUPPORT

        // The backtrace support is disabled: warns the user
		String szErrReport = "An error has occured.\nDebugging information could not be logged.\nPlease report to our forums (http://www.ta3d.org/) so we can fix it.";
		criticalMessage(szErrReport);

	# endif // ifdef TA3D_BUILTIN_BACKTRACE_SUPPORT
	exit(-1);
}

#endif // ifdef TA3D_PLATFORM_DARWIN





int init_signals (void)
{
	// Signals should be disabled under OS X, since the system already produces a crash report
	// More information are available here :
	// http://developer.apple.com/technotes/tn2004/tn2123.html
	#ifndef TA3D_PLATFORM_DARWIN

	# ifdef TA3D_PLATFORM_WINDOWS
		int signum[] = { SIGFPE, SIGILL, SIGSEGV, SIGABRT };
		int nb_signals = 4;
	# else
		int signum[] = { SIGFPE, SIGILL, SIGSEGV, SIGBUS, SIGABRT, SIGIOT, SIGTRAP, SIGSYS };
		int nb_signals = 8;
	# endif // ifdef TA3D_PLATFORM_WINDOWS
	for (int i = 0; i < nb_signals; ++i)
	{
		if (signal (signum[i], backtrace_handler) == SIG_IGN)
			signal (signum[i], SIG_IGN);
	}

	#endif // ifdef TA3D_PLATFORM_DARWIN
	return 0; // TODO missing value ?
}

void clear_signals (void)
{
	// Signals should be disabled under OS X, since the system already produces a crash report
	// More information are available here :
	// http://developer.apple.com/technotes/tn2004/tn2123.html
	#ifndef TA3D_PLATFORM_DARWIN

	# ifdef TA3D_PLATFORM_WINDOWS
		int signum[] = { SIGFPE, SIGILL, SIGSEGV, SIGABRT };
		int nb_signals = 4;
	# else
		int signum[] = { SIGFPE, SIGILL, SIGSEGV, SIGBUS, SIGABRT, SIGIOT, SIGTRAP, SIGSYS };
		int nb_signals = 8;
	# endif // ifdef TA3D_PLATFORM_WINDOWS
	for (int i = 0; i < nb_signals; ++i)
		signal (signum[i], SIG_IGN);

	#endif // ifdef TA3D_PLATFORM_DARWIN
}


void criticalMessage(const String &msg)
{
	std::cerr << msg << std::endl;      // Output error message to stderr

	Gui::Utils::message("TA3D - Critical Error", msg.c_str());
}

/*!
 * \brief Display a window presenting the crash report to the user
 * The crash report window tells the user something went wrong. It shows the content of the
 * crash report that would be sent to the bug report server. The user can accept to send the
 * report or not.
 */
void bug_reporter()
{
	bool bSendReport = false;
	std::string report;

	// Engine version
	report += TA3D_ENGINE_VERSION;
#ifdef TA3D_CURRENT_REVISION
	report += " r";
	report += TA3D_CURRENT_REVISION;
#endif
	report += '\n';

	// Current mod
	if (!TA3D_CURRENT_MOD.empty())
	{
		report += "MOD: ";
		report += TA3D_CURRENT_MOD.c_str();
		report += '\n';
	}

	// System info
	report += "\nSystem info:\n";
#ifdef TA3D_PLATFORM_DARWIN
	report += "OS: darwin\n";
#elif defined TA3D_PLATFORM_LINUX
	report += "OS: linux\n";
#elif defined TA3D_PLATFORM_WINDOWS
	report += "OS: windows\n";
#else
	report += "OS: unknown\n";
#endif

	// OpenGL info
	report += "OpenGL Informations :\n";
	(report += "Vendor: ") += (const char*) glGetString(GL_VENDOR);
	(report += "\nRenderer: ") += (const char*) glGetString(GL_RENDERER);
	(report += "\nVersion: ") += (const char*) glGetString(GL_VERSION);
	report += "\nExtensions:\n";
	const char *ext = (const char*) glGetString(GL_EXTENSIONS);
	for(; *ext ; ++ext)
		report += *ext == ' ' ? '\n' : *ext;
	report += '\n';
	report += '\n';

	const String fBacktrace = TA3D::Paths::Logs + "backtrace.txt";
	report += "\nstacktrace:\n";
	std::string bt;
	Yuni::Core::IO::File::LoadFromFile(bt, fBacktrace);

	report += bt;

	Gui::Window wnd("Bug report", 640, 240, Gui::Window::MOVEABLE);
	wnd.addChild(Gui::TabWidget_("tabs")
				/ (Gui::Spacer_(false) | Gui::Button_("ok", " send report ") | Gui::Spacer_(false) | Gui::Button_("cancel", " don't send ") | Gui::Spacer_(false)));

	TABWIDGET(tabs)->addTab("info", Gui::Label_("info")
									/ Gui::Spacer_(true)
									/ Gui::Label_("size"));
	TABWIDGET(tabs)->addTab("report", Gui::ScrollArea_("scroll"));

	SCROLLAREA(scroll)->setCentralWidget(Gui::Label_("text", report));

	BUTTON(ok)->addListener(Gui::Utils::actionSetBool(bSendReport));
	BUTTON(ok)->addListener(Gui::Utils::actionCloseWindow());
	BUTTON(cancel)->addListener(Gui::Utils::actionCloseWindow());

	LABEL(info)->setCaption("An error has occured.\n"
							"A bug report has been prepared. You can review it in the 'report' tab.\n"
							"It contains information about your version of TA3D, OS, OpenGL renderer\n"
							"and a stack trace to help us find what's wrong.\n"
							"\n"
							"Do you want to send the bug report ?");
	String buf;
	buf << "(report size = " << report.size() << " bytes)";
	LABEL(size)->setCaption(buf.c_str());

	wnd();

	if (!bSendReport)
		return;

	// Send the bug report to the bug server
	TA3D::SocketTCP sock;
	sock.open("bugs.ta3d.org", 1905);
	if (!sock.isOpen())
	{
		Gui::Utils::message("Socket error", "Error: could not connect to server.");
		return;
	}
	sock.send("BUG REPORT\n");
	sock.send(report);
	sock.send("DISCONNECT");
	sock.close();
	Gui::Utils::message("Success", "Bug report has been sent to server.");
}
