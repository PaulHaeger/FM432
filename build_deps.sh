#! /bin/sh


export PLATFORM=msp432
export YAHAL_DIR="$PWD/YAHAL"
export TOOLCHAIN_PATH=/usr/
cd $YAHAL_DIR
FILE="makefiles/platform-msp432.mk"
cp "$FILE" backup.mk
sed -i 's/$(CCS_ROOT)\/tools\/compiler\/gcc-arm-none-eabi-9-2019-q4-major/\/usr\//' $FILE
make $@
mv backup.mk $FILE
