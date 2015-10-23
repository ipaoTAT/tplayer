#! /bin/bash

cd src && make

if [[ $? != 0 ]];then
	echo "Abort"
	exit 1
fi

echo "Build Success , run command under and enjoy it."
echo -e "\tcd bin && ./tplayer\n"
