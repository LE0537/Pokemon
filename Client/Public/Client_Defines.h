#pragma once

namespace Client
{
	const unsigned int g_iWinSizeX = 1280;
	const unsigned int g_iWinSizeY = 720;

	enum LEVEL { LEVEL_STATIC, LEVEL_LOADING, LEVEL_LOGO, LEVEL_GAMEPLAY, LEVEL_END };
	enum POKETYPE { NORMAL, FIRE, WATER, THUNDER, GRASS, ICE, FIGHT, POISON, TERRAIN, FLY, ESPER, BUG, ROCK, GHOST, DRAGON, EVIL, STEEL, FAIRY, POKETYPE_END };
	enum STATINFO {STUN,SHOCK,FREEZ,BURN,POISONING,SLEEP,STATINFO_END};


}

#include <process.h>
#include <string>
// Client_Defines.h

extern HWND			g_hWnd;
extern HINSTANCE	g_hInst;
extern bool		g_bBag;
extern bool		g_PokeInfo;
extern bool		g_bPokeDeck;
extern bool     g_CollBox;
extern bool		g_Battle;

using namespace Client;