#------------------------------------------------------------
#   Summary
#------------------------------------------------------------

# Build a single RowController Target, as specified
# in the 'i2c_protocol' document for this project.
# 
# All RowControllers share identical software but the difference
# is the I2C Slave Address.
#
# So we compile using 'arduino-cli', and #define 'I2C_ADDR'
# using the gcc option:
#   -DI2C_ADDR=<i2c_addr>
#
# This script builds a single target, and moves it to a special
# location (the 'artifacts' folder). The I2C Address is appended
# to the filename.
#
# To build multiple targets, call this script multiple times.

#------------------------------------------------------------
#   Script Begin
#------------------------------------------------------------

NARGS="$#"
APP_NAME="$0"
ADDR="$1"

TARGET="RowControllerBase"
BUILD_DIR="./builds/"
OUT_FILE="${BUILD_DIR}/${TARGET}.ino.elf"

# The executable will be named 'XXXX.ino.elf'
# We want to RENAME to XXXX_<i2c_addr>.inno.elf, and move to the ARTIFACTS directory
# This way, we can build multiple targets, each with different I2C Address
ARTIFACTS_DIR="./artifacts/"
ARTIFACT_FILE="${ARTIFACTS_DIR}/${TARGET}_${ADDR}.ino.elf"

check_params()
{
    if [ $NARGS -ne 1 ]; then
        echo "Usage: $APP_NAME <I2C_ADDR>"
        exit 1
    fi
}

BOARD="-b ATTinyCore:avr:attinyx4"
FLAGS="${FLAGS} --build-property build.extra_flags=-DI2C_ADDR=${ADDR}"
FLAGS="${FLAGS} --build-cache-path ${BUILD_DIR}"
FLAGS="${FLAGS} --build-path ${BUILD_DIR}"
FLAGS="${FLAGS} --verbose"
FLAGS="${FLAGS} --no-color"

#------------------------------------------------------------
#   Script Main
#------------------------------------------------------------

check_params

arduino-cli compile ${BOARD} ${FLAGS} ${TARGET}

cp ${OUT_FILE} ${ARTIFACT_FILE}
avr-objdump -D ${OUT_FILE} > ${ARTIFACT_FILE}.dis
avr-nm -S --size-sort --demangle ${OUT_FILE} > ${ARTIFACT_FILE}.size
