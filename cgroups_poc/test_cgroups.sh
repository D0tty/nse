#!/bin/bash

# exec as root
if [[ "$USER" != "root" ]]; then
    echo "Script must be executed as root"
    exit 1
fi

# echo on
set -x

cgroup_dir="test_cgroup"
my_cgroup="my_cgroup"
test_bin="test_malloc"
memory="$((10 * 1024 * 1024))"

[[ "$#" -eq 1 ]] && memory="$1"

# display bin
cat "$test_bin"
# make test malloc
make "$test_bin"
cp "$test_bin" "/tmp/$test_bin"
# pause
read

# do dirty things in tmp
cd "/tmp"
# create our cgroup directory
[[ ! -d "$cgroup_dir" ]] && mkdir "$cgroup_dir"
# mount the cgroups
mount -t cgroup -o memory cgroup "$cgroup_dir"
# go inside
cd "$cgroup_dir" && ls --color=auto
# pause
read

# create our own_cgroup where all files are created automatically
[[ ! -d "$my_cgroup" ]] && mkdir "$my_cgroup"

cd "$my_cgroup" && ls --color=auto
# pause
read

# add our shell in the cgroups procs
echo "$$" > cgroup.procs
cat cgroup.procs

# add memory limit
echo "$memory" > memory.limit_in_bytes

# execute
/tmp/$test_bin
