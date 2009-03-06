/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2006  Roland BROCHARD

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
#include "TA3D_NameSpace.h"
#include "threads/thread.h"
#include "cTA3D_Engine.h"
#include "ta3dbase.h"

#include "3do.h"               // For 3DO/3DM management
#include "scripts/cob.h"               // For unit scripts management
#include "tdf.h"               // For 2D features
#include "EngineClass.h"         // The Core Engine

#include "UnitEngine.h"            // The Unit Engine

#include "tnt.h"               // The TNT loader
#include "scripts/script.h"               // The game script manager
#include "ai/ai.h"                  // AI Engine
#include "gfx/fx.h"					// Special FX engine
#include "misc/paths.h"
#include "languages/i18n.h"
#include "sounds/manager.h"
#include "misc/math.h"





namespace TA3D
{

	namespace
	{
		void showError(const String& s, const String& additional = String())
		{
			LOG_ERROR(I18N::Translate(s));
//			allegro_message((String(I18N::Translate(s)) << additional).c_str());
#warning FIXME: ugly print to console instead of a nice window
            std::cerr << I18N::Translate(s) << additional << std::endl;
		}

	}


	cTA3D_Engine::cTA3D_Engine(void)
	{
		InterfaceManager = NULL;
		VARS::sound_manager = NULL;
		VARS::HPIManager = NULL;
		m_SDLRunning = false;
		VARS::gfx = NULL;
		m_SignaledToStop = false;
		m_GFXModeActive = false;

		InterfaceManager = new IInterfaceManager();

		String str = format( "%s initializing started:\n\n", TA3D_ENGINE_VERSION );
		I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void *)str.c_str(), NULL, NULL );

		str = format("build info : %s , %s\n\n",__DATE__,__TIME__);
		I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void *)str.c_str(), NULL, NULL );

		// Initalizing SDL video
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
			throw ("SDL_Init(SDL_INIT_VIDEO) yielded unexpected result.");

		// set SDL running status;
		m_SDLRunning = true;

		// Installing SDL timer
		if (SDL_InitSubSystem(SDL_INIT_TIMER) != 0)
			throw ("SDL_InitSubSystem(SDL_INIT_TIMER) yielded unexpected result.");

		// Creating HPI Manager
		TA3D::VARS::HPIManager = new TA3D::UTILS::HPI::cHPIHandler();

		// Creating translation manager
        I18N::Instance()->loadFromFile("gamedata\\translate.tdf", true, true);
        I18N::Instance()->loadFromFile("ta3d.res", false);   // Loads translation data (TA3D translations in UTF8)

		if (!HPIManager->Exists("gamedata\\sidedata.tdf") || !HPIManager->Exists("gamedata\\allsound.tdf") || !HPIManager->Exists("gamedata\\sound.tdf"))
		{
			showError("RESOURCES ERROR");
			exit(1);
		}

		// Creating Sound & Music Interface
		sound_manager = new TA3D::Audio::Manager();
		sound_manager->loadTDFSounds(true);
		sound_manager->loadTDFSounds(false);

		if (!sound_manager->isRunning() && !lp_CONFIG->quickstart)
			showError("FMOD WARNING");

		// Creating GFX Interface
        // Don't try to start sound before gfx, if we have to display the warning message while in fullscreen
		TA3D::VARS::gfx = new TA3D::GFX();		// TA3D's main window might lose focus and message may not be shown ...
		m_GFXModeActive = true;

		gfx->Init();

		SDL_WM_SetCaption("Total Annihilation 3D","TA3D");

        init_mouse();

        init_keyboard();

		ThreadSynchroniser = new ObjectSync;
	}


	cTA3D_Engine::~cTA3D_Engine(void)
	{
		destroyThread();
		delete ThreadSynchroniser;
        cursor.clear();
		ta3dSideData.destroy();

		delete HPIManager;
		delete sound_manager;

		delete gfx;
		m_GFXModeActive = false;

		delete InterfaceManager;

		if (m_SDLRunning)
		{
			SDL_Quit();
			m_SDLRunning = false;
		}
	}

	void cTA3D_Engine::Init()
	{
		model_manager.init();
		unit_manager.init();
		feature_manager.init();
		weapon_manager.init();
		fx_manager.init();
        Math::InitializeRandomTable();

		ta3dSideData.init();
		ta3dSideData.loadData();

		sound_manager->loadTDFSounds(false);
	}


	void cTA3D_Engine::proc(void*)
	{
		Init();
		while( !m_SignaledToStop )
			rest( 100 );
	}
	void cTA3D_Engine::signalExitThread()
	{
		m_SignaledToStop = true;
		return;
	}


} // namespace TA3D
