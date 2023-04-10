#!/bin/bash

VERSION=0.5
DEFCONFIG=lineageos_i9300_defconfig
TOOLCHAIN=/home/admin/Schreibtisch/Kernel/arm-linux-androideabi-4.9/bin/arm-linux-androideabi-
KCONFIG=false
CUST_CONF=no
BUILD_NUMBER=
DEVICE=UNKNOWN
KCONF_REPLACE=false
KERNEL_NAME="html6405_kernel"
KERNEL_VERSION="r1.0"
SKIP_MODULES=true
DONTPACK=false
USER=$USER
DATE=`date`
SOURCE_PATH=`pwd`
BUILD_PATH=$SOURCE_PATH/../obj/
CLEAN=false


usage() {
	echo "Koffee build script v$VERSION"
	echo `date`
	echo "Written by A\$teroid <acroreiser@gmail.com>"
	echo ""
    echo "Usage:"
    echo ""
	echo "make-koffee.sh [-d/-D/-O <file>] [-K] [-U <user>] [-N <BUILD_NUMBER>] [-k] [-R] -t <toolchain_prefix> -d <defconfig>"
	echo ""
	echo "Main options:"
	echo "	-K 			call Kconfig (use only with ready config!)"
	echo "	-d 			defconfig for the kernel. Will try to use already generated .config if not specified"
	echo "	-S 			set device codename (m0 for i9300 or t03g, for n7100, p4noterf for n8000, or t0lte for n7105)"
	echo "	-O <file> 			external/other defconfig."
	echo "	-t <toolchain_prefix> 			toolchain prefix"
	echo ""
	echo "Extra options:"
	echo "	-j <number_of_cpus> 			set number of CPUs to use"
	echo "	-k 			make only zImage, do not pack into zip"
	echo "	-C 			cleanup before building"
	echo "	-R 			save your arguments to reuse (just run make-koffee.sh on next builds)"
	echo "	-U <username> 			set build user"
	echo "	-N <release_number> 			set release number"
	echo "	-v 			show build script version"
	echo "	-h 			show this help"

}

get_revision()
{
	git rev-parse --short HEAD
}

get_tag()
{
	cur_tag=""
	for t in `git tag | sort --version-sort --reverse | grep "r[0-9]\."`; do
		git merge-base --is-ancestor $t HEAD;
		if [ $? -eq 0 ] ; then cur_tag=$t; break; fi;
	done

	echo $cur_tag
}

prepare()
{
 	make -j4 clean
}

make_config()
{
	mkdir -p $BUILD_PATH
	if [ "$CUST_CONF" != "no" ]; then
		cp $CUST_CONF `pwd`/.config
		echo "Using custom configuration from $CUST_CONF"
		DEFCONFIG=custom
	else
		if [ ! -f "`pwd`/.config" ] || [ "$KCONF_REPLACE" = true ]; then
			make O=$BUILD_PATH ARCH=arm $JOBS $DEFCONFIG &>/dev/null
		fi
	fi
	if [ "$KCONFIG" = "true" ]; then
		make $BUILD_PATH ARCH=arm $JOBS menuconfig
	fi
}

build_kernel()
{
	mkdir -p $BUILD_PATH
	make O=$BUILD_PATH ARCH=arm KBUILD_BUILD_VERSION=$BUILD_KERNEL $JOBS KBUILD_BUILD_USER=$USER CROSS_COMPILE=$TOOLCHAIN zImage
	if [ $? -eq 0 ]; then
		return 0
	else
		return 1
	fi
}

build_modules()
{
	mkdir -p $BUILD_PATH
	make O=$BUILD_PATH ARCH=arm KBUILD_BUILD_VERSION=$BUILD_NUMBER $JOBS KBUILD_BUILD_USER=$USER CROSS_COMPILE=$TOOLCHAIN modules
	if [ $? -eq 0 ]; then
		return 0
	else
		return 1
	fi
}

make_flashable()
{

	mkdir -p $REPACK_PATH
	# copy anykernel template over
	if [ "$DEVICE" == "m0" ]; then
		cp -R $SOURCE_PATH/anykernel_boeffla-i9300/* $REPACK_PATH
	fi	
	if [ "$DEVICE" == "t03g" ]; then
		cp -R $SOURCE_PATH/anykernel_boeffla-n7100/* $REPACK_PATH
	fi
	if [ "$DEVICE" == "p4noterf" ]; then
		cp -R $SOURCE_PATH/anykernel_boeffla-n8000/* $REPACK_PATH
	fi
	if [ "$DEVICE" == "kona3gxx" ]; then
  		cp -R $SOURCE_PATH/anykernel_boeffla-n51xx/* $REPACK_PATH
  fi
  if [ "$DEVICE" == "konawifixx" ]; then
    		cp -R $SOURCE_PATH/anykernel_boeffla-n51xx/* $REPACK_PATH
  fi
	if [ "$DEVICE" == "konaltexx" ]; then
		cp -R $SOURCE_PATH/anykernel_boeffla-n51xx/* $REPACK_PATH
	fi
	if [ "$DEVICE" == "t0lte" ]; then
		cp -R $SOURCE_PATH/anykernel_boeffla-t0lte/* $REPACK_PATH
	else		  
		echo "Attempt to create zip for unknown device. Aborting..."
		return 0
	fi	
		

	cd $REPACK_PATH
	# delete placeholder files
	find . -name placeholder -delete

	# copy kernel image
	cp $BUILD_PATH/arch/arm/boot/zImage $REPACK_PATH/zImage

	if [ "$SKIP_MODULES" = "false" ]; then
	{
		# copy modules to either modules folder (CM and derivates) or directly in ramdisk (Samsung stock)
		MODULES_PATH=$REPACK_PATH/modules

		mkdir -p $MODULES_PATH

		# copy generated modules
		find $BUILD_PATH -name '*.ko' -exec cp -av {} $MODULES_PATH \;

		# set module permissions
		chmod 0644 $MODULES_PATH/*

		# strip modules

		${TOOLCHAIN}strip --strip-unneeded $MODULES_PATH/*

	} 2>/dev/null

	fi
	REVISION=`get_revision`
	KERNEL_VERSION=`get_tag`
	# replace variables in anykernel script
	cd $REPACK_PATH
	KERNELNAME="Flashing html6405 kernel"
	sed -i "s;###kernelname###;${KERNELNAME};" META-INF/com/google/android/update-binary;
	COPYRIGHT_SCRIPT=$(echo '(c) A\$teroid × Lord Boeffla, 2018')
	sed -i "s;###copyright_script###;${COPYRIGHT_SCRIPT};" META-INF/com/google/android/update-binary;
	COPYRIGHT=$(echo '(c) html6405, 2022')
	sed -i "s;###copyright###;${COPYRIGHT};" META-INF/com/google/android/update-binary;
	BUILDINFO="Release ${BUILD_NUMBER}, $DATE (revision $REVISION)"
	sed -i "s;###buildinfo###;${BUILDINFO};" META-INF/com/google/android/update-binary;
	SOURCECODE="Source code: https://github.com/html6405/android_kernel_samsung_smdk4412"
	sed -i "s;###sourcecode###;${SOURCECODE};" META-INF/com/google/android/update-binary;


	# create zip file
	zip -r9 ${KERNEL_NAME}_${KERNEL_VERSION}-${REVISION}.zip * -x ${KERNEL_NAME}_${KERNEL_VERSION}-${REVISION}.zip

	# sign recovery zip if there are keys available
	if [ -f "$SOURCE_PATH/tools_boeffla/testkey.x509.pem" ]; then
		java -jar $SOURCE_PATH/tools_boeffla/signapk.jar -w $SOURCE_PATH/tools_boeffla/testkey.x509.pem $SOURCE_PATH/tools_boeffla/testkey.pk8 ${KERNEL_NAME}_${KERNEL_VERSION}-${REVISION}.zip ${KERNEL_NAME}_${KERNEL_VERSION}-${REVISION}-signed.zip
		cp ${KERNEL_NAME}_${KERNEL_VERSION}-${REVISION}-signed.zip $BUILD_PATH/${KERNEL_NAME}_${KERNEL_VERSION}-${REVISION}-${DEVICE}-signed.zip
		md5sum $SOURCE_PATH/${KERNEL_NAME}_${KERNEL_VERSION}-${REVISION}-${DEVICE}-signed.zip > $SOURCE_PATH/${KERNEL_NAME}_${KERNEL_VERSION}-${REVISION}-${DEVICE}-signed.zip.md5
	else
		cp ${KERNEL_NAME}_${KERNEL_VERSION}-${REVISION}.zip $SOURCE_PATH/${KERNEL_NAME}_${KERNEL_VERSION}-${REVISION}-${DEVICE}.zip
		md5sum $SOURCE_PATH/${KERNEL_NAME}_${KERNEL_VERSION}-${REVISION}-${DEVICE}.zip > $SOURCE_PATH/${KERNEL_NAME}_${KERNEL_VERSION}-${REVISION}-${DEVICE}.zip.md5
	fi



	cd $SOURCE_PATH
	rm -fr .tmpzip
	return 0
}

# Pre

_exit=-1
while getopts "hvO:j:Kd:kB:S:N:CU:t:" opt
do
	case $opt in
	h) usage; _exit=0; break;;
	v) echo $VERSION; _exit=0; break;;
	t) TOOLCHAIN=$OPTARG;;
	j) THREADS=$OPTARG;;
	O) CUST_CONF=$OPTARG; DEFCONFIG=custom; KCONF_REPLACE=true;;
	C) CLEAN=true;;
	N) BUILD_NUMBER=$OPTARG;;
	S) DEVICE=$OPTARG;;
	K) KCONFIG=true;;
	k) DONTPACK=true;;
	d) DEFCONFIG="$OPTARG"; KCONF_REPLACE=true;;
	U) USER=$OPTARG;;
	*) usage; _exit=0; break;;
esac
done

main() {
	if [ -z $THREADS ]; then
		JOBS="-j1"
		THREADS=1
	else
		JOBS="-j$THREADS"
	fi

	if [ -d "`pwd`/.tmpzip" ]; then
		rm -fr "`pwd`/.tmpzip"
	fi
	REPACK_PATH=`pwd`/.tmpzip

	# ENTRY POINT
	echo "Koffee build script v$VERSION"
	echo $DATE

	if [ "$DEFCONFIG" == "" ];
	then
		if [ ! -f "$BUILD_PATH/.config" ]; then
			echo "FATAL: No config specified!"
			echo "*** BUILD FAILED ***"
			return 1
		fi
		DEFCONFIG=".config"
	fi

	if [ "$DEFCONFIG" == "lineageos_i9300_defconfig" ]; then
		DEVICE="m0"
	fi

	if [ "$DEFCONFIG" == "lineageos_n7100_defconfig" ]; then
		DEVICE="t03g"
	fi

	if [ "$DEFCONFIG" == "lineageos_n8000_defconfig" ]; then
		DEVICE="p4noterf"
	fi
	
	if [ "$DEFCONFIG" == "lineageos_t0lte_defconfig" ]; then
		DEVICE="t0lte"
	fi

	if [ "$DEFCONFIG" == "lineageos_5100_defconfig" ]; then
  	DEVICE="kona3gxx"
  fi

  if [ "$DEFCONFIG" == "lineageos_5110_defconfig" ]; then
   	DEVICE="konawifixx"
  fi

	if [ "$DEFCONFIG" == "lineageos_5120_defconfig" ]; then
		DEVICE="konaltexx"
	fi

	if [ -z $TOOLCHAIN ]; then
		echo "FATAL: No toolchain prefix specified!"
		echo "*** BUILD FAILED ***"
		return 1
	fi

	if [ "$CLEAN" = "true" ]; then
		prepare &>/dev/null
	fi
	if [ "$DEFCONFIG" != ".config" ] || [ "$KCONFIG" == "true" ]; then
		make_config
	fi
	TVERSION=$(${TOOLCHAIN}gcc --version | grep gcc)

	if [ -z $BUILD_NUMBER ]; then 
		if [ -f "`pwd`/.version" ]; then
			BVERN=$(cat `pwd`/.version)
		else
			BVERN=1
		fi
	else
		BVERN=$BUILD_NUMBER
	fi

	if [ $? -eq 0 ]; then
		echo "--------------------------------------"
		echo "| Build  date:	$DATE"
		echo "| Version:	$KERNEL_VERSION"
		echo "| Configuration file:	$DEFCONFIG"
		echo "| Release:	$BVERN"
		echo "| Building for:	$DEVICE"
		echo "| Build  user:	$USER"
		echo "| Build  host:	`hostname`"
		echo "| Build  toolchain:	$TVERSION"
		echo "| Number of threads:	$THREADS"
		echo "--------------------------------------"
	else
		echo "*** CONFIGURATION FAILED ***"
		return 1
	fi
	echo "*** NOW WE GO! ***"
	echo "---- Stage 1: Building the kernel ----"
	build_kernel
	if [ $? -eq 0 ]; then
		echo "*** Kernel is ready! ***"
	else
		echo "*** zImage BUILD FAILED ***"
		return 1
	fi

	if [ "$SKIP_MODULES" = "false" ]; then
		echo "---- Stage 2: Building modules ----"
		build_modules
		if [ $? -eq 0 ]; then
			echo "*** Modules is ready! ***"
		else
			echo "*** MODULE BUILD FAILED ***"
			return 1
		fi
	else
		echo "---- Stage 2(skipped): Building modules ----"
	fi

	if [ "$DONTPACK" = "false" ]; then
		echo "---- Stage 3: Packing all stuff ----"
		make_flashable
		if [ $? -eq 0 ]; then
			echo "--------------------------------------"
			echo "--------------------------------------"
			echo "| Build  date:	$DATE"
			echo "| Version:	$KERNEL_VERSION"
			echo "| Configuration file:	$DEFCONFIG"
			echo "| Release:	$BVERN"
			echo "| Building for:	$DEVICE"
			echo "| Build  user:	$USER"
			echo "| Build  host:	`hostname`"
			echo "| Build  toolchain:	$TVERSION"
			echo "| Number of threads:	$THREADS"
			echo "> Flashable ZIP: $(ls | grep ${KERNEL_NAME}_${KERNEL_VERSION}-${REVISION}-${DEVICE} | grep .zip | head -n 1)"
			echo "> MD5sum: $(ls | grep ${KERNEL_NAME}_${KERNEL_VERSION}-${REVISION}-${DEVICE} | grep .md5)"
			echo "--------------------------------------"
			echo "*** Koffee is ready! ***"
		else
			echo "*** KOFFEE ZIP BUILD FAILED ***"
			return 1
		fi
	else
		echo "---- Stage 3(skipped): Packing all stuff ----"
		echo "--------------------------------------"
		echo "--------------------------------------"
		echo "| Build  date:	$DATE"
		echo "| Version:	$KERNEL_VERSION"
		echo "| Configuration file:	$DEFCONFIG"
		echo "| Release:	$BVERN"
		echo "| Building for:	$DEVICE"
		echo "| Build  user:	$USER"
		echo "| Build  host:	`hostname`"
		echo "| Build  toolchain:	$TVERSION"
		echo "| Number of threads:	$THREADS"
		echo "> zImage: arch/arm/boot/zImage"
		echo "--------------------------------------"
		echo "*** Koffee is ready! ***"
	fi
}

if [ ! $_exit -ge 0 ] ; then
	main
fi
