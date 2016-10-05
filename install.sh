EXE_PATH=`grep PATCH_TARGET_PATH config.h|sed 's/#define[\t ]*//g'|sed 's/PATCH_TARGET_PATH[\t ]*//g'|sed 's/"//g'`
EXE_NAME=`grep PATCH_TARGET_EXE config.h|sed 's/#define[\t ]*//g'|sed 's/PATCH_TARGET_EXE[\t ]*//g'|sed 's/"//g'`

rm -f "$EXE_PATH/libpatcher.dylib"

cp libpatcher.dylib "$EXE_PATH/libpatcher.dylib"

if [ ! -x "$EXE_PATH/$EXE_NAME""_" ]; then
	mv "$EXE_PATH/$EXE_NAME" "$EXE_PATH/$EXE_NAME""_"
fi

if [ ! -x "$EXE_PATH/$EXE_NAME" ]; then
	cp Launcher "$EXE_PATH/$EXE_NAME"
fi
