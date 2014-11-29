#!/bin/bash
# ##########################################################
# ALPS(Android4.1 based) build environment profile setting
# ##########################################################

# Overwrite ANDROID_JAVA_HOME environment variable setting if already exists
ANDROID_JAVA_HOME=$JAVA_HOME
export ANDROID_JAVA_HOME

# Overwrite PATH environment setting for JDK & arm-eabi if already exists
PATH=$JAVA_HOME/bin:$PWD/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.7/bin:$PWD/prebuilts/gcc/linux-x86/arm/arm-eabi-4.7/bin:$PATH
export PATH

# Add MediaTek developed Python libraries path into PYTHONPATH
if [ -z "$PYTHONPATH" ]; then
  PYTHONPATH=$PWD/mediatek/build/tools
else
  PYTHONPATH=$PWD/mediatek/build/tools:$PYTHONPATH
fi
export PYTHONPATH

# MTK customized functions
# Adatping MTK build system with custmer(L)'s setting
function m()
{
    export MY_BUILD_CMD="m $@"
    local HERE=$PWD
    T=$(gettop)
    if [ "$T" ]; then
        local LOG_FILE=$T/build_log.txt
        cd $T
        time ./makeMtk -t -o=TARGET_BUILD_VARIANT=$TARGET_BUILD_VARIANT,TARGET_BUILD_TYPE=$TARGET_BUILD_TYPE,$@ \
            $TARGET_PRODUCT r | tee -a $LOG_FILE
        if [ ${PIPESTATUS[0]} -ne 0 ]; then
            cd $HERE > /dev/null
            echo -e "\nBuild log was written to '$LOG_FILE'."
            echo "Error: Building failed"
            return 1
        fi
        cd $HERE > /dev/null
        echo -e "\nBuild log was written to '$LOG_FILE'."
    else
        echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
}

#EMAKE start
if [ ! "$JAVA_HOME" ]; then
    export JAVA_HOME=/usr/lib/jvm/java-6-oracle
fi
export ECLOUD_BIN=/opt/ecloud/i686_Linux/64/bin
export PATH=$ECLOUD_BIN:$PATH

function EMAKE()
{
    export "EMAKEFLAGS=--emake-build-label=mt6577_${TARGET_PRODUCT}_android --emake-class=U1004_ICS --emake-autodepend=1 --emake-root=../:/usr/:/etc/alternatives/:/opt/bms/bin/ --emake-cm=10.168.176.194 --emake-emulation=gmake3.81 --emake-historyfile=./build/mt6577_android.hist --emake-annofile=mt6577_${TARGET_PRODUCT}_@ECLOUD_BUILD_ID@.anno --emake-annodetail=basic --emake-optimize-deps=1 --emake-parse-avoidance=1 --emake-suppress-include=*.d --emake-suppress-include=*.P"
}

function em()
{
    export MY_BUILD_CMD="em $@"
    local HERE=$PWD
    EMAKE
    T=$(gettop)
    if [ "$T" ]; then
        local LOG_FILE=$T/build_log.txt
        cd $T
        time ./emakeMtk -t -o=TARGET_BUILD_VARIANT=$TARGET_BUILD_VARIANT,TARGET_BUILD_TYPE=$TARGET_BUILD_TYPE,$@ \
            $TARGET_PRODUCT r | tee -a $LOG_FILE
        if [ ${PIPESTATUS[0]} -ne 0 ]; then
            cd $HERE > /dev/null
            echo -e "\nE-Build log was written to '$LOG_FILE'."
            echo "Error: E-Building failed"
            return 1
        fi
        cd $HERE > /dev/null
        echo -e "\nE-Build log was written to '$LOG_FILE'."
    else
        echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
}

function mm()
{
    local HERE=$PWD
    # If we're sitting in the root of the build tree, just do a
    # normal make.
    if [ -f build/core/envsetup.mk -a -f Makefile ]; then
        ./makeMtk -t -o=TARGET_BUILD_VARIANT=$TARGET_BUILD_VARIANT,TARGET_BUILD_TYPE=$TARGET_BUILD_TYPE,$@ \
            $TARGET_PRODUCT new
    else
        # Find the closest Android.mk file.
        T=$(gettop)
        local M=$(findmakefile)
        # Remove the path to top as the makefilepath needs to be relative
        local M=`echo $M|sed 's:'$T'/::'`
        local P=`echo $M|sed 's:'/Android.mk'::'`
        if [ ! "$T" ]; then
            echo "Couldn't locate the top of the tree. Try setting TOP."
        elif [ ! "$M" ]; then
            echo "Couldn't locate a makefile from the current directory."
        else
            (cd $T;./makeMtk -t -o=TARGET_BUILD_VARIANT=$TARGET_BUILD_VARIANT,TARGET_BUILD_TYPE=$TARGET_BUILD_TYPE \
                $TARGET_PRODUCT mm $P)
            cd $HERE > /dev/null
        fi
    fi
}

function mmm()
{
    T=$(gettop)
    if [ "$T" ]; then
        local MAKEFILE=
        local ARGS=
        local DIR TO_CHOP
        local DASH_ARGS=$(echo "$@" | awk -v RS=" " -v ORS=" " '/^-.*$/')
        local DIRS=$(echo "$@" | awk -v RS=" " -v ORS=" " '/^[^-].*$/')
        local SNOD=
        for DIR in $DIRS ; do
            DIR=`echo $DIR | sed -e 's:/$::'`
            if [ -f $DIR/Android.mk ]; then
                TO_CHOP=`(cd -P -- $T && pwd -P) | wc -c | tr -d ' '`
                TO_CHOP=`expr $TO_CHOP + 1`
                START=`PWD= /bin/pwd`
                MFILE=`echo $START | cut -c${TO_CHOP}-`
                if [ "$MFILE" = "" ] ; then
                    MFILE=$DIR/Android.mk
                else
                    MFILE=$MFILE/$DIR/Android.mk
                fi
                MAKEFILE="$MAKEFILE $MFILE"
            else
                if [ "$DIR" = snod ]; then
                    ARGS="$ARGS snod"
                    SNOD=snod
                elif [ "$DIR" = showcommands ]; then
                    ARGS="$ARGS showcommands"
                elif [ "$DIR" = dist ]; then
                    ARGS="$ARGS dist"
                elif [ "$DIR" = incrementaljavac ]; then
                    ARGS="$ARGS incrementaljavac"
                else
                    echo "No Android.mk in $DIR."
                    return 1
                fi
            fi
            (cd $T; ./makeMtk -t -o=TARGET_BUILD_VARIANT=$TARGET_BUILD_VARIANT,TARGET_BUILD_TYPE=$TARGET_BUILD_TYPE,SNOD=$SNOD \
                $TARGET_PRODUCT mm $DIR)           
        done
    else
        echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
}

function create_link
{
    MAKEFILE_PATH=`/bin/ls device/*/*/$1.mk 2>/dev/null`

    TARGET_PRODUCT=`cat $MAKEFILE_PATH | grep PRODUCT_NAME | awk '{print $3}'`
    BASE_PROJECT=`cat $MAKEFILE_PATH | grep BASE_PROJECT | awk '{print $3}'`

    if [ ! "x$BASE_PROJECT" == x ]; then
        echo "Create symbolic link - $TARGET_PRODUCT based on $BASE_PROJECT"
        if [ ! -e "./mediatek/config/$TARGET_PRODUCT" ]; then
            ln -s `pwd`/mediatek/config/$BASE_PROJECT ./mediatek/config/$TARGET_PRODUCT
        fi
        if [ ! -e "./mediatek/custom/$TARGET_PRODUCT" ]; then
            ln -s `pwd`/mediatek/custom/$BASE_PROJECT ./mediatek/custom/$TARGET_PRODUCT
        fi
    else
        echo ./mediatek/config/$TARGET_PRODUCT
        if [ ! -e "./mediatek/config/$TARGET_PRODUCT" ]; then
            echo "NO BASE_PROJECT!!"
            return 1
        fi
    fi
}

function snod()
{
    T=$(gettop)
    if [ "$T" ]; then
        ./makeMtk -t -o=TARGET_BUILD_VARIANT=$TARGET_BUILD_VARIANT,TARGET_BUILD_TYPE=$TARGET_BUILD_TYPE,$@ \
            $TARGET_PRODUCT snod
    else
        echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
}

function mtk_custgen()
{
    T=$(gettop)
    if [ "$T" ]; then
        rm -rf mediatek/config/out/$TARGET_PRODUCT
        ./makeMtk -o=TARGET_BUILD_VARIANT=$TARGET_BUILD_VARIANT,TARGET_BUILD_TYPE=$TARGET_BUILD_TYPE $TARGET_PRODUCT custgen
    else
        echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
}

function m_sync_lge_chrome()
{
    PWD=$(pwd)

    if [ "${PWD##*android}" != "" ]; then
        echo 'Do it in android/ folder'
        exit 1
    fi

    BRANCH_NAME=$(echo $(repo manifest) | tr -d '\"' | grep -m 1 -o 'revision=[a-z0-9_]*' | cut -d '=' -f 2)
    MISSING_PROJECTS="vendor/lge/external/chromium_lge/src/chrome/test/data/perf/third_party/octane \
        vendor/lge/external/chromium_lge/src/chrome/tools/test/reference_build/chrome_linux \
        vendor/lge/external/chromium_lge/src/swe/android/support/src \
        vendor/lge/external/chromium_lge/src/third_party/eyesfree/src/android/java/src/com/googlecode/eyesfree/braille \
        vendor/lge/external/chromium_lge/src/third_party/jsoncpp/source/include \
        vendor/lge/external/chromium_lge/src/third_party/jsoncpp/source/src/lib_json \
        vendor/lge/external/chromium_lge/src/third_party/libjingle/source/talk \
        vendor/lge/external/chromium_lge/src/third_party/sfntly/cpp/src"

    echo "BRANCH NAME : $BRANCH_NAME"
    for project in $MISSING_PROJECTS
    do
        echo "PROJECT NAME : $project"
        repo sync -qc $project
        repo start $BRANCH_NAME $project
    done

}

function m_clean()
{
    PWD=$(pwd)

    if [ "${PWD##*android}" != "" ]; then
        echo 'Do it in android/ folder'
        exit 1
    fi

    repo forall -c 'git clean -xdf; git checkout -f'

    m_sync_lge_chrome
}

function m_md5sum()
{
    if [ "$1" == "--help" ]; then
        echo "msa_md5sum"
        echo "msa_md5sum --detail [OUT FILE NAME]"
        return 0
    fi
    if [ "$1" == "--detail" ]; then
        if [ "$2" != "" ]; then
            MD5SUM_OUTFILENAME=$2
        else    
            MD5SUM_OUTFILENAME=$(date +%Y%m%d)
        fi
        repo forall -pc 'git log -1 --pretty=format:%H' > md5sum_${MD5SUM_OUTFILENAME}.txt
    else
        echo -e "\e[1;35m$(repo forall -c 'git log -1 --pretty=format:%H' | md5sum | sed "s/ -//g" | tr -d ' ')\e[00m"
    fi
}
