/*-------------------------------------------------------------------------------

	BARONY
	File: messages.hpp
	Desc: defines stuff for messages that draw onto the screen and then
	fade away after a while.

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#pragma once

//#include "SDL.h"

#include "main.hpp"
#include "interface/interface.hpp"

/*
 * Right, so this is how it's going to work:
 * This is a "class" to emulate a virtual console -- minecraft style. I mean, message log, not console.
 * It draws messages up above the main bar and stuff. It...ya. Minecraft messages pop up, dissapear, you know?
 * This is what that does.
 */

typedef struct Message
{
	string_t* text; //Same size as the message in draw.c. Make sure not to overrun it.

	//The time it's been displayed so far.
	int time_displayed;

	//The alpha of the message (SDL > 1.1.5, or whatever version it was, has 255 as SDL_ALPHA_OPAQUE and 0 as ASL_ALPHA_TRANSPARENT).
	/*
	 * Building on that last point, we could probably:
		if (SDL_ALPHA_TRANSPARENT < SDL_ALPHA_OPAQUE)
		{
			alpha--;
		}
		else
		{
			alpha++;
		}
	 * To ensure everything always works right. I guess. Maybe not necessary. Whatever. There are much bigger problems to worry about.
	 */
	Sint16 alpha;

	static const int CHAT_MESSAGE_SFX = 238;
} Message;

/*
* Remove single % from message strings.
*/
std::string messageSanitizePercentSign(std::string src, int* percentSignsFound);

extern const int MESSAGE_LIST_SIZE_CAP;