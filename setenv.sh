#!/bin/bash
export ENV_OK=y
export TOP_DIR=$PWD
export ECOS_PRODUCT_DIR=$TOP_DIR
export ARCH="arm"
export CROSS_COMPILE="$ECOS_PRODUCT_DIR/toolschain/4.5.1/bin/arm-none-linux-gnueabi-"
export ECOS_CROSS_COMPILE=$CROSS_COMPILE
