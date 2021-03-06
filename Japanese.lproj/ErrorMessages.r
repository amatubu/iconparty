/* ------------------------------------------------------------ */
/*  ErrorMessages.r                                             */
/*     エラーメッセージ（日本語版）                                       */
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
	{"パレットが開けませんでした。致命的エラーです。強制終了します。"},
};

resource 'STR#' (4002,"Dot Palette routine error messages",purgeable)
{
	{
		"名前は半角31文字までです。",
		"同じ名前のライブラリが存在します。",
		"なんらかの原因でライブラリファイルが作成できませんでした。",
	}
};

resource 'STR#' (4003,"Update check routine error messages",purgeable)
{
	{
		"お使いのIconPartyは最新バージョンです。アップデートの必要はありません。",
	}
};

resource 'STR#' (4004,"Preferences routine error messages",purgeable)
{
	{
		"初期設定ファイルを開くことができません。",
		"初期設定ファイルから読み込めませんでした。他のアプリケーションで使用されているのかもしれません。",
		"初期設定が保存できませんでした。",
		"アプリケーションが壊れています。",
	}
};

resource 'STR#' (4005,"PNG routine error messages",purgeable)
{
	{
		"PNG保存失敗（setjmp）。",
		"ファイルが開けませんでした。",
		"ファイルの形式が正しくありません。",
		"PNG読み込み失敗（setjmp）。",
		"イメージのサイズが大きすぎます。",
		"IconPartyではアルファチャンネルをサポートしていませんので削除します。",
		"読み込むためのメモリが足りません。",
		"libpngでエラーが起こりました：",
	}
};

resource 'STR#' (4006,"Paint routine error messages",purgeable)
{
	{
		"処理をするためのメモリが不足しています。",
		"メモリが足りません。取り消しできません。",
	}
};

resource 'STR#' (4007,"Menu routine error messages",purgeable)
{
	{
		"ブレンドパレットファイルの保存に失敗しました。",
		"ブレンドパレットファイルの読み込みに失敗しました。",
	}
};

resource 'STR#' (4008,"Icon Family Window routine error messages",purgeable)
{
	{
		"アイコンファイルを開けません。他のアプリケーションで開かれているのかも知れません。",
		"カスタムアイコンを編集できません。他のアプリケーションで開かれているのかも知れません。",
		"ペーストのためのメモリが足りません。",
		"メモリが足りません。",
		"なんらかの原因で保存に失敗しました。",
		"外部エディタによって編集されているアイコンを閉じることができませんでした。",
	}
};

resource 'STR#' (4009,"IconParty error messages",purgeable)
{
	{
		"このアプリケーションの実行には漢字Talk7以降が必要です。",
		"メモリが足りません。致命的エラーです。強制終了します。",
		"このアプリケーションの実行にはMacOS 8.1以降とCarbonLib 1.0.2以降が必要です。",
	}
};

resource 'STR#' (4010,"File routine error messages",purgeable)
{
	{
		"メモリ不足です。",
		"アイコンファイルが作成できませんでした。",
		"保存に失敗しました。ファイルが使用中かも知れません。",
		"保存に失敗しました。",
		"保存に失敗しました。アイコンが作成できませんでした。",
		"保存に失敗しました。メモリが不足しています。",
		"ファイルが開けませんでした。他のアプリケーションで使用中なのかもしれません。",
		"ファイルが異常です。読み込めませんでした。",
		"ファイルからの読み込みに失敗しました。",
		"ファイルが大きすぎるため、読み込むメモリが足りません。",
		"画像を開くだけのメモリがありません。",
		"エイリアスの元ファイルを見つけることが出来ませんでした。",
	}
};

resource 'STR#' (4011,"Edit routine error messages",purgeable)
{
	{
		"ペーストのためのメモリが足りません。",
		"メモリが足りません。",
	}
};

resource 'STR#' (4012,"AE Install routine error messages",purgeable)
{
	{
		"Gestaltでエラーが発生しました。",
		"現在のシステムではAppleイベントがサポートされていません。",
		"oappイベントハンドラがインストールできませんでした。",
		"odocイベントハンドラがインストールできませんでした。",
		"pdocイベントハンドラがインストールできませんでした。",
		"qappイベントハンドラがインストールできませんでした。",
	}
};

