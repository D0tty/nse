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

echo "----> Building code"
# make test malloc
make "$test_bin"
cp "$test_bin" "/tmp/$test_bin"
# pause
read

echo "----> Preparing cgroup directory"
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

echo "----> Creating cgroup"
# create our own_cgroup where all files are created automatically
[[ ! -d "$my_cgroup" ]] && mkdir "$my_cgroup"

cd "$my_cgroup" && ls --color=auto
# pause
read

echo "----> Attach our process to the cgroup"
# add our shell in the cgroups procs
echo "$$" > cgroup.procs
cat cgroup.procs
# pause
read

echo "----> Limiting memory to $memory"
# add memory limit
echo "$memory" > memory.limit_in_bytes
cat memory.limit_in_bytes
# pause
read

echo "----> Executing binary"
# execute
/tmp/$test_bin
