#!/bin/bash


# TODO remove
echo "EN CHANTIER NE PAS LANCER"
exit 1

# args
if [ -z "$1" ]; then
    echo "Usage: $0 ISO"
    exit 1
fi

# vars
iso="$1"
rootdir="container-root"

# libcgroup
find_package() {
    output="$(which $1)"
    if [ "$?" -ne 0 ]; then
        echo "Package $1 not found, \'libcgroup\' required."
        exit 1
    else
        echo "Found $output."
    fi
}

for pkg in cgcreate cgset cgexec; do
    find_package "$pkg"
done

# DL conteneur
if [ ! -d "$rootdir" ]; then
    mkdir -p "$rootdir"
    sudo mount -o loop "$iso" "$rootdir"
else
    echo "Rootdir $(realpath $rootdir) already exists, skipping."
fi

# validity
cd "$rootdir"
if [ ! -d "bin" ]; then
    echo "Not a valid container."
fi

# control group setup
cgroup_id="cgroup_$(shuf -i 1000-2000 -n 1)"
controllers="cpu,cpuacct,memory"

sudo cgcreate -g "$controllers:$cgroup_id"

sudo cgset -r cpu.shares=512 "$cgroup_id"
sudo cgset -r memory.limit_in_bytes=1000000000 "$cgroup_id"
# [...]

# sudo cgget "$cgroup_id" # affiche les details du cgroupe

# run container
sudo cgexec -g "$controllers:$cgroup_id" \
    unshare -fmuipn --mount-proc \
    chroot "$PWD" \
    /bin/sh -c "
        /bin/mount -t proc proc /proc &&
        /bin/hostname mycontainer &&
        /usr/bin/bash"
