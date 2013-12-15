#!/bin/sh

# アプリケーションをビルド

/Developer/Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild -configuration Deployment

# ドキュメントを RTF 形式に

pandoc -s README.md -o ReadMe.rtf
pandoc -s CHANGES.md -o Changes.rtf

# 古い Zip ファイルを削除

rm iconparty_beta.zip

# リリース用 Zip ファイルを生成

cd build/Deployment
zip -r ../../iconparty_beta.zip IconParty.app
cd ../..
zip -u iconparty_beta.zip ReadMe.rtf Changes.rtf LICENSE

# リリース用 Zip ファイルを公開

cp iconparty_beta.zip ~/Dropbox/Public/
