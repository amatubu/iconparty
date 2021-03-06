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


resource 'vers' (1,purgeable)
{
	0x01, /* major revision */
	0x30, /* minor revision */
	beta, /* release stage */
	0x00, /* non-final release # */
	verJapan,
	"1.30b3",
	"1.30b3 (c) naoki iimura, 1997-2014"
};

resource 'vers' (2,purgeable)
{
	0x01, /* major revision */
	0x30, /* minor revision */
	beta,
	0x00,
	verJapan,
	"1.30b3",
	"256 colors painting tool & icon editor"
};

resource 'kind' (1000,purgeable)
{
	'Ipty',verJapan,
	{
		'apnm',"IconParty",
		'PICT',"IconParty PICT image",
		'GIFf',"IconParty GIF image",
		'PNGf',"IconParty PNG image",
		'rsrc',"IconParty resource file",
		'icns',"IconParty Icon file(for MacOS X)",
		'ICO ',"IconParty Icon file(for Windows)",
		'fold',"folder",
		'disk',"volume"
	}
};

resource 'STR#' (5001,purgeable)
{
	{
		"アップデートチェック",
	}
};
