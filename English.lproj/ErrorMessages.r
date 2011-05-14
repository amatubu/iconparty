/* ------------------------------------------------------------ */
/*  ErrorMessages.r                                                 */
/*     error messages (english version)                        */
/*                                                              */
/*                 2002.6.9 - 2002.6.9  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include "Types.r"
#include "Balloons.r"
#include "AEUserTermTypes.r"
#include "AERegistry.r"
#include "AEDataModel.r"
#include "Appearance.r"
#include "Dialogs.r"
#include "ConditionalMacros.r"
#else
#include "Carbon.r"
#endif

resource 'STR#' (4001,"Window routine error messages",purgeable)
{
	{
		"Can't open the Palettes. Quit."
	},
};

resource 'STR#' (4002,"Dot Palette routine error messages",purgeable)
{
	{
		"Name is too long.",
		"A library which has same name already exists.",
		"An error was occurred in creating library.",
	}
};

resource 'STR#' (4003,"Update check routine error messages",purgeable)
{
	{
		"Your IconParty is Latest version. You need not to update.",
	}
};

resource 'STR#' (4004,"Preferences routine error messages",purgeable)
{
	{
		"Can't open the preference file.",
		"Can't open the preference file. Probably the file is opened by another application.",
		"Can't save to the preference file.",
		"The application file is damaged.",
	}
};

resource 'STR#' (4005,"PNG routine error messages",purgeable)
{
	{
		"Failed in saving as PNG.",
		"Failed in opening file.",
		"Invalid file format.",
		"Failed in opening PNG.",
		"Image size is too large.",
		"IconParty does not support alpha-channel.",
		"Out of memory in opening file.",
		"An error occured in libpng : ",
	}
};

resource 'STR#' (4006,"Paint routine error messages",purgeable)
{
	{
		"Out of memory.",
		"Out of memory. Can't undo.",
	}
};

resource 'STR#' (4007,"Menu routine error messages",purgeable)
{
	{
		"Can't save to the blend file.",
		"Can't open the blend file.",
	}
};

resource 'STR#' (4008,"Icon Family Window routine error messages",purgeable)
{
	{
		"Can't open the icon file. Probably the file is opened by another application.",
		"Can't edit the custom icon. Probably the file is opened by another application.",
		"Can't paste. Out of memory.",
		"Out of memory.",
		"An error occurred in saving to file.",
		"Can't close the icon which by edited by the external editor.",
	}
};

resource 'STR#' (4009,"IconParty error messages",purgeable)
{
	{
		"This application requires System 7 or later.",
		"Out of memory. Quit.",
		"This application requires MacOS 8.1 or later and CarbonLib 1.0.2 or later.",
	}
};

resource 'STR#' (4010,"File routine error messages",purgeable)
{
	{
		"Out of memory.",
		"Can't make icon file(resource file).",
		"Failed in saving. Probably the file is opened by another application.",
		"Failed in saving.",
		"Failed in saving. Can't make icon.",
		"Failed in saving. Out of memory.",
		"Can't open the file. Probably the file is opened by anothor application.",
		"Can't open the file. Invalid file format.",
		"Can't open the file.",
		"Can't open the file. Out of memory.",
		"Memory is short to opeing the file.",
		"Can't fild original file of the alias.",
	}
};

resource 'STR#' (4011,"Edit routine error messages",purgeable)
{
	{
		"Memory is short to paste picture.",
		"Out of memory.",
	}
};

resource 'STR#' (4012,"AE Install routine error messages",purgeable)
{
	{
		"An error occurred in Gestalt.",
		"Current System does not support AppleEvents.",
		"Failed to install 'oapp' event handler.",
		"Failed to install 'odoc' event handler.",
		"Failed to install 'pdoc' event handler.",
		"Failed to install 'qapp' event handler.",
	}
};

