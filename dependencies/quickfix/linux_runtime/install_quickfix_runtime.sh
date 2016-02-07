#!/bin/bash

SHARED_OBJECT_NAME=libquickfix.so.16.0.1
SHARED_OBJECT_LINK1=libquickfix.so
SHARED_OBJECT_LINK2=libquickfix.so.16
SO_RUNTIME_DIR=/usr/local/lib
SO_CONF_FILE=/etc/ld.so.conf

function append_to_end_of_file()
{
	local text=$1
	local target_file=$2
	echo $text | sudo tee --append $target_file
}

# Step 1. Copy shared object and the static library to /usr/local/lib
sudo cp $SHARED_OBJECT_NAME $SO_RUNTIME_DIR
# Step 2. Create soft links 
sudo ln -sf $SO_RUNTIME_DIR/$SHARED_OBJECT_NAME $SO_RUNTIME_DIR/$SHARED_OBJECT_LINK1
sudo ln -sf $SO_RUNTIME_DIR/$SHARED_OBJECT_NAME $SO_RUNTIME_DIR/$SHARED_OBJECT_LINK2
# Step 3. Add entry to ld.so.conf if it is not there already
if ! sudo grep -q $SO_RUNTIME_DIR $SO_CONF_FILE ; then
    append_to_end_of_file $SO_RUNTIME_DIR $SO_CONF_FILE
fi
# Step 4. Execute ldconfig
sudo ldconfig
