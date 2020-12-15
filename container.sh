#!/bin/bash

# vars
url="bit.ly/fish-container"
archive="fish.tar"
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
if [ ! -f "$archive" ]; then
    echo "Downloading fish container..."
    wget -nv "$url" -O "$archive"
else
    echo "Archive $(realpath $archive) already exists, skipping."
fi

# dir setup
if [ ! -d "$rootdir" ]; then
    mkdir -p "$rootdir" && cd "$rootdir"
    tar -xf "../$archive"
else
    echo "Container $(realpath $rootdir) already exists, skipping."
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
        export \"PATH=/bin:$PATH\" &&
        /bin/mount -t proc proc /proc &&
        /bin/hostname mycontainer &&
        /usr/bin/fish"
