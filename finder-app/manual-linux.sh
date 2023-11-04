#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u
export PATH=$PATH:/home/amente/Downloads/EmLinuxCourseAssigments/arm-cross-compiler/gcc-arm-10.2-2020.11-x86_64-aarch64-none-linux-gnu/bin

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    echo "Cleaning..."
    make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} mrproper
    echo "Defconfiging..."
    make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} defconfig

    if ! make -j4 ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} all; then
        echo "patching"
        sed -i 's/^YYLTYPE yylloc;/extern YYLTYPE yylloc;/' scripts/dtc/dtc-lexer.lex.c
        make -j4 ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} all
    fi

    #make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- modules
    make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- dtbs
fi

echo "Adding the Image in outdir"
cp -v ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}/

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
mkdir -p ${OUTDIR}/rootfs
cd ${OUTDIR}/rootfs
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu-
make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- CONFIG_PREFIX="${OUTDIR}/busybox" install

echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"


for dir in bin sbin usr/bin usr/sbin; do
    cp -a ${OUTDIR}/busybox/${dir}/* ${OUTDIR}/rootfs/${dir}
done

# TODO: Add library dependencies to rootfs
SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)
cp -Lv "$SYSROOT"/lib/ld-linux-aarch64.* "${OUTDIR}/rootfs/lib"
cp -Lv "$SYSROOT"/lib64/libm.so.* "${OUTDIR}/rootfs/lib64"
cp -Lv "$SYSROOT"/lib64/libresolv.so.* "${OUTDIR}/rootfs/lib64"
cp -Lv "$SYSROOT"/lib64/libc.so.* "${OUTDIR}/rootfs/lib64"

# TODO: Make device nodes
echo "making device nodes"
cd ${OUTDIR}/rootfs
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 666 dev/console c 5 1

# others
# create /dev/random
sudo mknod -m 444 dev/random c 1 8
sudo mknod -m 444 dev/urandom c 1 9
# create /dev/ttyS0
sudo mknod -m 666 dev/ttyS0 c 4 64
# create /dev/tty
sudo mknod -m 666 dev/tty c 5 0

# TODO: Clean and build the writer utility
cd ${FINDER_APP_DIR}
make clean
make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE}

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp -av ${FINDER_APP_DIR}/{writer,finder.sh,finder-test.sh,autorun-qemu.sh} ${OUTDIR}/rootfs/home/
mkdir -pv ${OUTDIR}/rootfs/home/conf
mkdir -pv ${OUTDIR}/rootfs/conf
cp -v ${FINDER_APP_DIR}/conf/{assignment.txt,username.txt} ${OUTDIR}/rootfs/home/conf/
cp -v ${FINDER_APP_DIR}/conf/{assignment.txt,username.txt} ${OUTDIR}/rootfs/conf/

# TODO: Chown the root directory
sudo chown -R root:root ${OUTDIR}/rootfs

# TODO: Create initramfs.cpio.gz
cd "$OUTDIR/rootfs"
find . | cpio  -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
cd "$OUTDIR"
gzip -f initramfs.cpio




